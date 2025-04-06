//
// Created by Ceram J on 2025/3/21.
//
#include "CoreLibrary.h"
#include <iostream>

#include <opencv2/opencv.hpp>


CoreLibrary::CoreLibrary()
    : initialized(false),
      audioCodecCtx(nullptr),
      audioFrame(nullptr),
      audioPacket(nullptr),
      networkInited(false),
      sockft(-1) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}


CoreLibrary::~CoreLibrary() {
    releaseCore();
#ifdef _WIN32
    WSACleanup(); // 释放Winsock
#endif
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
    if (!audioCodecCtx) {
        if (!initFFmpegAudioEncoder(sampleRate, channels)) {
            std::cerr << "[CoreLibrary] Failed to initialize FFmpeg audio encoder." << std::endl;
            return -1;
        }
    }
    if (!encodeAudioFrame(audioData, sampleRate, channels)) {
        std::cerr << "[CoreLibrary] Failed to encode audio frame." << std::endl;
        return -1;;
    }
    return 0;
}

bool CoreLibrary::initFFmpegAudioEncoder(int sampleRate, int channels) {
    const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!codec) {
        std::cerr << "[CoreLibrary] AAC encoder not found!" << std::endl;
        return false;
    }
    audioCodecCtx = avcodec_alloc_context3(codec);
    if (!audioCodecCtx) {
        std::cerr << "[CoreLibrary] Could not allocate audio codec context." << std::endl;
        return false;
    }

    /* 初始化ch_layout,接着指定声道数 */
    av_channel_layout_default(&audioCodecCtx->ch_layout, channels);
    audioCodecCtx->ch_layout.order = AV_CHANNEL_ORDER_NATIVE;

    audioCodecCtx->sample_rate = sampleRate;
    audioCodecCtx->sample_fmt = codec->sample_fmts ? codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    audioCodecCtx->bit_rate = 64000;

    /* 打开编码器 */
    if (avcodec_open2(audioCodecCtx, codec, nullptr) < 0) {
        std::cerr << "[CoreLibrary] Could not open AAC codec." << std::endl;
        return false;
    }

    /* 分配音频帧*/
    audioFrame = av_frame_alloc();
    if (!audioFrame) {
        std::cerr << "[CoreLibrary] Could not allocate audioFrame." << std::endl;
        return false;
    }
    audioFrame->nb_samples = audioCodecCtx->frame_size;
    audioFrame->format = audioCodecCtx->sample_fmt;
    audioFrame->ch_layout = audioCodecCtx->ch_layout;

    /* 分配音频帧缓冲区 */
    int ret = av_frame_get_buffer(audioFrame, 0);
    if (ret < 0) {
        std::cerr << "[CoreLibrary] Could not allocate audio frame buffer." << std::endl;
        return false;
    }

    /* 分配音频包 */
    audioPacket = av_packet_alloc();
    if (!audioPacket) {
        std::cerr << "[CoreLibrary] Could not allocate audio packet." << std::endl;
        return false;
    }

    std::cout << "[CoreLibrary] FFmpeg audio encoder(AAC) initialized successfully." << std::endl;
    return true;
}

bool CoreLibrary::encodeAudioFrame(uint8_t *audioData, int sampleRate, int channels) {
    if (!audioCodecCtx || !audioFrame || !audioPacket) {
        std::cerr << "[CoreLibrary] FFmpeg context not properly initialized." << std::endl;
        return false;
    }

    /*
    * 计算音频一帧的数据大小
    * 根据sample_fmt / ch_layout / frame_size计算
    */
    int dataSize = av_samples_get_buffer_size(nullptr,
                                              audioCodecCtx->ch_layout.nb_channels,
                                              audioCodecCtx->frame_size,
                                              audioCodecCtx->sample_fmt,
                                              1);
    if (dataSize < 0) {
        std::cerr << "[CoreLibrary] Failed to calculate audio data size." << std::endl;
        return false;
    }

    /* 确保 audioFrame可写 */
    if (av_frame_make_writable(audioFrame) < 0) {
        std::cerr << "[CoreLibrary] Audio frame not writable." << std::endl;
        return false;
    }

    /*
     * 复制PCM数据到 audioFrame->data[0](未做重采样),
     * 后续再添加 SwrContext 重采样以匹配编码器要求
     */
    std::memcpy(audioFrame->data[0], audioData, dataSize);

    /* 发送帧到编码器*/
    int ret = avcodec_send_frame(audioCodecCtx, nullptr);
    if (ret < 0) {
        std::cerr << "[CoreLibrary] Error sending frame to encoder." << std::endl;
        return false;
    }

    /* 从编码器获取编码后的数据*/
    while (true) {
        ret = avcodec_receive_packet(audioCodecCtx, audioPacket);
        if (ret == 0) {
            /* 编码成功*/
            std::cout << "[CoreLibrary] Encoded audio packet, size = " << audioPacket->size << " bytes." << std::endl;
            /* 处理Packet*/

            /* 释放AVPacket audioPacket资源*/
            av_packet_unref(audioPacket);
        } else if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            std::cout << "[CoreLibrary] No Packet recevied yet (need more data or EOF)." << std::endl;
            break;
        } else {
            std::cerr << "[CoreLibrary] Error receiving encoded packet." << std::endl;
            return false;
        }
    }

    return true;
}

