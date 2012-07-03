package org.linaro.glmark2;

import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.KeyEvent;
import android.view.inputmethod.EditorInfo;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;
import android.content.Intent;
import android.view.WindowManager;

public class EditorActivity extends Activity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_editor);

        /* Show the soft-keyboard */
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);

        /* Get the benchmark position as sent by the main activity */
        final int benchmarkPos = this.getIntent().getIntExtra("benchmark-pos", 0);

        /* Set the textview widget text */
        TextView tv = (TextView) findViewById(R.id.benchmarkEditorTextView);
        tv.setText(getIntent().getStringExtra("benchmark-text"));

        /* Handle editor events */
        tv.setOnEditorActionListener(new OnEditorActionListener() {
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_DONE ||
                    (event != null && event.getKeyCode() == KeyEvent.KEYCODE_ENTER &&
                     event.getAction() == KeyEvent.ACTION_UP))
                {
                    /* Return result to caller activity */
                    Intent intent = new Intent();
                    intent.putExtra("benchmark-text", v.getText().toString());
                    intent.putExtra("benchmark-pos", benchmarkPos);
                    setResult(RESULT_OK, intent);
                    finish();
                }
                return true;
            }
        });
    }
}
