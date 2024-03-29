//
// Created by jasonxiao{github.com/JasonXiao001} on 2019/1/19.
//

#include "easy_player.h"
#include "log_util.h"
#include "common.h"
#include "opensles.h"
#include <unistd.h>

inline
char *GetAVErrorMsg(int code) {
    static char err_msg[1024];
    av_strerror(code, err_msg, sizeof(err_msg));
    return err_msg;
}

void PacketQueue::Put(AVPacket *pkt) {
    std::unique_lock<std::mutex> lck(mutex_);
    full_.wait(lck, [this] {
        return queue_.size() <= kMaxSize;
    });
    queue_.push(*pkt);
    ready_.notify_all();
}

void PacketQueue::Get(AVPacket *pkt) {
    if (pkt == nullptr) return;
    std::unique_lock<std::mutex> lck(mutex_);
    ready_.wait(lck, [this]{
        return !queue_.empty();
    });
    *pkt = queue_.front();
    queue_.pop();
    full_.notify_all();
}

void PacketQueue::Clear() {
    std::lock_guard<std::mutex> lck(mutex_);

}

AVFrame *FrameQueue::Get() {
    std::unique_lock<std::mutex> lck(mutex_);
    cond_.wait(lck, [this]{
        return !queue_.empty();
    });
    AVFrame *frame = queue_.front();
    queue_.pop();
    return frame;
}

void FrameQueue::Put(AVFrame *frame) {
    std::lock_guard<std::mutex> lck(mutex_);
    AVFrame *tmp = av_frame_alloc();
    av_frame_move_ref(tmp, frame);
    queue_.push(tmp);
    cond_.notify_all();
}


EasyPlayer::EasyPlayer() : state_(State::Idle),
                           ic_(nullptr), eof(0), video_stream_(-1), audio_stream_(-1),
                           event_cb_(nullptr), audio_clock(0) {
    av_log_set_callback(ff_log_callback);
    av_log_set_level(AV_LOG_DEBUG);
    //av_register_all();
    avformat_network_init();
}

EasyPlayer::~EasyPlayer() {
    if (ic_) {
        avformat_free_context(ic_);
        ic_ = nullptr;
    }

    if (audio_codec_ctx_) {
        avcodec_free_context(&audio_codec_ctx_);
        audio_codec_ctx_ = nullptr;
    }

    if (video_codec_ctx_) {
        avcodec_free_context(&video_codec_ctx_);
        video_codec_ctx_ = nullptr;
    }
}

int EasyPlayer::SetDataSource(const std::string &path) {
    if (state_ != State::Idle) {
        ELOG("illegal state|current:%d", state_)
        return ERROR_ILLEGAL_STATE;
    }
    ILOG("source path:%s", path.c_str())
    path_ = path;
    state_ = State ::Initialized;
    return SUCCESS;
}

int EasyPlayer::PrepareAsync() {
    if (state_ != State::Initialized && state_ != State::Stoped) {
        ELOG("illegal state|current:%d", state_)
        return ERROR_ILLEGAL_STATE;
    }
    state_ = State ::Preparing;
    read_thread_.reset(new std::thread(&EasyPlayer::read, this));
    return SUCCESS;
}

void EasyPlayer::SetEventCallback(EventCallback *cb) {
    event_cb_ = cb;
}

void EasyPlayer::GetData(uint8_t **buffer, int &buffer_size) {
    if (audio_buffer_ == nullptr) return;
    auto frame = audio_frames_.Get();
    int next_size;
    if (audio_codec_ctx_->sample_fmt == AV_SAMPLE_FMT_S16P) {
        next_size = av_samples_get_buffer_size(frame->linesize, audio_codec_ctx_->ch_layout.nb_channels, audio_codec_ctx_->frame_size, audio_codec_ctx_->sample_fmt, 1);
    }else {
        av_samples_get_buffer_size(&next_size, audio_codec_ctx_->ch_layout.nb_channels, audio_codec_ctx_->frame_size, audio_codec_ctx_->sample_fmt, 1);
    }
    int ret = swr_convert(swr_ctx_, &audio_buffer_, frame->nb_samples,
                          (uint8_t const **) (frame->extended_data),
                          frame->nb_samples);
    audio_clock = frame->best_effort_timestamp * av_q2d(audio_st_->time_base);
    av_frame_unref(frame);
    av_frame_free(&frame);
    *buffer = audio_buffer_;
    buffer_size = next_size;
}


