package de.dfki.av.eotcontrolapp.UI;


import android.content.Context;
import android.support.annotation.LayoutRes;
import android.support.annotation.NonNull;
import android.widget.ArrayAdapter;

import de.dfki.av.eotcontrolapp.Arg;
import de.dfki.av.eotcontrolapp.MainActivity;

public class MenuAdapter extends ArrayAdapter<String> {
    /**
     * Constructor
     *
     * @param context The current context.
     * @param resource The resource ID for a layout file containing a TextView to use when
     *                 instantiating views.
     * @param objects The objects to represent in the ListView.
     */
    public MenuAdapter(Context context, @LayoutRes int resource, @NonNull String[] objects) {
        super(context, resource, 0, objects);
    }

    @Override
    public boolean areAllItemsEnabled() {
        return false;
    }

    @Override
    public boolean isEnabled(int position) {
        if( ( null == MainActivity.Args || null == MainActivity.Args.get(Arg.MQTT_CLIENT) ) && 0 != position ) {
            return false;
        }
        return true;
    }

}
