/******************************************************************************
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 *******************************************************************************/

/******************************************************************************
 * @file PlaySurfaceView.kt
 * @ingroup  FFmpegPlayer
 * @author
 * @brief This file is designed as xxxx service
 ******************************************************************************/
package com.fawvw.hmi.ffmpegplayer

import android.content.Context
import android.graphics.Canvas
import android.util.Log
import android.view.SurfaceView

class PlaySurfaceView(context: Context): SurfaceView(context) {
    companion object {
        private const val TAG = "PlaySurfaceView"
    }
    override fun onDraw(canvas: Canvas?) {
        super.onDraw(canvas)

        Log.i(TAG, "surface onDraw")
    }

    override fun draw(canvas: Canvas?) {
        super.draw(canvas)
        Log.i(TAG, "surface draw")
    }
}