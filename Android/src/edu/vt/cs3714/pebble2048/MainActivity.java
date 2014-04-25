
import java.util.ArrayList;
import java.util.UUID;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.widget.ListView;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;

import edu.vt.cs3714.pebble2048.R;

/**
 * Sample code demonstrating how Android applications can send+receive data using the 'Golf' app,
 * one of Pebble's built-in sports watch-apps.
 */
public class MainActivity extends Activity {
	private static final UUID uuid = UUID.fromString("9222ED46-176A-4E74-9E8D-50D70E754101");
    private PebbleKit.PebbleDataReceiver dataReceiver;
    private ArrayList<String[]> content;
    private ListView list;
    
    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.scoreboard);
        if(content == null)
        	content = new ArrayList<String[]>();

        list.setAdapter(new ContentAdapter(this, android.R.layout.simple_list_item_1, content));
    }

    @Override
    protected void onPause() {
        super.onPause();

        // Always deregister any Activity-scoped BroadcastReceivers when the Activity is paused
        if (dataReceiver != null) {
            unregisterReceiver(dataReceiver);
            dataReceiver = null;
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        list = (ListView)this.findViewById(R.id.list);

        //updateUi();

        // In order to interact with the UI thread from a broadcast receiver, we need to perform any updates through
        // an Android handler. For more information, see: http://developer.android.com/reference/android/os/Handler.html
        final Handler handler = new Handler();

        // To receive data back from the sports watch-app, android
        // applications must register a "DataReceiver" to operate on the
        // dictionaries received from the watch.
        //
        // In this example, we're registering a receiver to listen for
        // button presses sent from the watch, allowing us to page
        // through the holes displayed on the phone and watch.
        dataReceiver = new PebbleKit.PebbleDataReceiver(uuid) {
            @Override
            public void receiveData(final Context context, final int transactionId, final PebbleDictionary data) {
                final String score = data.getString(1);
                final String date = "now";
                final String[] score_data = {score, date};

                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        // All data received from the Pebble must be ACK'd, otherwise you'll hit time-outs in the
                        // watch-app which will cause the watch to feel "laggy" during periods of frequent
                        // communication.
                        PebbleKit.sendAckToPebble(context, transactionId);
                        content.add(score_data);
                    }
                });
            }
        };
        PebbleKit.registerReceivedDataHandler(this, dataReceiver);
    }

    // Send a broadcast to launch the specified application on the connected Pebble
    public void startWatchApp(View view) {
        PebbleKit.startAppOnPebble(getApplicationContext(), uuid);
    }

    // Send a broadcast to close the specified application on the connected Pebble
    public void stopWatchApp(View view) {
        PebbleKit.closeAppOnPebble(getApplicationContext(), uuid);
    }
}
