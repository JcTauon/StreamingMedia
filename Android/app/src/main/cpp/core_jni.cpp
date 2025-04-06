// Write C++ code here.
//
// Do not forget to dynamically load the C++ library into your application.
//
// For instance,
//
// In MainActivity.java:
//    static {
//       System.loadLibrary("androidplatform");
//    }
//
// Or, in MainActivity.kt:
//    companion object {
//      init {
//         System.loadLibrary("androidplatform")
//      }
//    }
#include "CoreLibrary.h"
#include <string>
#include <jni.h>

static CoreLibrary *gCore = nullptr;
extern "C" {

// 初始化核心库
JNIEXPORT jint JNICALL
Java_com_example_androidplatform_CoreJNI_initCore(JNIEnv *env, jobject /* this */, jstring configJson) {
    if (!gCore) gCore = new CoreLibrary();
    const char *config = env->GetStringUTFChars(configJson, nullptr);
    int result = gCore->initCore(std::string(config));
    env->ReleaseStringUTFChars(configJson, config);
    return result;
}

// 处理视频帧
JNIEXPORT jint JNICALL
Java_com_example_androidplatform_CoreJNI_processVideoFrame(JNIEnv *env, jobject, jbyteArray frameData,
                                                           jint width, jint height, jint format) {
    if (!gCore) return -1;
    jbyte *data = env->GetByteArrayElements(frameData, nullptr);
    int result = gCore->processVideoFrame(reinterpret_cast<uint8_t *>(data), width, height, format);
    env->ReleaseByteArrayElements(frameData, data, 0);
    return result;
}

// 处理音频数据
JNIEXPORT jint JNICALL
Java_com_example_androidplatform_CoreJNI_processAudioData(JNIEnv *env, jobject, jbyteArray audioData,
                                                          jint sampleRate, jint channels) {
    if (!gCore) return -1;
    jbyte *data = env->GetByteArrayElements(audioData, nullptr);
    int result = gCore->processAudioData(reinterpret_cast<uint8_t *>(data), sampleRate, channels);
    env->ReleaseByteArrayElements(audioData, data, 0);
    return result;
}

// 初始化网络连接
JNIEXPORT jboolean JNICALL
Java_com_example_androidplatform_CoreJNI_initNetwork(JNIEnv *env, jobject, jstring ip, jint port) {
    if (!gCore) return JNI_FALSE;
    const char *ipStr = env->GetStringUTFChars(ip, nullptr);
    bool result = gCore->initNetwork(std::string(ipStr), port);
    env->ReleaseStringUTFChars(ip, ipStr);
    return result ? JNI_TRUE : JNI_FALSE;
}

// 关闭网络连接
JNIEXPORT jboolean JNICALL
Java_com_example_androidplatform_CoreJNI_closeNetwork(JNIEnv *env, jobject) {
    if (!gCore) return JNI_FALSE;
    return gCore->closeNetwork() ? JNI_TRUE : JNI_FALSE;
}

// 发送数据
JNIEXPORT jint JNICALL
Java_com_example_androidplatform_CoreJNI_transmitData(JNIEnv *env, jobject, jbyteArray buffer, jint length) {
    if (!gCore) return -1;
    jbyte *data = env->GetByteArrayElements(buffer, nullptr);
    int result = gCore->transmitData(reinterpret_cast<uint8_t *>(data), length);
    env->ReleaseByteArrayElements(buffer, data, 0);
    return result;
}

// 释放资源
JNIEXPORT void JNICALL
Java_com_example_androidplatform_CoreJNI_releaseCore(JNIEnv *, jobject) {
if (gCore) {
gCore->releaseCore();
delete gCore;
gCore = nullptr;
}
}

} // extern "C"