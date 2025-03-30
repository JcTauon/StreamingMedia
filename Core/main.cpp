#include "CoreLibrary.h"
#include <iostream>
#include <vector>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/mat.hpp>


int main() {
    CoreLibrary core;
    std::string configJson = R"({"video_filter":"default", "audio_codec":"AAC"})";

    if (core.initCore(configJson) != 0) {
        std::cerr << "Failed to initialize CoreLibrary." << std::endl;
        return -1;
    }

    bool netOK = core.initNetwork("127.0.0.1", 9000);
    if (!netOK) {
        std::cout << "[Main] initWork failed. Will use simulated transmission." << std::endl;
    }

    //选用图片作为测试OpenCV集成情况
    cv::Mat testFrame = cv::imread("/Users/ceramj/StreamingMedia/Core/grass.jpg");
    if (testFrame.empty()) {
        std::cerr << "Failed to load sample image, skipping video test." << std::endl;
    } else {
        int width = testFrame.cols;
        int height = testFrame.rows;
        if (!testFrame.isContinuous()) {
            testFrame = testFrame.clone();
        }
        int format = 2;
        core.processVideoFrame(testFrame.data, width, height, format);
        cv::imshow("Processed Frame", testFrame);
        cv::waitKey(0);
    }

    //音频接口测试
    int audioDataSize = 32768;
    std::vector<uint8_t> audioData(audioDataSize, 10);

    if (core.processAudioData(audioData.data(), 44100, 2) != 0) {
        std::cerr << "Audio processing failed." << std::endl;
    }

    core.transmitData(audioData.data(), audioDataSize);

    core.releaseCore();

    std::cout << "CoreLibrary demo completed." << std::endl;

    return 0;
}
