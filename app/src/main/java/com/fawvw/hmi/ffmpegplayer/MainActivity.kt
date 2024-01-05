/******************************************************************************
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 *******************************************************************************/

/******************************************************************************
 * @file MainActivity.kt
 * @ingroup  FFmpegPlayer
 * @author
 * @brief This file is designed as xxxx service
 ******************************************************************************/
package com.fawvw.hmi.ffmpegplayer

import android.Manifest
import android.app.ActionBar.LayoutParams
import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.ViewGroup
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.fawvw.hmi.ffmpegplayer.databinding.ActivityMainBinding
import com.fawvw.hmi.ffmpegplayer.player.FFMpegPlayer
import com.fawvw.hmi.ffmpegplayer.player.IJKPlayer
import com.fawvw.hmi.ffmpegplayer.player.IPlayer
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.io.File

class MainActivity : AppCompatActivity(), SurfaceHolder.Callback{
    companion object {
        private const val TAG = "MainActivity"
    }

    private val PERMISSIONS = arrayOf(
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.FOREGROUND_SERVICE
    )
    private var mVideoPath = "rtsp://192.168.40.146/live"
    private var mVideoView: PlaySurfaceView? = null
    private lateinit var mVideoPlayer: IPlayer
    private lateinit var binding: ActivityMainBinding
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        val view = binding.root
        setContentView(view)
        if (ContextCompat.checkSelfPermission(
                this,
                PERMISSIONS[0]
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            requestPermissions(PERMISSIONS, 0)
        }

        mVideoPlayer = FFMpegPlayer()
        handleEvent()
    }

    private fun handleEvent() {
        binding.btnStart.setOnClickListener {
//            mVideoPath = binding.pathChoose.text.toString()
            initVideoPlayer()
        }

        binding.btnPausePlay.setOnClickListener {
            mVideoPlayer.pause()
        }

        binding.btnGrabCover.setOnClickListener {
            val sTime = System.currentTimeMillis()
            val inputPath = getExternalFilesDir(null)!!.absolutePath + "/dvr_highway_20231103113305.mp4"
            //val inputPath = "http://192.168.35.146/DCIM/EVT/20231218180012.mp4"
            val outFile = getExternalFilesDir("cover")!!
            val outputPath = outFile.absolutePath + "/" + inputPath.substring(
                inputPath.lastIndexOf('/') + 1,
                inputPath.lastIndexOf('.')
            ) + ".jpg"

            val cmd =
                "ffmpeg -i $inputPath -n -f image2 -frames:v 1 -vf select=eq(pict_type\\,I) -s 345*194 $outputPath"
            mVideoPlayer.execute(cmd)
            Log.i(TAG, "load cover used time: ${System.currentTimeMillis() - sTime} ms")
        }
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        Log.i(TAG, "surfaceCreated--")
        mVideoPlayer.setSurface(holder.surface)
        mVideoPlayer.prepareAsync()
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        //do nothing
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        //do nothing
    }

    private fun initVideoPlayer() {
        //测试本地视频播放
//        val videoFile = getExternalFilesDir(null)!!.absolutePath + "/dvr_highway_20231103113305.mp4"
//        if (!File(videoFile).exists()) {
//            return
//        }
//        mVideoPlayer.setDataSource(videoFile)
        mVideoPlayer.setDataSource(mVideoPath)
        mVideoPlayer.setEventCallback(object : IPlayer.PlayerEventCallback {
            override fun onPrepared() {
                val videoWidth = mVideoPlayer.getVideoWidth()
                val videoHeight = mVideoPlayer.getVideoHeight()
                CoroutineScope(Dispatchers.Main).launch {
                    val viewWidth = mVideoView!!.width
                    val layoutParams = mVideoView!!.layoutParams
                    layoutParams.width = viewWidth
                    layoutParams.height = ((videoHeight / videoWidth.toFloat()) * viewWidth).toInt()
                    mVideoView!!.layoutParams = layoutParams
                    Log.i(TAG, "onPrepared--width:$videoWidth, height:$videoHeight")
                }

                Log.i(TAG, "onPrepared--width:$videoWidth, height:$videoHeight")
                mVideoPlayer.start()
            }
        })

        binding.clPreContainer.removeView(mVideoView)
        if (mVideoView != null) {
            mVideoView = null
        }
        mVideoView = PlaySurfaceView(this)
        mVideoView!!.holder.addCallback(this)
        binding.clPreContainer.addView(
            mVideoView,
            0,
            ViewGroup.LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT)
        )
    }
}