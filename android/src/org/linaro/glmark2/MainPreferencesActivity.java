package org.linaro.glmark2;
 
import android.os.Bundle;
import android.preference.PreferenceActivity;
 
public class MainPreferencesActivity extends PreferenceActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.preferences);
    }
}
