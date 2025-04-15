package com.example.androidplatform.camera;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.*;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.util.Log;
import android.util.Size;
import android.view.Surface;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;

import com.example.androidplatform.utils.YUVUtils;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class CameraController {
    private static final String TAG = "CameraController";
    private final Context context;
    private CameraDevice cameraDevice;
    private CameraCaptureSession session;
    private ImageReader imageReader;
    private SurfaceTexture previewTexture;
    private Size previewSize = new Size(1280, 720);
    private String currentCameraId;
    private boolean isFrontCamera = false;
    private boolean isCameraReady = false;
    private boolean isSurfaceReady = false;



    public interface OnFrameListener {
        void onFrame(byte[] data, int width, int height, long timestamp);
    }

    private OnFrameListener frameListener;

    public CameraController(Context ctx) {
        this.context = ctx;
    }

    public void switchCamera() {
        isFrontCamera = !isFrontCamera;
        closeCamera();  // 关闭当前摄像头
        openCamera();   // 重新打开目标摄像头
    }


    public void setInputSurface(SurfaceTexture surfaceTexture) {
        if (surfaceTexture == null) {
            Log.e("CameraController", "SurfaceTexture is null!");
            return;
        }
        Log.d("CameraDebug", "setInputSurface 被调用");
        Log.d("CameraCheck", "SurfaceTexture received, size=" + previewSize.getWidth() + "x" + previewSize.getHeight());
        this.previewTexture = surfaceTexture;
        this.previewTexture.setDefaultBufferSize(previewSize.getWidth(), previewSize.getHeight());

        isSurfaceReady = true;
        tryStartPreview();
    }

    private void tryStartPreview() {
        if (isCameraReady && isSurfaceReady && cameraDevice != null) {
            startPreview();
        }
    }


    public void setFrameListener(OnFrameListener listener) {
        this.frameListener = listener;
    }

    public void openCamera() {
        isCameraReady = true;
        tryStartPreview();
    }

    public void closeCamera() {
        if (cameraDevice != null) {
            cameraDevice.close();
        }
    }

    private final CameraDevice.StateCallback stateCallback = new CameraDevice.StateCallback() {
        public void onOpened(@NonNull CameraDevice cd) {
            cameraDevice = cd;
            startPreview();
        }

        public void onDisconnected(@NonNull CameraDevice cd) {
            cd.close();
        }

        public void onError(@NonNull CameraDevice cd, int error) {
            cd.close();
        }
    };

    private void startPreview() {
        try {
            imageReader = ImageReader.newInstance(previewSize.getWidth(), previewSize.getHeight(), ImageFormat.YUV_420_888, 2);
            imageReader.setOnImageAvailableListener(reader -> {
                Image image = reader.acquireLatestImage();
                if (image != null && frameListener != null) {
                    byte[] data = YUVUtils.imageToByteArray(image);
                    frameListener.onFrame(data, image.getWidth(), image.getHeight(), System.nanoTime() / 1000);
                    image.close();
                }
            }, null);

            Surface previewSurface = new Surface(previewTexture);
            List<Surface> surfaces = Arrays.asList(previewSurface, imageReader.getSurface());

            CaptureRequest.Builder builder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            builder.addTarget(previewSurface);
            builder.addTarget(imageReader.getSurface());

            cameraDevice.createCaptureSession(surfaces, new CameraCaptureSession.StateCallback() {
                public void onConfigured(@NonNull CameraCaptureSession cs) {
                    session = cs;
                    try {
                        session.setRepeatingRequest(builder.build(), null, null);
                    } catch (CameraAccessException e) {
                        e.printStackTrace();
                    }
                }

                public void onConfigureFailed(@NonNull CameraCaptureSession cs) {}
            }, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }
}
