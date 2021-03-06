package edu.vt.cs3714.pebble_2048;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Comparator;
import java.util.Date;
import java.util.List;
import java.util.Locale;

import edu.vt.cs3714.pebble_2048.R;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

public class ContentAdapter extends ArrayAdapter<String[]> {
	
	public ContentAdapter(Context context, int textViewResourceId) {
	    super(context, textViewResourceId);
	}
	
	public ContentAdapter(Context context, int resource, List<String[]> items) {
	    super(context, resource, items);
	}
	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
	    View v = convertView;
	
	    if (v == null) {
	        LayoutInflater vi;
	        vi = LayoutInflater.from(getContext());
	        v = vi.inflate(R.layout.scoreboard_item, null);
	    }
	
	    String[] p = getItem(position);
	
	    if (p != null) {
	        TextView num = (TextView) v.findViewById(R.id.num);
	        num.setText(String.valueOf(position + 1));
	        
	        TextView score = (TextView) v.findViewById(R.id.score);
	        score.setText(p[0]);
	        
	        TextView date = (TextView) v.findViewById(R.id.date);
	        date.setText(p[1]);
	    }
	    
	    return v;
	}
	@Override
	public void notifyDataSetChanged() {
	    //do your sorting here
		
	    super.notifyDataSetChanged();
	}
	public Comparator getComparator(){
		return new Comparator<String[]>() {
		    @Override
		    public int compare(String[] lhs, String[] rhs) {
		    	int L = Integer.valueOf( lhs[0] );
		    	int R = Integer.valueOf( rhs[0] );
		    	if(L < R)
		    		return 1;
		    	if(L > R)
		    		return -1;
	    		return 0;
		    }
		};
	}
}