void EasyPlayer::read() {
    int err;
    char err_buff[1024];
    err = avformat_open_input(&ic_, path_.c_str(), nullptr, nullptr);
    if (err) {
        av_strerror(err, err_buff, sizeof(err_buff));
        ELOG("avformat_open_input failed|ret:%d|msg:%s", err, err_buff)
        return;
    }
    err = avformat_find_stream_info(ic_, nullptr);
    if (err < 0) {
        ELOG("could not find codec parameters")
        return;
    }
    for(int i = 0; i < ic_->nb_streams; i++) {
        if (ic_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_ = i;
            ILOG("find audio stream index %d", i)
        }
        if (ic_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_ = i;
            ILOG("find video stream index %d", i)
        }
    }
    // start audio decode thread
    if (audio_stream_ >= 0) {
        openStream(audio_stream_);
    }
    // start video decode thread
    if (video_stream_ >= 0) {
        openStream(video_stream_);
    }

    auto *pkt = (AVPacket *)av_malloc(sizeof(AVPacket));
    if (pkt == nullptr) {
        ELOG("Could not allocate avPacket")
        return;
    }
    while (true) {
        err = av_read_frame(ic_, pkt);
        if (err < 0) {
            if ((err == AVERROR_EOF || avio_feof(ic_->pb))) {
                eof = 1;
            }
            if (ic_->pb && ic_->pb->error)
                break;
        }
        if (pkt->stream_index == audio_stream_) {
            audio_packets_.Put(pkt);
        } else if (pkt->stream_index == video_stream_) {
            video_packets_.Put(pkt);
        } else {
            av_packet_unref(pkt);
        }
    }
}

void EasyPlayer::decodeAudio() {
    AVPacket pkt;
    AVFrame *frame = av_frame_alloc();
    while (true) {
        audio_packets_.Get(&pkt);
        int ret;
        ret = avcodec_send_packet(audio_codec_ctx_, &pkt);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            ELOG("avcodec_send_packet error|code:%d|msg:%s", ret, GetAVErrorMsg(ret))
            break;
        }
        ret = avcodec_receive_frame(audio_codec_ctx_, frame);
        if (ret < 0 && ret != AVERROR_EOF) {
            ELOG("avcodec_receive_frame error|code:%d|msg:%s", ret, GetAVErrorMsg(ret))
            if (ret == -11) {
                continue;
            }
            break;
        }
        audio_frames_.Put(frame);
        if (audio_frames_.Size() >= AUDIO_READY_SIZE && state_ == State::Preparing) {
            // TODO illegal state check
            state_ = State ::Prepared;
            if (event_cb_) {
                event_cb_->OnPrepared();
            }
        }
    }
}

void EasyPlayer::decodeVideo(){
    ILOG("start decode video")
    AVPacket pkt;
    AVFrame *frame = av_frame_alloc();
    while (true) {
        video_packets_.Get(&pkt);
        int ret;
        ret = avcodec_send_packet(video_codec_ctx_, &pkt);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            ELOG("avcodec_send_packet error|code:%d|msg:%s", ret, GetAVErrorMsg(ret))
            break;
        }
        ret = avcodec_receive_frame(video_codec_ctx_, frame);
        if (ret < 0 && ret != AVERROR_EOF) {
            ELOG("avcodec_receive_frame error|code:%d|msg:%s", ret, GetAVErrorMsg(ret))
            if (ret == -11) {
                continue;
            }
            break;
        }

        video_frames_.Put(frame);
        if (state_ == State::Preparing) {
            // TODO illegal state check
            state_ = State ::Prepared;
            if (event_cb_) {
                event_cb_->OnPrepared();
            }
        }
    }
}

