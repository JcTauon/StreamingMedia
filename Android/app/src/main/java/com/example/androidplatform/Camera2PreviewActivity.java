package com.example.androidplatform;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.widget.Toast;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

public class Camera2PreviewActivity extends AppCompatActivity {

    private static final int REQUEST_CAMERA_PERMISSION = 1001;
    private SurfaceView surfaceView;
    private Camera2Helper camera2Helper;
    private AutoFitSurfaceView previewSize;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        surfaceView = findViewById(R.id.surfaceView);
        camera2Helper = new Camera2Helper(this);

        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(@NonNull SurfaceHolder holder) {
                if (ActivityCompat.checkSelfPermission(Camera2PreviewActivity.this
                        , Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED) {
                    camera2Helper.openCamera((SurfaceHolder) holder);
                    ((AutoFitSurfaceView) surfaceView).setAspectRatio(previewSize.getWidth(), previewSize.getHeight());

                } else {
                    ActivityCompat.requestPermissions(Camera2PreviewActivity.this,
                            new String[]{Manifest.permission.CAMERA}, REQUEST_CAMERA_PERMISSION);
                }
            }

            @Override
            public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {}

            @Override
            public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
                camera2Helper.closeCamera();
            }
        });
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_CAMERA_PERMISSION && grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            camera2Helper.openCamera((SurfaceHolder) surfaceView.getHolder());
        } else {
            Toast.makeText(this, "没有相机权限", Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        camera2Helper.closeCamera();
    }
}
