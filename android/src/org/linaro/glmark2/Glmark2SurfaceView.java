
package org.linaro.glmark2;

import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.content.res.AssetManager;
import android.app.Activity;
import android.util.Log;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/** 
 * Class that holds a configuration of a GL visual.
 */
class GLVisualConfig {
    public GLVisualConfig() {}
    public GLVisualConfig(int r, int g, int b, int a, int d, int buf) {
        red = r;
        green = g;
        blue = b;
        alpha = a;
        depth = d;
        buffer = buf;
    }

    public int red;
    public int green;
    public int blue;
    public int alpha;
    public int depth;
    public int buffer;
}


class Glmark2SurfaceView extends GLSurfaceView {

    public static final String LOG_TAG = "glmark2";

    public Glmark2SurfaceView(Activity activity) {
        super(activity);
        mActivity = activity;

        setEGLContextClientVersion(2);

        // Uncomment the commands below to get an RGBA8888 surface and config.
        //this.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        //setEGLConfigChooser(new Glmark2ConfigChooser(8, 8, 8, 8, 16, 0));
        setEGLConfigChooser(new Glmark2ConfigChooser(5, 6, 5, 0, 16, 0));

        setRenderer(new Glmark2Renderer(this));
    }

    /**
     * EGLConfigChooser that quits with an error dialog when a suitable config
     * cannot be found.
     */
    private class Glmark2ConfigChooser implements EGLConfigChooser {
        private int[] mAttribList;

        public Glmark2ConfigChooser(int redSize, int greenSize, int blueSize,
                                    int alphaSize, int depthSize, int stencilSize)
        {
            mAttribList = new int[] {
                    EGL10.EGL_RED_SIZE, redSize,
                    EGL10.EGL_GREEN_SIZE, greenSize,
                    EGL10.EGL_BLUE_SIZE, blueSize,
                    EGL10.EGL_ALPHA_SIZE, alphaSize,
                    EGL10.EGL_DEPTH_SIZE, depthSize,
                    EGL10.EGL_STENCIL_SIZE, stencilSize,
                    EGL10.EGL_RENDERABLE_TYPE, 4, /* 4 = EGL_OPENGL_ES2_BIT */
                    EGL10.EGL_NONE };
            mRedSize = redSize;
            mGreenSize = greenSize;
            mBlueSize = blueSize;
            mAlphaSize = alphaSize;
            mDepthSize = depthSize;
            mStencilSize = stencilSize;
       }

        @Override
        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
            try {
                return chooseConfigInternal(egl, display);
            }
            catch (Exception e) {
                /* Log an error message */
                Log.e(LOG_TAG, "No suitable EGLConfig for GLES2.0 found. Please check that proper GLES2.0 drivers are installed.");
                /* Display an informative (and lethal for the app) dialog */
                mActivity.runOnUiThread(new Runnable() {
                    public void run() {
                        mActivity.showDialog(Glmark2Activity.DIALOG_EGLCONFIG_FAIL_ID);
                    }
                });

                /* Wait here until the app process gets killed... */
                synchronized (this) {
                    try { this.wait(); } catch (Exception ex) { }
                }
            }
            return null;
        }

        private EGLConfig chooseConfigInternal(EGL10 egl, EGLDisplay display) {
            /* Get the number of available configs matching the attributes */
            int[] num_config = new int[1];
            if (!egl.eglChooseConfig(display, mAttribList, null, 0, num_config)) {
                throw new IllegalArgumentException("eglChooseConfig failed");
            }

            int numConfigs = num_config[0];

            if (numConfigs <= 0) {
                throw new IllegalArgumentException("No matching configs found");
            }

            /* Get the matching configs */
            EGLConfig[] configs = new EGLConfig[numConfigs];
            if (!egl.eglChooseConfig(display, mAttribList, configs, numConfigs,
                                     num_config))
            {
                throw new IllegalArgumentException("eglChooseConfig#2 failed");
            }

            /*
             * Try to find a config that matches exactly the RGBA size
             * specified by the user and is >= for depth and stencil.
             */
            for (EGLConfig config : configs) {
                int d = findConfigAttrib(egl, display, config,
                                         EGL10.EGL_DEPTH_SIZE, 0);
                int s = findConfigAttrib(egl, display, config,
                                         EGL10.EGL_STENCIL_SIZE, 0);
                int r = findConfigAttrib(egl, display, config,
                                         EGL10.EGL_RED_SIZE, 0);
                int g = findConfigAttrib(egl, display, config,
                                         EGL10.EGL_GREEN_SIZE, 0);
                int b = findConfigAttrib(egl, display, config,
                                         EGL10.EGL_BLUE_SIZE, 0);
                int a = findConfigAttrib(egl, display, config,
                                         EGL10.EGL_ALPHA_SIZE, 0);
                if (r == mRedSize && g == mGreenSize &&
                    b == mBlueSize && a == mAlphaSize &&
                    d >= mDepthSize && s >= mStencilSize)
                {
                    return config;
                }
            }

            throw new IllegalArgumentException("No configs match exactly");
        }

        private int findConfigAttrib(EGL10 egl, EGLDisplay display, EGLConfig config,
                                     int attribute, int defaultValue)
        {
            int[] value = new int[] { defaultValue };
            egl.eglGetConfigAttrib(display, config, attribute, value);
            return value[0];
        }

        protected int mRedSize;
        protected int mGreenSize;
        protected int mBlueSize;
        protected int mAlphaSize;
        protected int mDepthSize;
        protected int mStencilSize;
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
        if (!Glmark2Native.render())
            mView.getActivity().finish();
    }

    public void onSurfaceChanged(GL10 gl, int width, int height) {
        Glmark2Native.resize(width, height);
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        String args = mView.getActivity().getIntent().getStringExtra("args");
        Glmark2Native.init(mView.getActivity().getAssets(), args);
    }

    private Glmark2SurfaceView mView;
}

class Glmark2Native {
    public static native void init(AssetManager assetManager, String args);
    public static native void resize(int w, int h);
    public static native boolean render();
    public static native void done();
    public static native int scoreConfig(GLVisualConfig vc, GLVisualConfig target);
}
