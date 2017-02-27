package de.dfki.av.eotcontrolapp.UI;

import android.app.Fragment;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;

import de.dfki.av.eotcontrolapp.Arg;
import de.dfki.av.eotcontrolapp.MQTT_Client;
import de.dfki.av.eotcontrolapp.MainActivity;
import de.dfki.av.eotcontrolapp.R;
import de.dfki.av.eotcontrolapp.UI.Dialog.FolderDialog;
import de.dfki.av.eotcontrolapp.UI.Dialog.SDCardDialog;

public class SDCardSubpageDownload extends Fragment {
    private static File sdcardFile = null;
    private static File downloadDir = null;

    private View mView;
    private SDCardDialog mSDCardFileDialog = new SDCardDialog();
    private FolderDialog mFolderDialog = new FolderDialog();

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        mView = inflater.inflate(R.layout.sdcard_subpage_download, container, false);

        final TextView selectSDCardFileTextView = (TextView) mView.findViewById(R.id.select_sdcard_file_tv);
        final Button selectSDCardFileBtn = (Button) mView.findViewById(R.id.select_sdcard_file_btn);

        final TextView selectDownloadDirTextView = (TextView) mView.findViewById(R.id.select_download_directory_tv);
        final Button selectDownloadDirBtn = (Button) mView.findViewById(R.id.download_location_btn);

        final Button downloadBtn = (Button) mView.findViewById(R.id.download_btn);

        mSDCardFileDialog.setOnSelectListener(new SDCardDialog.OnSelectListener() {
            @Override
            public void onSelect(File file) {
                SDCardSubpageDownload.sdcardFile = file;
                selectSDCardFileTextView.setText( SDCardSubpageDownload.sdcardFile.getAbsolutePath() );
            }
        });
        selectSDCardFileBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSDCardFileDialog.show(getFragmentManager(), "Select SDCard File Dialog");
            }
        });


        mFolderDialog.setOnSelectListener(new FolderDialog.OnSelectListener() {
            @Override
            public void onSelect(File file) {
                SDCardSubpageDownload.downloadDir = file;
                selectDownloadDirTextView.setText( SDCardSubpageDownload.downloadDir.getAbsolutePath() );
            }
        });
        selectDownloadDirBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mFolderDialog.show(getFragmentManager(), "Select Folder Dialog");
            }
        });

        downloadBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                MQTT_Client mqttClient = (MQTT_Client) MainActivity.Args.get(Arg.MQTT_CLIENT);
                mqttClient.downloadFile(sdcardFile.getAbsolutePath(), new MQTT_Client.OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                        byte[] data = (byte[]) result;
                        try {
                            FileOutputStream fos=new FileOutputStream( downloadDir.getAbsolutePath() + "/" + sdcardFile.getName() );
                            fos.write(data);
                            fos.close();

                            Toast message = Toast.makeText(getActivity(), "File downloaded.", Toast.LENGTH_LONG);
                            message.show();
                        }
                        catch (java.io.IOException e) {
                            Toast message = Toast.makeText(getActivity(), (String) e.getMessage(), Toast.LENGTH_LONG);
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
