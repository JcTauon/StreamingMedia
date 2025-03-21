//
// Created by Ceram J on 2025/3/21.
//

#ifndef CORELIBRARY_H
#define CORELIBRARY_H
#include <string>

class CoreLibrary {
public:
    CoreLibrary();
    ~CoreLibrary();

    //初始化核心库，传入配置参数(JSON格式字符串: 滤镜参数、分辨率设置、硬件加速选项等)
    int initCore(const std::string& configJson);

    //处理单帧视频数据，返回处理后的数据或状态码
    int processVideoFrame(uint8_t* frameData, int width, int height, int format);

    //处理音频数据，实现编解码同步
    int processAudioData(uint8_t* audioData, int sampleRate, int channels);

    //实现网络传输，将处理后的数据通过Socket发送出去
    int transmitData(uint8_t* buffer, int length);

    //释放核心库资源，结束所有后台处理
    void releaseCore();
};
#endif //CORELIBRARY_H
