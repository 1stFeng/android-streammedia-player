/******************************************************************************
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 *******************************************************************************/

/******************************************************************************
 * @file FFMpegPlayer.kt
 * @ingroup  FFmpegPlayer
 * @author
 * @brief This file is designed as xxxx service
 ******************************************************************************/
package com.fawvw.hmi.ffmpegplayer.player

import android.view.Surface

class FFMpegPlayer : IPlayer {
    init {
        System.loadLibrary("native-lib")
    }

    override fun setDataSource(url: String) {
        nativeSetDataSource(url)
    }

    override fun setEventCallback(cb: IPlayer.PlayerEventCallback) {
        nativeSetEventCallback(cb)
    }

    override fun prepareAsync() {
        nativePrepareAsync()
    }

    override fun start() {
        nativeStart()
    }

    override fun pause() {
        nativePause()
    }

    override fun setSurface(surface: Surface) {
        nativeSetSurface(surface)
    }

    override fun getVideoWidth(): Int {
        return nativeGetVideoWidth()
    }

    override fun getVideoHeight(): Int {
        return nativeGetVideoHeight()
    }

    override fun execute(commands: String): Int {
        return nativeExecute(commands)
    }

    private external fun nativeExecute(commands: String): Int

    private external fun nativeSetEventCallback(cb: IPlayer.PlayerEventCallback)

    private external fun nativeSetDataSource(url: String)

    private external fun nativePrepareAsync()

    private external fun nativeStart()

    private external fun nativePause()

    private external fun nativeSetSurface(surface: Surface)

    private external fun nativeGetVideoWidth(): Int

    private external fun nativeGetVideoHeight(): Int
}