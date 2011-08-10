package org.linaro.glmark2;

import android.app.Activity;
import android.os.Bundle;
import android.opengl.GLSurfaceView;

public class Glmark2Activity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mGLView = new Glmark2SurfaceView(this);
        setContentView(mGLView);
    }

    @Override
    protected void onPause() {
        super.onPause();
        mGLView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mGLView.onResume();
    }

    private GLSurfaceView mGLView;

    static {
        System.loadLibrary("glmark2");
    }
}

