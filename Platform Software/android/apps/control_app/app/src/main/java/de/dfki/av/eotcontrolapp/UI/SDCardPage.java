package de.dfki.av.eotcontrolapp.UI;

import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Context;
import android.os.Bundle;
import android.support.design.widget.TabLayout;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import de.dfki.av.eotcontrolapp.R;

public class SDCardPage extends Fragment implements GUI {
    private static final String ARG_TAB_POSITION = "Tab Position";

    private OnUpdateListener mUpdateGUICallback;
    private View mView;
    private int mCurrentTabPosition;

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
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        mView = inflater.inflate(R.layout.sdcard_page, container, false);


        TabLayout tabLayout = (TabLayout) mView.findViewById(R.id.tabLayout);
        tabLayout.addTab(tabLayout.newTab().setText(getResources().getString(R.string.overview)));
        tabLayout.addTab(tabLayout.newTab().setText(getResources().getString(R.string.download)));
        tabLayout.addTab(tabLayout.newTab().setText(getResources().getString(R.string.upload)));
        tabLayout.setTabGravity(TabLayout.GRAVITY_FILL);
        tabLayout.setOnTabSelectedListener(new TabLayout.OnTabSelectedListener() {
            @Override
            public void onTabSelected(TabLayout.Tab tab) {
                mCurrentTabPosition = tab.getPosition();
                mUpdateGUICallback.onUpdate();
            }

            @Override
            public void onTabUnselected(TabLayout.Tab tab) {
            }

            @Override
            public void onTabReselected(TabLayout.Tab tab) {
            }
        });
        if (null != savedInstanceState) {
            mCurrentTabPosition = savedInstanceState.getInt(SDCardPage.ARG_TAB_POSITION);
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

        outState.putInt(SDCardPage.ARG_TAB_POSITION, mCurrentTabPosition);
    }

    @Override
    public void update() {
        Fragment page;
        switch (mCurrentTabPosition) {
            case 0:
                page = new SDCardSubpageOverview();
                break;
            case 1:
                page = new SDCardSubpageDownload();
                break;
            case 2:
                page = new SDCardSubpageUpload();
                break;
            default:
                page = new SDCardSubpageOverview();
                break;
        }
        FragmentTransaction transaction = getFragmentManager().beginTransaction();
        transaction.replace(R.id.container, page);
        // Commit the transaction
        transaction.commit();

        TabLayout tabLayout = (TabLayout) mView.findViewById(R.id.tabLayout);
        tabLayout.getTabAt(mCurrentTabPosition).select();
    }
}