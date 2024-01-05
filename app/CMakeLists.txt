# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

set(lib_src_DIR ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI})
include_directories(
        ${CMAKE_SOURCE_DIR}/src/main/cpp/include
)

add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavcodec.so)

add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavfilter.so)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavformat.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavutil.so)

add_library(postproc SHARED IMPORTED)
set_target_properties(postproc PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libpostproc.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libswresample.so)

add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libswscale.so)
# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("native-lib")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        ${CMAKE_SOURCE_DIR}/src/main/cpp/native-lib.cpp
        ${CMAKE_SOURCE_DIR}/src/main/cpp/log_util.cpp
        ${CMAKE_SOURCE_DIR}/src/main/cpp/easy_player.cpp
        ${CMAKE_SOURCE_DIR}/src/main/cpp/opensles.cpp
        ${CMAKE_SOURCE_DIR}/src/main/cpp/event_callback.cpp
        ${CMAKE_SOURCE_DIR}/src/main/cpp/native_window_renderer.cpp
        ${CMAKE_SOURCE_DIR}/src/main/cpp/fftools/cmdutils.c
        ${CMAKE_SOURCE_DIR}/src/main/cpp/fftools/ffmpeg.c
        ${CMAKE_SOURCE_DIR}/src/main/cpp/fftools/ffmpeg_filter.c
        ${CMAKE_SOURCE_DIR}/src/main/cpp/fftools/ffmpeg_hw.c
        ${CMAKE_SOURCE_DIR}/src/main/cpp/fftools/ffmpeg_opt.c
        ${CMAKE_SOURCE_DIR}/src/main/cpp/fftools/sync_queue.c
        ${CMAKE_SOURCE_DIR}/src/main/cpp/fftools/opt_common.c
        ${CMAKE_SOURCE_DIR}/src/main/cpp/fftools/objpool.c
        ${CMAKE_SOURCE_DIR}/src/main/cpp/fftools/ffmpeg_dec.c
        ${CMAKE_SOURCE_DIR}/src/main/cpp/fftools/ffmpeg_enc.c
        ${CMAKE_SOURCE_DIR}/src/main/cpp/fftools/thread_queue.c
        ${CMAKE_SOURCE_DIR}/src/main/cpp/fftools/ffmpeg_mux.c
        ${CMAKE_SOURCE_DIR}/src/main/cpp/fftools/ffmpeg_demux.c
        ${CMAKE_SOURCE_DIR}/src/main/cpp/fftools/ffmpeg_mux_init.c
        )

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        android
        log
        OpenSLES
        avcodec
        avfilter
        avformat
        avutil
        postproc
        swresample
        swscale
        )