package de.dfki.av.eotcontrolapp.UI;

import android.app.ListFragment;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;

import de.dfki.av.eotcontrolapp.Arg;
import de.dfki.av.eotcontrolapp.MainActivity;
import de.dfki.av.eotcontrolapp.R;

public class Menu extends ListFragment implements GUI{
    public interface OnMenuSelectedListener {
        /** Called by Menu when a list item is selected */
        public void onMenuItemSelected(int position);
    }
    private final static String ARGS_CURRENT_POSITION = "Current Position";

    private OnMenuSelectedListener mCallback;
    private OnUpdateListener mUpdateGUICallback;
    private int mCurrentPosition = 0;

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        // This makes sure that the container activity has implemented
        // the callback interface. If not, it throws an exception.
        try {
            mCallback = (OnMenuSelectedListener) context;
        } catch (ClassCastException e) {
            throw new ClassCastException(context.toString()
                    + " must implement OnHeadlineSelectedListener");
        }
        try {
            mUpdateGUICallback = (OnUpdateListener) context;
        } catch (ClassCastException e) {
            throw new ClassCastException(context.toString()
                    + " must implement OnUpdateListener");
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setListAdapter(new MenuAdapter(getActivity(), android.R.layout.simple_list_item_activated_1, getResources().getStringArray(R.array.menu_list)));
    }
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        // If activity recreated (such as from screen rotate), restore attributes
        if (savedInstanceState != null) {
            mCurrentPosition = savedInstanceState.getInt(Menu.ARGS_CURRENT_POSITION);
        }

        return super.onCreateView(inflater, container, savedInstanceState);
    }

    @Override
    public void onStart() {
        super.onStart();
        // During startup, check if there are arguments passed to the fragment.
        // onStart is a good place to do this because the layout has already been
        // applied to the fragment at this point so we can safely call the method
        // below that sets the article text.
        mUpdateGUICallback.onUpdate();
    }

    @Override
    public void onListItemClick(ListView l, View v, int position, long id) {
        // Notify the parent activity of selected item
        mCallback.onMenuItemSelected(position);
        // Set the item as checked to be highlighted when in two-pane layout
        getListView().setItemChecked(position, true);

        mCurrentPosition = position;
    }
    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        // Save the current attributes in case we need to recreate the fragment
        outState.putInt(Menu.ARGS_CURRENT_POSITION, mCurrentPosition);
    }

    @Override
    public void update() {
        if(null == MainActivity.Args.get(Arg.MQTT_CLIENT)) {
            mCurrentPosition = 0;
        }
        // When in two-pane layout, set the listview to highlight the selected list item
        // (We do this during onStart because at the point the listview is available.)
//        if (getFragmentManager().findFragmentById(R.id.menu_fragment) != null) {
            getListView().setChoiceMode(ListView.CHOICE_MODE_SINGLE);
            // Set the item as checked to be highlighted when in two-pane layout
            getListView().setItemChecked(mCurrentPosition, true);
//        }
        ListView listView = getListView();
        for(int i=0; i<listView.getChildCount(); ++i) {
            listView.getChildAt(i).setEnabled( getListAdapter().isEnabled(i) );
        }
    }
}
