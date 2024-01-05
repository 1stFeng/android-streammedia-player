/******************************************************************************
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 *******************************************************************************/

/******************************************************************************
 * @file IJKPlayer.kt
 * @ingroup  FFmpegPlayer
 * @author
 * @brief This file is designed as xxxx service
 ******************************************************************************/
package com.fawvw.hmi.ffmpegplayer.player

import android.view.Surface
import tv.danmaku.ijk.media.player.IMediaPlayer
import tv.danmaku.ijk.media.player.IjkMediaPlayer

class IJKPlayer:IPlayer {
    companion object {
        // all possible internal states
        private const val STATE_ERROR = -1
        private const val STATE_IDLE = 0
        private const val STATE_PREPARING = 1
        private const val STATE_PREPARED = 2
        private const val STATE_PLAYING = 3
        private const val STATE_PAUSED = 4
        private const val STATE_PLAYBACK_COMPLETED = 5
    }

    private var mVideoPath = ""
    private var mCurrentState = STATE_IDLE
    private var mTargetState  = STATE_IDLE
    private lateinit var mMediaPlayer: IMediaPlayer
    private var mEventCallback:IPlayer.PlayerEventCallback? = null
    init {
        IjkMediaPlayer.loadLibrariesOnce(null)
        IjkMediaPlayer.native_profileBegin("libijkplayer.so")
        IjkMediaPlayer.native_setLogLevel(IjkMediaPlayer.IJK_LOG_INFO)

        createPlayer()
    }
    override fun setDataSource(url: String) {
        mVideoPath = url
        mMediaPlayer.dataSource = mVideoPath
    }

    override fun setEventCallback(cb: IPlayer.PlayerEventCallback) {
        mEventCallback = cb
    }

    override fun prepareAsync() {
        mMediaPlayer.prepareAsync()
        mCurrentState = STATE_PREPARING
    }

    override fun start() {
        if (isInPlaybackState()) {
            mMediaPlayer.start()
            mCurrentState = STATE_PLAYING
        }
        mTargetState = STATE_PLAYING
    }

    override fun pause() {
        if (isInPlaybackState()) {
            if (mMediaPlayer.isPlaying) {
                mMediaPlayer.pause()
                mCurrentState = STATE_PAUSED
            } else {
                mMediaPlayer.start()
                mCurrentState = STATE_PLAYING
                mTargetState = STATE_PLAYING
            }
        }
        mTargetState = STATE_PAUSED
    }

    override fun setSurface(surface: Surface) {
        mMediaPlayer.setSurface(surface)
    }

    override fun getVideoWidth(): Int {
        return mMediaPlayer.videoWidth
    }

    override fun getVideoHeight(): Int {
        return mMediaPlayer.videoHeight
    }

    override fun execute(commands: String): Int {
        return 0
    }

    private fun isInPlaybackState(): Boolean {
        return mCurrentState != STATE_ERROR && mCurrentState != STATE_IDLE && mCurrentState != STATE_PREPARING
    }

    private fun createPlayer() {
        val ijkMediaPlayer = IjkMediaPlayer()

        //设置硬解码与软解码: value 1 硬解码，0 软解码
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_PLAYER, "mediacodec", 0)
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_PLAYER, "opensles", 0)
        ijkMediaPlayer.setOption(
            IjkMediaPlayer.OPT_CATEGORY_PLAYER,
            "overlay-format",
            IjkMediaPlayer.SDL_FCC_RV32.toLong()
        )
        ijkMediaPlayer.setOption(
            IjkMediaPlayer.OPT_CATEGORY_PLAYER,
            "max_cached_duration",
            1
        )
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_PLAYER, "infbuf", 1)
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_PLAYER, "packet-buffering", 0)
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_PLAYER, "is_live", 1)
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_PLAYER, "rtsp_transport", 1)
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_PLAYER, "framedrop", 1)
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_PLAYER, "start-on-prepared", 0)
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_PLAYER, "playback", 1)
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_FORMAT, "probesize", 100)
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_FORMAT, "analyzeduration", 100)
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_FORMAT, "http-detect-range-support", 0)
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_CODEC, "skip_loop_filter", 8L)
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_CODEC, "skip_frame", 0)
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_FORMAT, "analyzemaxduration", 100)
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_FORMAT, "flush_packets", 1L)
        ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_FORMAT, "fflags", "nobuffer")

        mMediaPlayer = ijkMediaPlayer

        mMediaPlayer.setOnPreparedListener {
            mCurrentState = STATE_PREPARED
            mEventCallback?.onPrepared()
        }

        mMediaPlayer.setOnErrorListener { _, _, _ ->
            mCurrentState = STATE_ERROR
            mTargetState = STATE_ERROR
            true
        }
    }
}