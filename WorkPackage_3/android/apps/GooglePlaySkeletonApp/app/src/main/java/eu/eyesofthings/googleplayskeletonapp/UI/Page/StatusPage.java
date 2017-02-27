package eu.eyesofthings.googleplayskeletonapp.UI.Page;

import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import eu.eyesofthings.googleplayskeletonapp.R;
import eu.eyesofthings.googleplayskeletonapp.UI.GUI;

public class StatusPage extends Fragment implements GUI {

    public static boolean hasFeedback = false;
    public static boolean isSuccessful = false;

    private OnUpdateListener mUpdateGUICallback;
    private View mView;

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        // This makes sure that the container activity has implemented
        // the callback interface. If not, it throws an exception.
        try {
            mUpdateGUICallback = (OnUpdateListener) context;
        } catch (ClassCastException e) {
            throw new ClassCastException(context.toString()
                    + " must implement OnUpdateListener");
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        mView = inflater.inflate(R.layout.status_page, container, false);
        TextView successfulTextView = (TextView) mView.findViewById(R.id.successful);
        TextView failedTextView = (TextView) mView.findViewById(R.id.failed);

        if(StatusPage.isSuccessful) {
            failedTextView.setVisibility(View.GONE);
        }
        else {
            successfulTextView.setVisibility(View.GONE);
        }
        return mView;
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
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
    }

    @Override
    public void update() {
    }

}
