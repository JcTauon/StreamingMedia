//
// Created by Ceram J on 2025/3/21.
//
#include "CoreLibrary.h"
#include <iostream>

#include <opencv2/opencv.hpp>


CoreLibrary::CoreLibrary() : initialized(false) {
}

CoreLibrary::~CoreLibrary() {
    releaseCore();
}

int CoreLibrary::initCore(const std::string &configJson) {
    std::lock_guard<std::mutex> lock(libMutex);
    std::cout << "[CoreLibrary] Initalizing with config: " << configJson << std::endl;
    initialized = true;
    return 0;
}

int CoreLibrary::processVideoFrame(uint8_t *frameData, int width, int height, int format) {
    std::lock_guard<std::mutex> lock(libMutex);
    if (!initialized) {
        std::cerr << "[CoreLibrary] Error: Core not initialized!" << std::endl;
        return -1;
    }

    std::cout << "[CoreLibrary] Processing video frame (" << width << "x" << height << ", format " << ")" << std::endl;
    applyVideoFilter(frameData, width, height, format);
    return 0;
}

int CoreLibrary::processAudioData(uint8_t *audioData, int sampleRate, int channels) {
    std::lock_guard<std::mutex> lock(libMutex);
    if (!initialized) {
        std::cerr << "[CoreLibrary] Error: Core not initialized!" << std::endl;
        return -1;
    }

    std::cout << "[CoreLibrary] Processing audio data (sample rate : " << sampleRate << ", channels: "
            << channels << ")" << std::endl;
    processAudioBlock(audioData, sampleRate, channels);
    return 0;
}

int CoreLibrary::transmitData(uint8_t *buffer, int length) {
    std::lock_guard<std::mutex> lock(libMutex);
    if (!initialized) {
        std::cerr << "[CoreLibrary] Error: Core not initialized!" << std::endl;
        return -1;
    }

    std::cout << "[CoreLibrary] Transmitting data of length: " << length << std::endl;
    simulateNetworkTransmission(buffer, length);
    return 0;
}

void CoreLibrary::releaseCore() {
    std::lock_guard<std::mutex> lock(libMutex);
    if (initialized) {
        std::cout << "[CoreLibrary] Releasing resources." << std::endl;
        initialized = false;
    }
}

/**
 *集成FFmpeg、OpenCV以及Socket库，实现音视频编解码、美颜滤镜以及网络传输等功能。
 ***/

void CoreLibrary::applyVideoFilter(uint8_t *frameData, int width, int height, int format) {
    std::cout << "[CoreLibrary] Applying video filter type: "
            << format << std::endl;

    cv::Mat frame(height, width, CV_8UC3, frameData);
    cv::Mat processedFrame;

    switch (format) {
        case 1 : //美颜（使用双边滤波实现皮肤平滑效果）
            std::cout << "[CoreLibrary] applyVideoFilter(), format = 1，face beautification effect" << std::endl;
            cv::bilateralFilter(frame, processedFrame, 15, 80, 80);
        break;
        case 2: //夜景（增强亮度和对比度）
            std::cout << "[CoreLibrary] applyVideoFilter(), format = 2，nightscape" << std::endl;
            frame.convertTo(processedFrame, -1, 1.2, 30);
        break;
        case 3: // 人像优化（通过直方图均衡化增强肤色对比度）
        {
            std::cout << "[CoreLibrary] applyVideoFilter(), format = 3，Portrait Optimisation" << std::endl;
            cv::Mat ycrcb;
            cv::cvtColor(frame, ycrcb, cv::COLOR_BGR2YCrCb);
            std::vector<cv::Mat> channels;
            cv::split(ycrcb, channels);
            cv::equalizeHist(channels[0],channels[0]);
            cv::merge(channels, ycrcb);
            cv::cvtColor(ycrcb, processedFrame, cv::COLOR_BGR2YCrCb);
            break;
        }
        case 4:
            std::cout << "[CoreLibrary] applyVideoFilter(), format = 4，Anti-shake, Gaussian Blur" << std::endl;
            cv::GaussianBlur(frame, processedFrame, cv::Size(3,3), 0);
        break;
        default:
            std::cout << "[CoreLibrary] Unknown effect format. No processing applied." << std::endl;
        processedFrame = frame.clone();
        break;
    }
    std::memcpy(frameData, processedFrame.data, width * height * 3);
}

void CoreLibrary::processAudioBlock(uint8_t *audioData, int sampleRate, int channels) {
    std::cout << "[CoreLibrary] Processing audio block XXX" << std::endl;
    int sum = 0;

    //测试
    for (int i = 0; i < 10 && i < sampleRate; ++i) {
        sum += audioData[i];
    }

    int avg = sum / 10;
    std::cout << "[CoreLibrary] Average audio sample: " << avg << std::endl;
}

void CoreLibrary::simulateNetworkTransmission(uint8_t *buffer, int length) {
    std::cout << "[CoreLibrary] Simulating network transmission yyy" << std::endl;
    std::cout << "[CoreLibrary] Data transmitted sucessfully." << std::endl;
}
