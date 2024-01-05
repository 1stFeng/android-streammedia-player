#include <jni.h>
#include <vector>
#include <string>
#include <sstream>
#include "ffmpeg.h"
#include "cmdutils.h"


extern "C" {
int cmd_exec(int argc, char **argv);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_mosi_mosc_dvr_media_FFMpegCmd_nativeExecute(JNIEnv *env, jobject obj,
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

    int len = elems.size();
    char *argv[len];
    for (int i = 0; i < len; i++) {
        argv[i] = new char[elems[i].size()];
        strcpy(argv[i], elems[i].c_str());
        LOGI("param %d: %s \n", i, argv[i]);
    }
    return cmd_exec(len, argv);
}