bool CoreLibrary::initNetwork(const std::string &ip, int port) {
    std::lock_guard<std::mutex> lock(libMutex);
    if (!initialized) {
        std::cerr << "[CoreLibrary] Error: Must call initCore() before initNetwork()." << std::endl;
        return false;
    }

    if (networkInited) {
        std::cout << "[CoreLibrary] Network already initialized." << std::endl;
        return true;
    }

    sockft = socket(AF_INET, SOCK_STREAM, 0);
    if (sockft < 0) {
        std::cerr << "[CoreLibrary] Failed to creat socket." << std::endl;
        return false;
    }

    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "[CoreLibrary] Invail Ip address: " << ip << std::endl;
        close(sockft);
        sockft = -1;
        return false;
    }

    if (connect(sockft, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "[CoreLibrary] Failed to connect to " << ip << ": " << port << std::endl;
        close(sockft);
        sockft = -1;
        return false;
    }

    std::cout << "[CoreLibrary] Connect to " << ip << " : " << port << " successfully." << std::endl;
    networkInited = true;
    return true;
}

bool CoreLibrary::closeNetwork() {
    std::lock_guard<std::mutex> lock(libMutex);
    if (!networkInited) {
        std::cerr << "[CoreLibrary] Network not initialized or already closed." << std::endl;
        return false;
    }

    close(sockft);
    sockft = -1;
    networkInited = false;
    std::cout << "[CoreLibrary] Network connection closed." << std::endl;
    return true;
}

int CoreLibrary::transmitData(uint8_t *buffer, int length) {
    std::lock_guard<std::mutex> lock(libMutex);
    if (!initialized) {
        std::cerr << "[CoreLibrary] Error: Core not initialized!" << std::endl;
        return -1;
    }
    std::cout << "[CoreLibrary] Transmitting data of length: " << length << std::endl;

    if (networkInited && sockft >= 0) {
        ssize_t sent = send(sockft, buffer, length, 0);
        if (sent < 0) {
            std::cerr << "[CoreLibrary] Socket send error." << std::endl;
            return -1;
        }
        std::cout << "[CoreLibrary] Sent " << sent << " bytes via Socket." << std::endl;
        return static_cast<int>(sent);
    } else {
        simulateNetworkTransmission(buffer, length);
        return length;
    }
}

void CoreLibrary::releaseCore() {
    std::lock_guard<std::mutex> lock(libMutex);
    if (initialized) {
        std::cout << "[CoreLibrary] Releasing resources..." << std::endl;

        if (networkInited) {
            closeNetwork();
        }

        if (audioFrame) {
            av_frame_free(&audioFrame);
            audioFrame = nullptr;
        }
        if (audioPacket) {
            av_packet_free(&audioPacket);
            audioPacket = nullptr;
        }
        if (audioCodecCtx) {
            avcodec_free_context(&audioCodecCtx);
            audioCodecCtx = nullptr;
        }
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
        case 1: /* 美颜（使用双边滤波实现皮肤平滑效果）*/
            std::cout << "[CoreLibrary] applyVideoFilter(), format = 1，face beautification effect" << std::endl;
            cv::bilateralFilter(frame, processedFrame, 15, 80, 80);
            break;
        case 2: /* 夜景（增强亮度和对比度）*/
            std::cout << "[CoreLibrary] applyVideoFilter(), format = 2，nightscape" << std::endl;
            frame.convertTo(processedFrame, -1, 1.2, 30);
            break;
        case 3: /* 人像优化（通过直方图均衡化增强肤色对比度）*/
        {
            std::cout << "[CoreLibrary] applyVideoFilter(), format = 3，Portrait Optimisation" << std::endl;
            cv::Mat ycrcb;
            cv::cvtColor(frame, ycrcb, cv::COLOR_BGR2YCrCb);
            std::vector<cv::Mat> channels;
            cv::split(ycrcb, channels);
            cv::equalizeHist(channels[0], channels[0]);
            cv::merge(channels, ycrcb);
            cv::cvtColor(ycrcb, processedFrame, cv::COLOR_BGR2YCrCb);
            break;
        }
        case 4:
            std::cout << "[CoreLibrary] applyVideoFilter(), format = 4，Anti-shake, Gaussian Blur" << std::endl;
            cv::GaussianBlur(frame, processedFrame, cv::Size(3, 3), 0);
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
