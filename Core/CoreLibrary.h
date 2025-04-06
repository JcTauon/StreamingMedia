//
// Created by Ceram J on 2025/3/21.
//

#ifndef CORELIBRARY_H
#define CORELIBRARY_H
#include <string>
#include <cstdint>
#include <mutex>// std::memset
#include <arpa/inet.h>     // htons, inet_*



extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}


#ifdef _WIN32
    #include <winsock2.h>
// 需要链接 Ws2_32.lib
    typedef SOCKET SocketType;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int SocketType;
#endif

/**CoreLibrary类
 *initCore()初始化核心库，传入配置参数(JSON格式字符串: 滤镜参数、分辨率设置、硬件加速选项等)
 *processVideoFrame()处理单帧视频数据，返回处理后的数据或状态码, Format选择的算法效果 (1: 美颜, 2: 夜景, 3: 人像优化, 4: 防抖)
 *processAudioData()处理音频数据，实现编解码同步
 *transmitData()实现网络传输，将处理后的数据通过Socket发送出去
 *releaseCore()释放核心库资源，结束所有后台处理
 ***/
class CoreLibrary {
public:
    CoreLibrary();

    ~CoreLibrary();

    int initCore(const std::string &configJson);

    int processVideoFrame(uint8_t *frameData, int width, int height, int format);

    int processAudioData(uint8_t *audioData, int sampleRate, int channels);

    bool initNetwork(const std::string &ip, int port);

    bool closeNetwork();

    int transmitData(uint8_t *buffer, int length);

    void releaseCore();

private:
    bool initialized;
    std::mutex libMutex;

    void applyVideoFilter(uint8_t *frameData, int width, int height, int format);

    AVCodecContext *audioCodecCtx;
    AVFrame *audioFrame;
    AVPacket *audioPacket;

    bool initFFmpegAudioEncoder(int sampleRate, int channels);

    bool encodeAudioFrame(uint8_t *audioData, int sampleRate, int channels);

    bool networkInited;
    SocketType sockft;

    void processAudioBlock(uint8_t *audioData, int sampleRate, int channels);

    void simulateNetworkTransmission(uint8_t *buffer, int length);
};
#endif //CORELIBRARY_H
