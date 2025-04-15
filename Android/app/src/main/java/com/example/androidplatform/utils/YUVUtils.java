package com.example.androidplatform.utils;

import android.media.Image;

import java.nio.ByteBuffer;

public class YUVUtils {
    public static byte[] imageToByteArray(Image image) {
        int width = image.getWidth();
        int height = image.getHeight();
        int ySize = width * height;
        int uvSize = ySize / 2;

        byte[] nv21 = new byte[ySize + uvSize];
        ByteBuffer yBuffer = image.getPlanes()[0].getBuffer();
        ByteBuffer uBuffer = image.getPlanes()[1].getBuffer();
        ByteBuffer vBuffer = image.getPlanes()[2].getBuffer();

        int yRowStride = image.getPlanes()[0].getRowStride();
        int uvRowStride = image.getPlanes()[1].getRowStride();
        int uvPixelStride = image.getPlanes()[1].getPixelStride();

        // copy Y
        for (int row = 0; row < height; row++) {
            yBuffer.position(row * yRowStride);
            yBuffer.get(nv21, row * width, width);
        }

        // copy UV: convert to NV21
        int pos = ySize;
        for (int row = 0; row < height / 2; row++) {
            int uvRowStart = row * uvRowStride;
            for (int col = 0; col < width / 2; col++) {
                int uIndex = uvRowStart + col * uvPixelStride;
                int vIndex = uvRowStart + col * uvPixelStride;

                uBuffer.position(uIndex);
                vBuffer.position(vIndex);
                nv21[pos++] = vBuffer.get(); // V
                nv21[pos++] = uBuffer.get(); // U
            }
        }

        return nv21;
    }
}
