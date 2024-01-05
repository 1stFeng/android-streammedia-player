/******************************************************************************
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 *******************************************************************************/

/******************************************************************************
 * @file IPlayer.kt
 * @ingroup  FFmpegPlayer
 * @author
 * @brief This file is designed as xxxx service
 ******************************************************************************/
package com.fawvw.hmi.ffmpegplayer.player

import android.view.Surface

interface IPlayer {
    fun setDataSource(url: String)
    fun setEventCallback(cb: PlayerEventCallback)
    fun prepareAsync()
    fun start()
    fun pause()
    fun setSurface(surface: Surface)
    fun getVideoWidth(): Int
    fun getVideoHeight(): Int
    fun execute(commands: String): Int

    interface PlayerEventCallback {
        fun onPrepared()
    }
}