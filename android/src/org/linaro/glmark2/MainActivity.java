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

import android.os.Bundle;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnDismissListener;
import android.content.Intent;
import android.widget.BaseAdapter;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Button;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.util.Log;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.AdapterView;

public class MainActivity extends Activity {
    public static final int DIALOG_BENCHMARK_ACTIONS_ID = 0;

    /**
     * The supported benchmark item actions.
     */
    public enum BenchmarkItemAction {
        EDIT, DELETE, CLONE, MOVEUP, MOVEDOWN
    }

    ArrayList<String> benchmarks;
    BaseAdapter adapter;
    SceneInfo[] sceneInfoList;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ArrayList<String> savedBenchmarks = null;

        if (savedInstanceState != null)
            savedBenchmarks = savedInstanceState.getStringArrayList("benchmarks");

        init(savedBenchmarks);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putStringArrayList("benchmarks", benchmarks);
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle bundle) {
        final CharSequence[] benchmarkActions = {"Delete", "Clone", "Move Up", "Move Down"};
        final BenchmarkItemAction[] benchmarkActionsId = {
                BenchmarkItemAction.DELETE, BenchmarkItemAction.CLONE,
                BenchmarkItemAction.MOVEUP, BenchmarkItemAction.MOVEDOWN
        };
        final int benchmarkPos = bundle.getInt("benchmark-pos");
        final int finalId = id;

        Dialog dialog;

        switch (id) {
            case DIALOG_BENCHMARK_ACTIONS_ID:
                AlertDialog.Builder builder = new AlertDialog.Builder(this);
                builder.setTitle("Pick an action");
                builder.setItems(benchmarkActions, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int item) {
                        doBenchmarkItemAction(benchmarkPos, benchmarkActionsId[item], null);
                        dismissDialog(DIALOG_BENCHMARK_ACTIONS_ID);
                    }
                });
                dialog = builder.create();
                break;

            default:
                dialog = null;
                break;
        }

        if (dialog != null) {
            dialog.setOnDismissListener(new OnDismissListener() {
                public void onDismiss(DialogInterface dialog) {
                    removeDialog(finalId);
                }
            });
        }

        return dialog;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.main_options_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        boolean ret = true;

        switch (item.getItemId()) {
            case R.id.save_benchmark_list:
                ret = true;
                break;
            case R.id.load_benchmark_list:
                ret = true;
                break;
            case R.id.about:
                ret = true;
                break;
            default:
                ret = super.onOptionsItemSelected(item);
                break;
        }

        return ret;
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == RESULT_OK) {
            String benchmarkText = data.getStringExtra("benchmark-text");
            int benchmarkPos = data.getIntExtra("benchmark-pos", 0);
            doBenchmarkItemAction(benchmarkPos, BenchmarkItemAction.EDIT, benchmarkText);
        }
    }

    /**
     * Initialize the activity.
     *
     * @param savedBenchmarks a list of benchmarks to load the list with (or null)
     */
    private void init(ArrayList<String> savedBenchmarks)
    {
        /* Fill in the benchmark list */
        if (savedBenchmarks == null) {
            benchmarks = new ArrayList<String>();
            benchmarks.add("Add benchmark...");
        }
        else {
            benchmarks = savedBenchmarks;
        }

        /* Get Scene information */
        sceneInfoList = Glmark2Native.getSceneInfo(getAssets());

        /* Set up the run button */
        Button button = (Button) findViewById(R.id.runButton);
        button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, Glmark2Activity.class);
                String args = "";
                for (int i = 0; i < benchmarks.size() - 1; i++)
                    args += "-b " + benchmarks.get(i) + " ";
                if (!args.isEmpty())
                    intent.putExtra("args", args);
                startActivity(intent);
            }
        });

        /* Set up the benchmark list view */
        ListView lv = (ListView) findViewById(R.id.benchmarkListView);
        adapter = new BenchmarkAdapter(this, R.layout.list_item, benchmarks);
        lv.setAdapter(adapter);

        lv.setOnItemClickListener(new OnItemClickListener() {
            public void onItemClick(AdapterView<?> parentView, View childView, int position, long id) {
                Intent intent = new Intent(MainActivity.this, EditorActivity.class);
                String t = benchmarks.get(position);
                if (position == benchmarks.size() - 1)
                    t = "";
                intent.putExtra("benchmark-text", t);
                intent.putExtra("benchmark-pos", position);
                intent.putExtra("scene-info", sceneInfoList);
                startActivityForResult(intent, 1);
            }
        });

        lv.setOnItemLongClickListener(new OnItemLongClickListener() {
            public boolean onItemLongClick(AdapterView<?> parentView, View childView, int position, long id) {
                if (position < benchmarks.size() - 1) {
                    Bundle bundle = new Bundle();
                    bundle.putInt("benchmark-pos", position);
                    showDialog(DIALOG_BENCHMARK_ACTIONS_ID, bundle);
                }
                return true;
            }
        });

    }

    /**
     * Perform an action on an listview benchmark item.
     *
     * @param position the position of the item in the listview
     * @param action the action to perform
     * @param data extra data needed by some actions
     */
    private void doBenchmarkItemAction(int position, BenchmarkItemAction action, String data)
    {
        int scrollPosition = position;

        switch(action) {
            case EDIT:
                if (position == benchmarks.size() - 1) {
                    benchmarks.add(position, data);
                    scrollPosition = position + 1;
                }
                else {
                    benchmarks.set(position, data);
                }
                break;
            case DELETE:
                benchmarks.remove(position);
                break;
            case CLONE:
                {
                    String s = benchmarks.get(position);
                    benchmarks.add(position, s);
                    scrollPosition = position + 1;
                }
                break;
            case MOVEUP:
                if (position > 0) {
                    String up = benchmarks.get(position - 1);
                    String s = benchmarks.get(position);
                    benchmarks.set(position - 1, s);
                    benchmarks.set(position, up);
                    scrollPosition = position - 1;
                }
                break;
            case MOVEDOWN:
                if (position < benchmarks.size() - 2) {
                    String down = benchmarks.get(position + 1);
                    String s = benchmarks.get(position);
                    benchmarks.set(position + 1, s);
                    benchmarks.set(position, down);
                    scrollPosition = position + 1;
                }
                break;
            default:
                break;
        }


        adapter.notifyDataSetChanged();

        /* Scroll the list view so that the item of interest remains visible */
        final int finalScrollPosition = scrollPosition;
        final ListView lv = (ListView) findViewById(R.id.benchmarkListView);
        lv.post(new Runnable() {
            @Override
            public void run() {
                lv.smoothScrollToPosition(finalScrollPosition);
            }
        });
    }

    /**
     * A ListView adapter that creates item views from benchmark strings.
     */
    private class BenchmarkAdapter extends ArrayAdapter<String> {
        private ArrayList<String> items;

        public BenchmarkAdapter(Context context, int textViewResourceId, ArrayList<String> items) {
            super(context, textViewResourceId, items);
            this.items = items;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            /* Get the view/widget to use */
            View v = convertView;
            if (v == null) {
                LayoutInflater vi = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                v = vi.inflate(R.layout.list_item, null);
            }

            /* Split the benchmark into its scene name and its options */
            String benchmark = items.get(position);
            String[] ba = benchmark.split(":", 2);

            if (ba != null) {
                TextView title = (TextView) v.findViewById(R.id.title);
                TextView summary = (TextView) v.findViewById(R.id.summary);
                title.setText("");
                summary.setText("");

                if (title != null && ba.length > 0)
                    title.setText(ba[0]);
                if (summary != null && ba.length > 1)
                    summary.setText(ba[1]);
            }
            return v;
        }
    }

    static {
        System.loadLibrary("glmark2-android");
    }
}
