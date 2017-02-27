package de.dfki.av.eotcontrolapp.UI;

import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileFilter;

import de.dfki.av.eotcontrolapp.Arg;
import de.dfki.av.eotcontrolapp.MQTT_Client;
import de.dfki.av.eotcontrolapp.MainActivity;
import de.dfki.av.eotcontrolapp.R;
import de.dfki.av.eotcontrolapp.UI.Dialog.FileDialog;

public class AppPage extends Fragment implements GUI {
    private static final String ARG_SELECTED_APP = "Selected App";
    private OnUpdateListener mUpdateGUICallback;
    private View mView;

    private FileDialog mAppFileDialog = new FileDialog();
    private File mSelectedApp = null;

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
        mView = inflater.inflate(R.layout.app_page, container, false);

        final Button app_select_eot_btn = (Button) mView.findViewById(R.id.select_app_btn);
        final Button app_upload_eot_btn = (Button) mView.findViewById(R.id.upload_app_btn);

        mAppFileDialog.setOnSelectListener(new FileDialog.OnSelectListener() {
            @Override
            public void onSelect(File file) {
                mSelectedApp = file;
                mUpdateGUICallback.onUpdate();
            }
        });
        mAppFileDialog.setFilter(new FileFilter() {
            @Override
            public boolean accept(File pathname) {
                if (pathname.isDirectory()) {
                    return true;
                }
                if (pathname.isFile() &&
                        pathname.getName().toLowerCase().endsWith(".elf")) {
                    return true;
                }
                return false;
            }
        });


        app_select_eot_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mAppFileDialog.show(getFragmentManager(), "Elf Dialog File");
            }
        });

        app_upload_eot_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                MQTT_Client mqttClient = (MQTT_Client) MainActivity.Args.get(Arg.MQTT_CLIENT);
                app_upload_eot_btn.setEnabled(false);
                mqttClient.uploadProgram(mSelectedApp.getAbsolutePath(), new MQTT_Client.OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object res) {
                        app_upload_eot_btn.setEnabled(true);
                        int result = (int)res;
                        if(result == 0) {
                            Toast message = Toast.makeText(getActivity(), "App sent.", Toast.LENGTH_LONG);
                            message.show();

                        }
                        else {
                            Toast message = Toast.makeText(getActivity(), "Error sending the App.", Toast.LENGTH_LONG);
                            message.show();
                        }

                        //Toast message = Toast.makeText(getActivity(), "File sent.", Toast.LENGTH_LONG);
                        //message.show();
                        mUpdateGUICallback.onUpdate();
                    }

                    @Override
                    public void onFailure(@Nullable Object result) {
                        app_upload_eot_btn.setEnabled(true);
                        Toast message = Toast.makeText(getActivity(), (String) result, Toast.LENGTH_LONG);
                        message.show();
                    }
                });
            }
        });

        if(null != savedInstanceState) {
            mSelectedApp = (File) savedInstanceState.getSerializable(AppPage.ARG_SELECTED_APP);
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

        outState.putSerializable(AppPage.ARG_SELECTED_APP, mSelectedApp);
    }

    @Override
    public void update() {
        TextView app_name_lbl = (TextView) mView.findViewById(R.id.app_path_tv);
        Button app_upload_eot_btn = (Button) mView.findViewById(R.id.upload_app_btn);

        if( null == mSelectedApp ) {
            app_upload_eot_btn.setEnabled(false);
            app_name_lbl.setText("");
        }
        else {
            app_upload_eot_btn.setEnabled(true);
            app_name_lbl.setText(mSelectedApp.getAbsolutePath());
        }

    }
}

