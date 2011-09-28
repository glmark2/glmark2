package org.linaro.glmark2;

import android.app.Activity;
import android.os.Bundle;
import android.opengl.GLSurfaceView;
import android.app.Dialog;
import android.app.AlertDialog;
import android.content.DialogInterface;

public class Glmark2Activity extends Activity {
    public static final int DIALOG_EGLCONFIG_FAIL_ID = 0;

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

