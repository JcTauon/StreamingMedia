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

    //选用图片作为测试OpenCV集成情况
    cv::Mat testFrame = cv::imread("grass.jpg");
    if (testFrame.empty()) {
        std::cerr << "Failed to load sample image. 请确保 grass.jpg 存在于工作目录中." << std::endl;
        return -1;
    }

    int width = testFrame.cols;
    int height = testFrame.rows;
    int frameSize = width * height * 3;

    if (!testFrame.isContinuous()) {
        testFrame = testFrame.clone();
    }

    int format = 2;
    if (core.processVideoFrame(testFrame.data, width, height, format) != 0) {
        std::cerr << "Video frame processing failed." << std::endl;
    }

    cv::imshow("Processed Frame", testFrame);
    cv::waitKey(0);

    //音频接口测试
    int audioDataLength = 1024;
    std::vector<uint8_t> audioData(audioDataLength, 10);

    if (core.processAudioData(audioData.data(), 44100, 2) != 0) {
        std::cerr << "Audio processing failed." << std::endl;
    }

    if (core.transmitData(testFrame.data, frameSize) != 0) {
        std::cerr << "Data transmission failed." << std::endl;
    }

    core.releaseCore();

    std::cout << "CoreLibrary demo completed." << std::endl;

    return 0;
}
