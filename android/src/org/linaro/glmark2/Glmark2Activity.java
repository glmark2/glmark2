package org.linaro.glmark2;

import android.app.Activity;
import android.os.Bundle;
import android.opengl.GLSurfaceView;
import android.app.Dialog;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Context;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;

import android.os.Build;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowInsetsController;

public class Glmark2Activity extends Activity {
    public static final int DIALOG_EGLCONFIG_FAIL_ID = 0;
    public static final String TAG = "GLMark2";
    private WakeLock mWakeLock;

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mWakeLock.release();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            getWindow().setDecorFitsSystemWindows(false);
        }
        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, TAG);
        mWakeLock.acquire();

        mGLView = new Glmark2SurfaceView(this);
        setContentView(mGLView);
        hideSystemBars();
    }

    private void hideSystemBars() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            WindowInsetsController controller =
                    getWindow().getInsetsController();

            if (controller != null) {
                controller.setSystemBarsBehavior(
                        WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);

                controller.hide(
                        WindowInsets.Type.statusBars()
                                | WindowInsets.Type.navigationBars());
            }
        } else {
            getWindow().getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);

        if (hasFocus) {
            hideSystemBars();
        }
    }

    @Override
    protected void onPause() {
        if (mGLView != null) {
            mGLView.onPause();
        }
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mGLView != null) {
            mGLView.onResume();
        }
        hideSystemBars();
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog;
        switch (id) {
            case DIALOG_EGLCONFIG_FAIL_ID:
                AlertDialog.Builder builder = new AlertDialog.Builder(this);
                builder.setMessage("Glmark2 cannot run because it couldn't find a suitable EGLConfig for GLES2.0. Please check that proper GLES2.0 drivers are installed.");
                builder.setCancelable(false);
                builder.setPositiveButton("Quit",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            Glmark2Activity.this.finish();
                            /*
                             * Force process shutdown. There is no safer way to
                             * do this, as we have open threads we can't close
                             * when we fail to get an EGLConfig
                             */
                            android.os.Process.killProcess(android.os.Process.myPid());
                        }
                    });

                dialog = builder.create();
                break;
            default:
                dialog = null;
                break;
        }

        return dialog;
    }


    private GLSurfaceView mGLView;

    static {
        System.loadLibrary("glmark2-android");
    }
}