void EasyPlayer::openStream(int index) {
    ILOG("open stream|index:%d", index)
    AVCodecContext *avctx;
    if (index < 0 || index >= ic_->nb_streams)
        return;
    avctx = avcodec_alloc_context3(nullptr);
    if (!avctx) {
        ELOG("can not alloc codec ctx")
        return;
    }
    int ret = avcodec_parameters_to_context(avctx, ic_->streams[index]->codecpar);
    if (ret < 0) {
        avcodec_free_context(&avctx);
        ELOG("avcodec_parameters_to_context error %d", ret)
        return;
    }
    avctx->time_base = ic_->streams[index]->time_base;
    //av_codec_set_pkt_timebase(avctx, ic_->streams[index]->time_base);
    auto codec = avcodec_find_decoder(avctx->codec_id);
    avctx->codec_id = codec->id;
    ic_->streams[index]->discard = AVDISCARD_DEFAULT;
    ret = avcodec_open2(avctx, codec, nullptr);
    if (ret < 0) {
        ELOG("Fail to open codec on stream:%d|code:%d", index, ret)
        avcodec_free_context(&avctx);
        return;
    }
    switch (avctx->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            swr_ctx_ = swr_alloc();
            swr_ctx_ = swr_alloc_set_opts(nullptr,
                                         avctx->channel_layout, AV_SAMPLE_FMT_S16, avctx->sample_rate,
                                         avctx->channel_layout, avctx->sample_fmt, avctx->sample_rate,
                                         0, nullptr);
            if (!swr_ctx_ || swr_init(swr_ctx_) < 0) {
                ELOG("Cannot create sample rate converter for conversion channels!")
                swr_free(&swr_ctx_);
                return;
            }
            audio_st_ = ic_->streams[index];
            audio_codec_ctx_ = avctx;
            audio_buffer_ = (uint8_t *) malloc(sizeof(uint8_t)*AUDIO_BUFFER_SIZE);
            initAudioPlayer(audio_codec_ctx_->sample_rate, audio_codec_ctx_->ch_layout.nb_channels, this);
            audio_decode_thread_.reset(new std::thread(&EasyPlayer::decodeAudio, this));
            break;
        case AVMEDIA_TYPE_VIDEO:
            video_st_ = ic_->streams[index];
            img_convert_ctx_ = sws_getContext(avctx->width, avctx->height, avctx->pix_fmt,
                                             avctx->width, avctx->height, AV_PIX_FMT_RGBA, SWS_BICUBIC, nullptr, nullptr, nullptr);
            video_codec_ctx_ = avctx;
            frame_rgba_ = av_frame_alloc();
            int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, avctx->width, avctx->height, 1);
            rgba_buffer_ = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
            av_image_fill_arrays(frame_rgba_->data, frame_rgba_->linesize, rgba_buffer_, AV_PIX_FMT_RGBA, avctx->width, avctx->height, 1);
            video_decode_thread_.reset(new std::thread(&EasyPlayer::decodeVideo, this));
            break;
    }
}

int EasyPlayer::Start() {
    if (state_ != State::Prepared) {
        ELOG("illegal state|current:%d", state_);
        return ERROR_ILLEGAL_STATE;
    }
    if (audio_stream_ >= 0) {
        audio_render_thread_.reset(new std::thread(startAudioPlay));
    }
    if (video_stream_ >= 0) {
        video_render_thread_.reset(new std::thread(videoRender));
    }
    state_ = State ::Started;
    return SUCCESS;
}

int EasyPlayer::Pause() {
    if (state_ != State::Started) {
        ELOG("illegal state|current:%d", state_);
        return ERROR_ILLEGAL_STATE;
    }
    stopAudioPlay();
    state_ = State ::Paused;
    return SUCCESS;
}

void EasyPlayer::GetData(uint8_t **buffer, AVFrame **frame, int &width, int &height) {
    auto m_frame = video_frames_.Get();
    sws_scale(img_convert_ctx_, (const uint8_t* const*)m_frame->data, m_frame->linesize, 0, video_codec_ctx_->height,
              frame_rgba_->data, frame_rgba_->linesize);
    *frame = frame_rgba_;
    *buffer = rgba_buffer_;
    width = video_codec_ctx_->width;
    height = video_codec_ctx_->height;
    av_frame_unref(m_frame);
    av_frame_free(&m_frame);
}

int EasyPlayer::GetVideoWidth() {
    if (video_codec_ctx_)
        return video_codec_ctx_->width;
    return 0;
}

int EasyPlayer::GetVideoHeight() {
    if (video_codec_ctx_)
        return video_codec_ctx_->height;
    return 0;
}
