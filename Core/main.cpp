#include "CoreLibrary.h"
#include <iostream>
#include <vector>


int main() {
    CoreLibrary core;
    std::string configJson = R"({"video_filter":"default", "audio_codec":"AAC"})";

    if (core.initCore(configJson) != 0) {
        std::cerr << "Failed to initialize CoreLibrary." << std::endl;
        return -1;
    }

    //视频接口测试
    int width = 1920, height = 1080;
    int frameSize = width * height * 3;
    std::vector<uint8_t> videoFrame(frameSize, 0);

    if (core.processVideoFrame(videoFrame.data(), width, height, 1) != 0) {
        std::cerr << "Video frame processing failed." << std::endl;
    }

    //音频接口测试
    int audioDataLength = 1024;
    std::vector<uint8_t> audioData(audioDataLength, 10);

    if (core.processAudioData(audioData.data(), 44100, 2) != 0) {
        std::cerr << "Audio processing failed." << std::endl;
    }

    if (core.transmitData(videoFrame.data(), frameSize) != 0) {
        std::cerr << "Data transmission failed." << std::endl;
    }

    core.releaseCore();

    std::cout << "CoreLibrary demo completed." << std::endl;

    return 0;
}
