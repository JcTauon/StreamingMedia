# 编译CoreLibrary成动态库.so，引进app

## 一、macOS编译android-ffmpeg-arm64-v8a

### 1、下载NDK:r21e

```shell
#从23版开始NDK不再提供交叉编译工具链，22版NDK编译FFmpeg有问题，21版能正常编译NDK
#Android studio -> settings -> SDK Tools -> 21.4.7075529
```

### 2、clone ffmpeg源码

```shell
git clone git@github.com:FFmpeg/FFmpeg.git
```

### 3、切换至n5.1.2

```shell
git fetch --tags                # 获取所有 tag（如果本地没有的话）
git checkout tags/n5.1.2       # 切换到 n5.1.2 的 tag
cd FFmpeg
```

### 4、配置编译脚本

```shell
#!/bin/bash

# === 配置 NDK 路径和相关变量 ===
NDK=/path/to/yours/Android/sdk/ndk/21.4.7075529
SYSTEM=darwin-x86_64
API=21
ARCH=arm64
CPU=armv8-a
TARGET=aarch64-linux-android
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/$SYSTEM
PREFIX=/your/target/3rdParty/ffmpeg/arm64


# === 创建输出目录 ===
mkdir -p $PREFIX

make clean
make distclean

echo "=== Configuring FFmpeg for Android [$ARCH] ==="

./configure \
  --prefix=PREFIX \
  --target-os=android \
  --arch=arm64 \
  --cpu=armv8-a \
  --enable-cross-compile \
  --cc=${NDK}/toolchains/llvm/prebuilt/darwin-x86_64/bin/aarch64-linux-android21-clang \
  --cxx=${NDK}/toolchains/llvm/prebuilt/darwin-x86_64/bin/aarch64-linux-android21-clang++ \
  --cross-prefix=${NDK}/toolchains/llvm/prebuilt/darwin-x86_64/bin/aarch64-linux-android- \
  --sysroot=${NDK}/toolchains/llvm/prebuilt/darwin-x86_64/sysroot \
  --enable-shared \
  --disable-static \
  --disable-programs \
  --disable-ffmpeg --disable-ffplay --disable-ffprobe \
  --enable-avdevice \
  --enable-postproc \
  --enable-avfilter \
  --enable-gpl \
  --enable-nonfree

echo "=== Building FFmpeg... ==="
make -j$(nproc)
make install

echo "=== FFmpeg built for Android [$ARCH] at: $PREFIX ==="
```

### 5、运行上述脚本文件

```shell
chmod +x build_ffmpeg.sh
./build_ffmpeg.sh
```

### 6、修改CmakeLists.txt

```
#添加 FFmpeg 库路径和头文件
set(FFMPEG_ROOT ${CMAKE_SOURCE_DIR}/3rdParty/ffmpeg/arm64)
include_directories(${FFMPEG_ROOT}/include)
link_directories(${FFMPEG_ROOT}/lib)

# 声明 FFmpeg 依赖库
add_library(avcodec     SHARED IMPORTED)
add_library(avformat    SHARED IMPORTED)
add_library(avutil      SHARED IMPORTED)
add_library(swresample  SHARED IMPORTED)
add_library(swscale     SHARED IMPORTED)

set_target_properties(avcodec     PROPERTIES IMPORTED_LOCATION ${FFMPEG_ROOT}/lib/libavcodec.so)
set_target_properties(avformat    PROPERTIES IMPORTED_LOCATION ${FFMPEG_ROOT}/lib/libavformat.so)
set_target_properties(avutil      PROPERTIES IMPORTED_LOCATION ${FFMPEG_ROOT}/lib/libavutil.so)
set_target_properties(swresample  PROPERTIES IMPORTED_LOCATION ${FFMPEG_ROOT}/lib/libswresample.so)
set_target_properties(swscale     PROPERTIES IMPORTED_LOCATION ${FFMPEG_ROOT}/lib/libswscale.so)

```

## 二、macOS编译android-openCV

### 1.clone openCV

```shell
#opencv_contrib为openCV的扩展库，后续会使用openGL因此也需要编译进core
git clone --branch 4.5.5 https://github.com/opencv/opencv.git
git clone --branch 4.5.5 https://github.com/opencv/opencv_contrib.git
mkdir -p ~/opencv_android_build
cd opencv_android_build
```

### 2、编译openCV

