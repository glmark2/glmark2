/*
 * Copyright © 2012 Linaro Limited
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * glmark2 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * glmark2.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Alexandros Frantzis
 */
package org.linaro.glmark2;

import java.util.ArrayList;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Parcelable;
import android.text.SpannableString;
import android.text.style.ForegroundColorSpan;
import android.util.Log;
import android.view.inputmethod.EditorInfo;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

public class EditorActivity extends Activity {
    public static final int DIALOG_SCENE_NAME_ID = 0;
    public static final int DIALOG_SCENE_OPTION_ID = 1;

    private EditorItemAdapter adapter;
    private ArrayList<SceneInfo> sceneInfoList;
    private String[] sceneNames;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_editor);

        /* Get information about the available scenes */
        sceneInfoList = getSceneInfoList();
        sceneNames = getSceneNames();

        /* Read information sent by the main activity */
        final int benchmarkPos = this.getIntent().getIntExtra("benchmark-pos", 0);
        String benchmarkText = getIntent().getStringExtra("benchmark-text");
        if (benchmarkText.isEmpty())
            benchmarkText = sceneNames[0];

        /* Set up the save button */
        Button button = (Button) findViewById(R.id.saveButton);
        button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                String newBenchmarkText = getBenchmarkDescriptionText();
                Intent intent = new Intent();
                intent.putExtra("benchmark-text", newBenchmarkText);
                intent.putExtra("benchmark-pos", benchmarkPos);
                setResult(RESULT_OK, intent);
                finish();
            }
        });

        /* Set up list view */
        ListView lv = (ListView) findViewById(R.id.editorListView);
        adapter = new EditorItemAdapter(this, R.layout.benchmark_item,
                                        getEditorItemList(benchmarkText));
        lv.setAdapter(adapter);

        lv.setOnItemClickListener(new OnItemClickListener() {
            public void onItemClick(AdapterView<?> parentView, View childView, int position, long id) {
                Bundle bundle = new Bundle();
                bundle.putInt("item-pos", position);
                /* Show the right dialog, depending on the clicked list position */
                if (position == 0)
                    showDialog(DIALOG_SCENE_NAME_ID, bundle);
                else
                    showDialog(DIALOG_SCENE_OPTION_ID, bundle);
            }
        });

        lv.setOnItemLongClickListener(new OnItemLongClickListener() {
            public boolean onItemLongClick(AdapterView<?> parentView, View childView, int position, long id) {
                /* Reset the value of the long-clicked option */
                if (position > 0) {
                    EditorItem item = adapter.getItem(position);
                    item.value = null;
                    adapter.notifyDataSetChanged();
                }
                return true;
            }
        });
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle bundle) {
        final int itemPos = bundle.getInt("item-pos");
        Dialog dialog;

        switch (id) {
            case DIALOG_SCENE_NAME_ID:
                {
                AlertDialog.Builder builder = new AlertDialog.Builder(this);
                builder.setTitle("Pick a scene");
                builder.setItems(sceneNames, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int item) {
                        adapter.clear();
                        for (EditorItem ei: getEditorItemList(sceneNames[item]))
                            adapter.add(ei);
                        adapter.notifyDataSetChanged();
                        removeDialog(DIALOG_SCENE_NAME_ID);
                    }
                });
                dialog = builder.create();
                }
                break;

            case DIALOG_SCENE_OPTION_ID:
                {
                AlertDialog.Builder builder = new AlertDialog.Builder(this);
                final EditorItem item = adapter.getItem(itemPos);
                final EditText input = new EditText(this);
                if (item.value != null)
                    input.setText(item.value);

                input.setOnEditorActionListener(new OnEditorActionListener() {
                    public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                        if (actionId == EditorInfo.IME_ACTION_DONE ||
                            (event != null && event.getKeyCode() == KeyEvent.KEYCODE_ENTER &&
                             event.getAction() == KeyEvent.ACTION_UP))
                        {
                            item.value = v.getText().toString();
                            removeDialog(DIALOG_SCENE_OPTION_ID);
                        }
                        return true;
                    }
                });
                builder.setTitle(item.option.name + ": " + item.option.description);
                dialog = builder.create();
                ((AlertDialog)dialog).setView(input, 15, 6, 15, 6);
                }
                break;

            default:
                dialog = null;
                break;
        }

        return dialog;
    }

    /**
     * Gets the value of an option.
     *
     * @param benchArray an array of option strings ("opt=val")
     * @param opt the options to get the value of
     *
     * @return the value or null
     */
    private String getOptionValue(String[] benchArray, String opt) {
        String ret = null;

        /* Search from the end to the beginning */
        for (int n = benchArray.length - 1; n >= 0; n--) {
            String s = benchArray[n].trim();
            if (s.startsWith(opt + "=")) {
                int i = s.indexOf('=');
                if (i >= 0 && i + 1 < s.length()) {
                    ret = s.substring(i + 1).trim();
                    break;
                }
            }
        }

        return ret;
    }

    /**
     * Gets the benchmark description string of the current editing state.
     *
     * @return the string
     */
    private String getBenchmarkDescriptionText() {
        String ret = "";

        for (int i = 0; i < adapter.getCount(); i++) {
            /* Convert each list item to a proper string representation */
            EditorItem item = adapter.getItem(i);
            String s = "";

            /*
             * Append "opt=" if this is an option item, except the
             * "__custom__" item.
             */
            if (item.option != null && item.value != null &&
                !item.option.name.equals("__custom__"))
            {
                s += item.option.name + "=";
            }

            /*
             * Append the item value if this is not "__custom__".
             */
            if (item.value != null && !item.value.equals("__custom__"))
                s += item.value;

            /*
             * Append ":" to the description string if needed.
             */
            if (!s.isEmpty() && !ret.isEmpty())
                ret += ":";

            /* Append the item representation */
            ret += s;
        }

        return ret;
    }

    /**
     * Creates an EditorItem list from a benchmark description string.
     *
     * @param benchDesc the benchmark description string
     *
     * @return the list
     */
    private ArrayList<EditorItem> getEditorItemList(String benchDesc) {
        String[] benchArray = benchDesc.split(":");
        String benchName = benchArray[0].trim();

        if (benchName.isEmpty())
            benchName = "__custom__";

        /* Find SceneInfo from name */
        SceneInfo sceneInfo = null;
        for (SceneInfo si: sceneInfoList) {
            if (si.name.equals(benchName)) {
                sceneInfo = si;
                break;
            }
        }

        /* If we couldn't find a matching SceneInfo, use __custom__ */
        if (sceneInfo == null) {
            for (SceneInfo si: sceneInfoList) {
                if (si.name.equals("__custom__")) {
                    sceneInfo = si;
                    break;
                }
            }
        }

        ArrayList<EditorItem> l = new ArrayList<EditorItem>();

        /* Append items to the list */
        if (!sceneInfo.name.equals("__custom__")) {
            /* Append scene name item */
            l.add(new EditorItem(null, sceneInfo.name));

            /* Append scene option items */
            for (SceneInfo.Option opt: sceneInfo.options)
                l.add(new EditorItem(opt, getOptionValue(benchArray, opt.name)));
        }
        else {
            String desc = new String(benchDesc);
            if (desc.startsWith("__custom__"))
                desc = "";

            /* Append scene name item */
            l.add(new EditorItem(null, sceneInfo.name));

            /* Append scene option items (only one for __custom__) */
            for (SceneInfo.Option opt: sceneInfo.options)
                l.add(new EditorItem(opt, desc));
        }

        return l;
    }

    /**
     * Gets a list of information about the available scenes.
     *
     * @return the list
     */
    private ArrayList<SceneInfo> getSceneInfoList() {
        ArrayList<SceneInfo> l = new ArrayList<SceneInfo>();
        SceneInfo customSceneInfo = new SceneInfo("__custom__");
        customSceneInfo.addOption("__custom__", "Custom benchmark string", "");

        for (Parcelable p: getIntent().getParcelableArrayExtra("scene-info"))
            l.add((SceneInfo)p);

        /* Add the "__custom__" SceneInfo */
        l.add(customSceneInfo);

        return l;
    }

    /**
     * Gets the array of scene names.
     *
     * @return the array
     */
    private String[] getSceneNames() {
        ArrayList<String> l = new ArrayList<String>();

        for (SceneInfo si: sceneInfoList) {
            if (!si.name.isEmpty())
                l.add(si.name);
        }

        String[] a = new String[0];
        return l.toArray(a);
    }


    static private class EditorItem {
        SceneInfo.Option option;

        public EditorItem(SceneInfo.Option o, String value) {
            this.option = o;
            this.value = value;
        }

        public String value;
    }

    /**
     * A ListView adapter that creates list item views from EditorItems
     */
    private class EditorItemAdapter extends ArrayAdapter<EditorItem> {
        static final int VIEW_TYPE_SCENE_NAME = 0;
        static final int VIEW_TYPE_SCENE_OPTION = 1;
        static final int VIEW_TYPE_COUNT = 2;

        public ArrayList<EditorItem> items;

        public EditorItemAdapter(Context context, int textViewResourceId,
                                 ArrayList<EditorItem> items)
        {
            super(context, textViewResourceId, items);
            this.items = items;
        }

        @Override
        public int getItemViewType(int position) {
            if (position == 0)
                return VIEW_TYPE_SCENE_NAME;
            else
                return VIEW_TYPE_SCENE_OPTION;
        }

        @Override
        public int getViewTypeCount() {
            return VIEW_TYPE_COUNT;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (position == 0)
                return getViewScene(position, convertView);
            else
                return getViewOption(position, convertView);
        }

        private View getViewScene(int position, View convertView) {
            /* Get the view/widget to use */
            View v = convertView;
            if (v == null) {
                LayoutInflater vi = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                v = vi.inflate(R.layout.benchmark_item, null);
            }

            EditorItem item = items.get(position);

            TextView title = (TextView) v.findViewById(R.id.title);
            TextView summary = (TextView) v.findViewById(R.id.summary);

            if (title != null)
                title.setText(item.value);
            if (summary != null)
                summary.setText("The scene to use");

            return v;
        }

        private View getViewOption(int position, View convertView) {
            /* Get the view/widget to use */
            View v = convertView;
            if (v == null) {
                LayoutInflater vi = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                v = vi.inflate(R.layout.benchmark_item, null);
            }

            EditorItem item = items.get(position);

            TextView title = (TextView) v.findViewById(R.id.title);
            TextView summary = (TextView) v.findViewById(R.id.summary);
            boolean hasUserSetValue = item.value != null;
            String value = hasUserSetValue ? item.value : item.option.defaultValue;

            if (title != null) {
                /* If the option has been edited by the user show it with emphasis */
                SpannableString titleText = new SpannableString(item.option.name + " = " + value);
                ForegroundColorSpan span = new ForegroundColorSpan(hasUserSetValue ? Color.CYAN : Color.LTGRAY);
                titleText.setSpan(span, item.option.name.length() + " = ".length(), titleText.length(), 0);
                title.setText(titleText);
            }

            if (summary != null)
                summary.setText(item.option.description);

            return v;
        }
    }
}
