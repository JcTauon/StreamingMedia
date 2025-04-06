cmake_minimum_required(VERSION 3.18.1)
project("androidplatform")

# 路径配置
set(CORE_INCLUDE_PATH /Users/ceramj/StreamingMedia/Core)
set(OPENCV_INCLUDE_PATH /Users/ceramj/StreamingMedia/Core/3rdParty/openCV/OpenCV-android-sdk/sdk/native/jni/include)
set(FFMPEG_INCLUDE_PATH /Users/ceramj/StreamingMedia/Core/3rdParty/ffmpeg/arm64/include)

# 头文件路径
include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CORE_INCLUDE_PATH}
        ${OPENCV_INCLUDE_PATH}
        ${FFMPEG_INCLUDE_PATH}
)

# 加载 libcore.so
add_library(core SHARED IMPORTED GLOBAL)
set_target_properties(core PROPERTIES
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/arm64-v8a/libcore.so
)

# 编译 JNI 代码
add_library(native-lib SHARED
        core_jni.cpp
)

# 链接
target_link_libraries(native-lib
        core
        log
        android
)
