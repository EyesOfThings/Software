package de.dfki.av.eotcontrolapp.UI;

import android.app.Fragment;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;

import de.dfki.av.eotcontrolapp.Arg;
import de.dfki.av.eotcontrolapp.MQTT_Client;
import de.dfki.av.eotcontrolapp.MainActivity;
import de.dfki.av.eotcontrolapp.R;
import de.dfki.av.eotcontrolapp.UI.Dialog.FileDialog;
import de.dfki.av.eotcontrolapp.UI.Dialog.SDCardFolderDialog;

/**
 * Created by reiser on 25.01.16.
 */
public class SDCardSubpageUpload extends Fragment {
    private static File uploadFile = null;
    private static File selectSDCardFolder = null;

    private View mView;
    private FileDialog mFileDialog = new FileDialog();
    private SDCardFolderDialog mSDCardFolderDialog = new SDCardFolderDialog();


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        mView = inflater.inflate(R.layout.sdcard_subpage_upload, container, false);

        final TextView selectFileTextView = (TextView) mView.findViewById(R.id.select_android_file_tv);
        final Button selectFileBtn = (Button) mView.findViewById(R.id.select_file_btn);

        final TextView selectSDCardFolderTextView = (TextView) mView.findViewById(R.id.select_sdcard_folder_tv);
        final Button selectSDCardFolderBtn = (Button) mView.findViewById(R.id.select_folder_btn);

        final Button uploadBtn = (Button) mView.findViewById(R.id.upload_btn);

        mFileDialog.setFilter(null);
        mFileDialog.setOnSelectListener(new FileDialog.OnSelectListener() {
            @Override
            public void onSelect(File file) {
                SDCardSubpageUpload.uploadFile = file;
                selectFileTextView.setText( SDCardSubpageUpload.uploadFile.getAbsolutePath() );
            }
        });
        selectFileBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mFileDialog.show(getFragmentManager(), "Select File Dialog");
            }
        });


        mSDCardFolderDialog.setOnSelectListener(new SDCardFolderDialog.OnSelectListener() {
            @Override
            public void onSelect(File file) {
                SDCardSubpageUpload.selectSDCardFolder = file;
                selectSDCardFolderTextView.setText( SDCardSubpageUpload.selectSDCardFolder.getAbsolutePath() );
            }
        });
        selectSDCardFolderBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSDCardFolderDialog.show(getFragmentManager(), "Select SDCard Folder Dialog");
            }
        });



        uploadBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                MQTT_Client mqttClient = (MQTT_Client) MainActivity.Args.get(Arg.MQTT_CLIENT);
                mqttClient.uploadFile(SDCardSubpageUpload.uploadFile.getAbsolutePath(),
                                      SDCardSubpageUpload.selectSDCardFolder.getAbsolutePath()+ "/" + SDCardSubpageUpload.uploadFile.getName(),
                                      new MQTT_Client.OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object res) {
                        int result = (int)res;
                        if(result == 0) {
                            Toast message = Toast.makeText(getActivity(), "File sent.", Toast.LENGTH_LONG);
                            message.show();

                        }
                        else {
                            Toast message = Toast.makeText(getActivity(), "Error sending the file.", Toast.LENGTH_LONG);
                            message.show();
                        }

                    }

                    @Override
                    public void onFailure(@Nullable Object result) {
                        Toast message = Toast.makeText(getActivity(), (String) result, Toast.LENGTH_LONG);
                        message.show();
                    }
                });
            }
        });


        if(null != SDCardSubpageUpload.uploadFile) {
            selectFileTextView.setText( SDCardSubpageUpload.uploadFile.getAbsolutePath() );
        }
        if(null != SDCardSubpageUpload.selectSDCardFolder) {
            selectSDCardFolderTextView.setText( SDCardSubpageUpload.selectSDCardFolder.getAbsolutePath() );
        }

        if(null != savedInstanceState) {
        }
        return mView;
    }
    @Override
    public void onStart() {
        super.onStart();
    }
    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
    }
}
