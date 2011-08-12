
package org.linaro.glmark2;

import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.content.res.AssetManager;
import android.app.Activity;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

class Glmark2SurfaceView extends GLSurfaceView {

    public Glmark2SurfaceView(Activity activity) {
        super(activity);
        mActivity = activity;

        setEGLContextClientVersion(2);
        
        // Uncomment the commands below to get an RGBA8888 surface and config.
        //this.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        //setEGLConfigChooser(8, 8, 8, 8, 16, 0);

        setRenderer(new Glmark2Renderer(this));
    }

    public Activity getActivity() {
        return mActivity;
    }

    private Activity mActivity;

}

class Glmark2Renderer implements GLSurfaceView.Renderer {
    public Glmark2Renderer(Glmark2SurfaceView view) {
        mView = view;
    }

    public void onDrawFrame(GL10 gl) {
        if (!nativeRender())
            mView.getActivity().finish();
    }

    public void onSurfaceChanged(GL10 gl, int width, int height) {
        nativeResize(width, height);
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeInit(mView.getActivity().getAssets());
    }

    private Glmark2SurfaceView mView;
    private static native void nativeInit(AssetManager assetManager);
    private static native void nativeResize(int w, int h);
    private static native boolean nativeRender();
    private static native void nativeDone();
}
