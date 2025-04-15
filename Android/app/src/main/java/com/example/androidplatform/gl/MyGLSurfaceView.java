package com.example.androidplatform.gl;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import java.util.function.Consumer;

public class MyGLSurfaceView extends GLSurfaceView {
    private CameraGLRenderer renderer;
    private Consumer<SurfaceTexture> surfaceCallback;

    public MyGLSurfaceView(Context context) {
        super(context);
        init();
    }

    public MyGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    private void init() {
        setEGLContextClientVersion(2);
        renderer = new CameraGLRenderer();
        renderer.setSurfaceReadyCallback(surface -> {
            if (surfaceCallback != null) {
                surfaceCallback.accept(surface);
            }
        });
        setRenderer(renderer);
        setRenderMode(RENDERMODE_CONTINUOUSLY);
    }

    public void setFrameCallback(Consumer<SurfaceTexture> callback) {
        this.surfaceCallback = callback;
    }
}