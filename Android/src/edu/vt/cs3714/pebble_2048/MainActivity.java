package edu.vt.cs3714.pebble_2048;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.Date;
import java.util.Locale;
import java.util.Map;
import java.util.UUID;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.widget.ListView;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;

import edu.vt.cs3714.pebble_2048.R;

/**
 * Sample code demonstrating how Android applications can send+receive data using the 'Golf' app,
 * one of Pebble's built-in sports watch-apps.
 */
public class MainActivity extends Activity {
	private static final UUID uuid = UUID.fromString("9222ed46-176a-4e74-9e8d-50d70e754101");
    private PebbleKit.PebbleDataReceiver dataReceiver;
    private ArrayList<String[]> content;
    private ListView list;
    private ContentAdapter adapter;

    public static final String PREFS_NAME = "Pebble2048";
    
    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.scoreboard);
        if(content == null)
        	content = new ArrayList<String[]>();
        	
        if(adapter == null)
        	adapter = new ContentAdapter(this, android.R.layout.simple_list_item_1, content);

        loadCache();
        
        boolean connected = PebbleKit.isWatchConnected(getApplicationContext());
    	Log.i(getLocalClassName(), "Pebble is " + (connected ? "connected" : "not connected"));  
    	
        boolean supported = PebbleKit.areAppMessagesSupported(getApplicationContext());
    	Log.i(getLocalClassName(), "AppMessages are " + (supported ? "supported" : "not supported"));
    	
    }

    @Override
    protected void onStop() {
    	saveCache();
    	super.onStop();
    }
    @Override
    protected void onPause() {
        super.onPause();

        // Always deregister any Activity-scoped BroadcastReceivers when the Activity is paused
        if (dataReceiver != null) {
        	try{
                unregisterReceiver(dataReceiver);
        	}catch(Exception e){
        		// receiver is not registered
        	};
            dataReceiver = null;
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        startWatchApp();
        list = (ListView)this.findViewById(R.id.list);
        list.setAdapter(adapter);

        // In order to interact with the UI thread from a broadcast receiver, we need to perform any updates through
        // an Android handler. For more information, see: http://developer.android.com/reference/android/os/Handler.html
        final Handler handler = new Handler();

        // To receive data back from the sports watch-app, android
        // applications must register a "DataReceiver" to operate on the
        // dictionaries received from the watch.
        dataReceiver = new PebbleKit.PebbleDataReceiver(uuid) {
            @Override
            public void receiveData(final Context context, final int transactionId, final PebbleDictionary data) {
            	Log.i(getLocalClassName(), "Incoming Pebble AppMessage");
                final Long score = data.getUnsignedInteger(0);

                final String date = getTimestamp("MM/dd/yy HH:mm");
                final String[] score_data = {String.valueOf(score), date};

                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        // All data received from the Pebble must be ACK'd, otherwise you'll hit time-outs in the
                        // watch-app which will cause the watch to feel "laggy" during periods of frequent
                        // communication.
                        PebbleKit.sendAckToPebble(context, transactionId);
                        if(!content.contains(score_data)){
                        	addScoreData(score_data);
                        }
                    }
                });
            }
        };
        PebbleKit.registerReceivedDataHandler(getApplicationContext(), dataReceiver);
    }
    public void addScoreData(String[] score_data){
    	content.add(score_data);
    	adapter.sort(adapter.getComparator());
    	adapter.notifyDataSetChanged();
    }
	public void loadCache(){
		content.clear();
    	adapter.notifyDataSetChanged();
        SharedPreferences settings = getSharedPreferences(PREFS_NAME, 0);
        Map<String, String> map = (Map<String, String>) settings.getAll();
        for (Map.Entry<String, String> entry : map.entrySet())
        {
        	String score = entry.getKey().split(" ")[1];
            String date = entry.getValue();
            String[] arr = {score, date};
        	addScoreData(arr);
        }
	}
	public void saveCache(){
        SharedPreferences settings = getSharedPreferences(PREFS_NAME, 0);
        SharedPreferences.Editor editor = settings.edit();
        editor.clear();
        for(int i = 0; i < content.size(); i++){
        	editor.putString(i + " " + content.get(i)[0], content.get(i)[1]);
        }
        editor.commit();
	}

    // Send a broadcast to launch the specified application on the connected Pebble
    public void startWatchApp() {
        PebbleKit.startAppOnPebble(getApplicationContext(), uuid);
    }

    // Send a broadcast to close the specified application on the connected Pebble
    public void stopWatchApp() {
        PebbleKit.closeAppOnPebble(getApplicationContext(), uuid);
    }
    private String getTimestamp(String format){
        SimpleDateFormat sdf = new SimpleDateFormat(format,Locale.US);
        String date = sdf.format(new Date());
        return date;
    }
}
