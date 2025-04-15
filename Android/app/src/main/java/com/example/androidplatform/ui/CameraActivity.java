package com.example.androidplatform.ui;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import com.example.androidplatform.R;
import com.example.androidplatform.camera.CameraController;
import com.example.androidplatform.gl.MyGLSurfaceView;

public class CameraActivity extends AppCompatActivity {

    private static final int REQUEST_PERMISSIONS = 1001;
    private static final String[] REQUIRED_PERMISSIONS = {
            Manifest.permission.CAMERA,
            Manifest.permission.RECORD_AUDIO
    };

    private MyGLSurfaceView glSurfaceView;
    private CameraController cameraController;
    private Button btn_switch;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (!hasAllPermissions()) {
            ActivityCompat.requestPermissions(this, REQUIRED_PERMISSIONS, REQUEST_PERMISSIONS);
            return;
        }

        setContentView(R.layout.activity_camera);

        glSurfaceView = findViewById(R.id.gl_surface_view);
        btn_switch = findViewById(R.id.btn_switch);

        cameraController = new CameraController(this);

        glSurfaceView.setFrameCallback(surfaceTexture -> {
            cameraController.setInputSurface(surfaceTexture);
            cameraController.openCamera();
        });

        cameraController.setFrameListener((data, width, height, pts) -> {
            runOnUiThread(() -> glSurfaceView.requestRender());
        });

        btn_switch.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                cameraController.switchCamera();
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (hasAllPermissions()) {
            glSurfaceView.onResume();
        }
    }

    @Override
    protected void onPause() {
        cameraController.closeCamera();
        glSurfaceView.onPause();
        super.onPause();
    }

    private boolean hasAllPermissions() {
        for (String permission : REQUIRED_PERMISSIONS) {
            if (ActivityCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_PERMISSIONS) {
            if (hasAllPermissions()) {
                recreate();
            } else {
                Toast.makeText(this, "需要相机和录音权限才能使用本应用", Toast.LENGTH_LONG).show();
                finish();
            }
        }
    }
}