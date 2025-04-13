package com.example.androidplatform;

public class CoreJNI {
    static {
        System.loadLibrary("opencv_core");
        System.loadLibrary("opencv_imgproc");
        System.loadLibrary("opencv_highgui");
        System.loadLibrary("opencv_videoio");
        System.loadLibrary("opencv_imgcodecs");
        System.loadLibrary("opencv_stitching");
        System.loadLibrary("native-lib");
        System.loadLibrary("core");
    }

    public native int initCore(String configJson);
    public native int processVideoFrame(byte[] frameData, int width, int height, int format);
    public native int processAudioData(byte[] audioData, int sampleRate, int channels);
    public native boolean initNetwork(String ip, int port);
    public native boolean closeNetwork();
    public native int transmitData(byte[] buffer, int length);
    public native void releaseCore();
}
