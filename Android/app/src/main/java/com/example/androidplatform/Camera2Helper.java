package com.example.androidplatform;

import android.content.Context;
import android.content.pm.PackageManager;
import android.hardware.camera2.*;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.util.Size;
import android.view.Surface;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;

import java.util.Collections;

public class Camera2Helper {

    private final Context context;
    private CameraDevice cameraDevice;
    private CameraCaptureSession captureSession;

    public Camera2Helper(Context context) {
        this.context = context;
    }

    public void openCamera(SurfaceHolder holder) {
        try {
            CameraManager cameraManager = (CameraManager) context.getSystemService(Context.CAMERA_SERVICE);
            String cameraId = cameraManager.getCameraIdList()[0];

            if (context.checkSelfPermission(android.Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
                return;
            }

            Size previewSize = getBestPreviewSize(cameraId, cameraManager);
            holder.setFixedSize(previewSize.getWidth(), previewSize.getHeight());


            cameraManager.openCamera(cameraId, new CameraDevice.StateCallback() {
                @Override
                public void onOpened(@NonNull CameraDevice camera) {
                    cameraDevice = camera;
                    startPreview(holder);
                }

                @Override
                public void onDisconnected(@NonNull CameraDevice camera) {
                    camera.close();
                }

                @Override
                public void onError(@NonNull CameraDevice camera, int error) {
                    camera.close();
                }
            }, null);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void startPreview(SurfaceHolder holder) {
        try {
            Surface surface = holder.getSurface();
            CaptureRequest.Builder previewRequestBuilder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            previewRequestBuilder.addTarget(surface);

            cameraDevice.createCaptureSession(Collections.singletonList(surface),
                    new CameraCaptureSession.StateCallback() {
                        @Override
                        public void onConfigured(@NonNull CameraCaptureSession session) {
                            captureSession = session;
                            try {
                                session.setRepeatingRequest(previewRequestBuilder.build(), null, null);
                            } catch (CameraAccessException e) {
                                e.printStackTrace();
                            }
                        }

                        @Override
                        public void onConfigureFailed(@NonNull CameraCaptureSession session) {}
                    }, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private Size getBestPreviewSize(String cameraId, CameraManager cameraManager) throws CameraAccessException {
        CameraCharacteristics characteristics = cameraManager.getCameraCharacteristics(cameraId);
        StreamConfigurationMap map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        Size[] sizes = map.getOutputSizes(SurfaceHolder.class);

        // 找到与屏幕比例最接近的尺寸（如 16:9）
        float targetRatio = 16f / 9f;
        Size bestSize = sizes[0];
        float minDiff = Float.MAX_VALUE;

        for (Size size : sizes) {
            float ratio = (float) size.getWidth() / size.getHeight();
            float diff = Math.abs(ratio - targetRatio);
            if (diff < minDiff) {
                minDiff = diff;
                bestSize = size;
            }
        }
        return bestSize;
    }

    public void closeCamera() {
        if (captureSession != null) {
            captureSession.close();
            captureSession = null;
        }
        if (cameraDevice != null) {
            cameraDevice.close();
            cameraDevice = null;
        }
    }
}
