#include <jni.h>
#include <vector>
#include <string>
#include <sstream>

#include "log_util.h"
#include "easy_player.h"
#include "event_callback.h"

extern "C" {
int cmd_exec(int argc, char **argv);
}

EasyPlayer *player = new EasyPlayer();
MyEventCallback *eventCallback;


extern "C"
void
Java_com_fawvw_hmi_ffmpegplayer_player_FFMpegPlayer_nativeSetEventCallback
        (JNIEnv *env, jobject obj, jobject jcallback) {
    if (eventCallback) {
        delete eventCallback;
    }
    eventCallback = new MyEventCallback(env, jcallback);
    player->SetEventCallback(eventCallback);
}

extern "C"
void
Java_com_fawvw_hmi_ffmpegplayer_player_FFMpegPlayer_nativeSetDataSource
        (JNIEnv *env, jobject obj, jstring url) {
    player->SetDataSource(env->GetStringUTFChars(url, nullptr));
}

extern "C"
void
Java_com_fawvw_hmi_ffmpegplayer_player_FFMpegPlayer_nativePrepareAsync
        (JNIEnv *env, jobject obj) {
    player->PrepareAsync();
}

extern "C"
void
Java_com_fawvw_hmi_ffmpegplayer_player_FFMpegPlayer_nativeStart
        (JNIEnv *env, jobject obj) {
    player->Start();
}

extern "C"
void
Java_com_fawvw_hmi_ffmpegplayer_player_FFMpegPlayer_nativePause
        (JNIEnv *env, jobject obj) {
    player->Pause();
}

extern "C"
void
Java_com_fawvw_hmi_ffmpegplayer_player_FFMpegPlayer_nativeSetSurface
        (JNIEnv *env, jobject obj, jobject surface) {
    initVideoRenderer(env, player, surface);
}

extern "C"
jint
Java_com_fawvw_hmi_ffmpegplayer_player_FFMpegPlayer_nativeGetVideoWidth
        (JNIEnv *env, jobject obj) {
    return player->GetVideoWidth();
}

extern "C"
jint
Java_com_fawvw_hmi_ffmpegplayer_player_FFMpegPlayer_nativeGetVideoHeight
        (JNIEnv *env, jobject obj) {
    return player->GetVideoHeight();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_fawvw_hmi_ffmpegplayer_player_FFMpegPlayer_nativeExecute(JNIEnv *env, jobject obj,
                                                     jstring commands) {

    std::string strCmd = env->GetStringUTFChars(commands, nullptr);

    std::string command;
    std::vector<std::string> elems;
    std::stringstream ss(strCmd);
    while (std::getline(ss, command, ' ')) {
        if (!command.empty()) {
            elems.push_back(command);
        }
    }

    int len = (int)elems.size();
    char *argv[len];
    for (int i = 0; i < len; i++) {
        argv[i] = new char[elems[i].size()];
        strcpy(argv[i], elems[i].c_str());
        ILOG("param %d: %s \n", i, argv[i]);
    }
    return cmd_exec(len, argv);
}

