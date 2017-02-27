package eu.eyesofthings.googleplayskeletonapp.UI;

import android.content.Context;
import android.support.annotation.LayoutRes;
import android.support.annotation.NonNull;
import android.widget.ArrayAdapter;

import eu.eyesofthings.googleplayskeletonapp.MainActivity;
import eu.eyesofthings.googleplayskeletonapp.R;
import eu.eyesofthings.googleplayskeletonapp.UI.Page.StatusPage;
import eu.eyesofthings.googleplayskeletonapp.UI.Page.UploadPage;

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
        if( true == UploadPage.UPLOADING) {
            return false;
        }
        if(2 == position) {
            return StatusPage.hasFeedback;
        }
        if(0 == position || 3 == position) {
            return true;
        }
        if( ( null == MainActivity.Args || null == MainActivity.Args.get(getContext().getResources().getString(R.string.variableMQTTClient)) ) ) {
            return false;
        }
        return true;
    }

}