```shell
#!/bin/bash

# ========== 1. 设置路径 ==========
export ANDROID_NDK=/your/Android/sdk/ndk/21.4.7075529  

# 当前脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# OpenCV 和 contrib 的绝对路径
export OPENCV_DIR="${SCRIPT_DIR}/../opencv"
export OPENCV_CONTRIB_DIR="${SCRIPT_DIR}/../opencv_contrib"

# ========== 2. 创建构建目录 ==========
mkdir -p "${SCRIPT_DIR}/build"
cd "${SCRIPT_DIR}/build"

# ========== 3. 配置构建 ==========
cmake -DCMAKE_TOOLCHAIN_FILE="${ANDROID_NDK}/build/cmake/android.toolchain.cmake" \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-21 \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TESTS=OFF \
      -DBUILD_PERF_TESTS=OFF \
      -DBUILD_ANDROID_PROJECTS=OFF \
      -DWITH_OPENCL=OFF \
      -DWITH_CUDA=OFF \
      -DWITH_IPP=OFF \
      -DWITH_TBB=OFF \
      -DWITH_EIGEN=OFF \
      -DOPENCV_FORCE_INSTALL=ON \
      -DOPENCV_ENABLE_NONFREE=ON \
      -DOPENCV_EXTRA_MODULES_PATH="${OPENCV_CONTRIB_DIR}/modules" \
      "${OPENCV_DIR}"

# ========== 4. 编译 ==========
make -j$(sysctl -n hw.logicalcpu) 
make install

```

### 三、将ffmpeg以及openCV编译好生成的include以及lib复制到项目Core文件夹下

### 1、修改CmakeLists.txt

```
cmake_minimum_required(VERSION 3.18.1)
project(Core LANGUAGES C CXX)

# 1. 设置 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. 显示详细编译信息
set(CMAKE_VERBOSE_MAKEFILE ON)

# 3. 添加头文件路径
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

# 4. 添加 OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# 5. 添加 FFmpeg 库路径和头文件
set(FFMPEG_ROOT ${CMAKE_SOURCE_DIR}/3rdParty/ffmpeg/arm64)
include_directories(${FFMPEG_ROOT}/include)
link_directories(${FFMPEG_ROOT}/lib)

# 6. 声明 FFmpeg 依赖库
add_library(avcodec     SHARED IMPORTED)
add_library(avformat    SHARED IMPORTED)
add_library(avutil      SHARED IMPORTED)
add_library(swresample  SHARED IMPORTED)
add_library(swscale     SHARED IMPORTED)

set_target_properties(avcodec     PROPERTIES IMPORTED_LOCATION ${FFMPEG_ROOT}/lib/libavcodec.so)
set_target_properties(avformat    PROPERTIES IMPORTED_LOCATION ${FFMPEG_ROOT}/lib/libavformat.so)
set_target_properties(avutil      PROPERTIES IMPORTED_LOCATION ${FFMPEG_ROOT}/lib/libavutil.so)
set_target_properties(swresample  PROPERTIES IMPORTED_LOCATION ${FFMPEG_ROOT}/lib/libswresample.so)
set_target_properties(swscale     PROPERTIES IMPORTED_LOCATION ${FFMPEG_ROOT}/lib/libswscale.so)

# 7. 添加核心源码
add_library(core SHARED
        CoreLibrary.cpp
        CoreLibrary.h
)

# 8. 链接库
target_link_libraries(core
        ${OpenCV_LIBS}
        avcodec
        avformat
        avutil
        swscale
        swresample
        android
        log
        z
        OpenSLES
        m
)

```

### 2、build

```
Build -> build Project
```

### 3、将生成的libcore.so复制到Android/app/src/main/jniLibs/arm64_v8a

```
#gradle会自动打包进apk
```

## 四、构建apk

### 1、编写app/src/main/cpp/CmakeLists.txt

```
cmake_minimum_required(VERSION 3.18.1)
project("androidplatform")

# 路径配置
set(CORE_INCLUDE_PATH /your/Core_include/path/Core)
set(OPENCV_INCLUDE_PATH /your/openCV_include/OpenCV-android-sdk/sdk/native/jni/include)
set(FFMPEG_INCLUDE_PATH /your/ffmpeg_include/ffmpeg/arm64/include)

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

```

### 2、改写app/build.gradle

```
plugins {
    alias(libs.plugins.android.application)
}

android {
    namespace 'com.example.androidplatform'
    compileSdk 35

    defaultConfig {
        applicationId "com.example.androidplatform"
        minSdk 24
        targetSdk 35
        versionCode 1
        versionName "1.0"

        testInstrumentationRunner "androidx.test.runner.AndroidJUnitRunner"
        externalNativeBuild {
            cmake {
                cppFlags ''
                abiFilters "arm64-v8a"
            }
        }
    }
		#指定ndk版本
    ndkVersion "21.4.7075529"

    buildTypes {
        release {
            minifyEnabled false
            proguardFiles getDefaultProguardFile('proguard-android-optimize.txt'), 'proguard-rules.pro'
        }
    }
    compileOptions {
        sourceCompatibility JavaVersion.VERSION_1_8
        targetCompatibility JavaVersion.VERSION_1_8
    }
    externalNativeBuild {
        cmake {
        		#指定Cmake版本
            version "3.18.1"
            path file('src/main/cpp/CMakeLists.txt')
        }
    }
}

dependencies {

    implementation libs.appcompat
    implementation libs.material
    implementation libs.activity
    implementation libs.constraintlayout
    testImplementation libs.junit
    androidTestImplementation libs.ext.junit
    androidTestImplementation libs.espresso.core
}
```

```
File ->Sync Project with Gradle Files
```

### 3、构建Project

```
Build -> Clean Project
Build -> ReBuild Project
```

