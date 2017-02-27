package de.dfki.av.eotcontrolapp.UI.Dialog;

import android.app.Dialog;
import android.app.DialogFragment;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.io.File;
import java.io.FileFilter;

import de.dfki.av.eotcontrolapp.Arg;
import de.dfki.av.eotcontrolapp.MQTT_Client;
import de.dfki.av.eotcontrolapp.MainActivity;
import de.dfki.av.eotcontrolapp.R;

public class SDCardDialog extends DialogFragment {

    public interface OnSelectListener {
        void onSelect(File file);
    }

    private final File m_root = new File("/mnt/sdcard");
    private File m_currentDir = new File("/mnt/sdcard");
    private OnSelectListener m_callback;

    private Button m_parentDirBtn;
    private TextView m_currentDirLbl;
    private LinearLayout m_dirLinearLayout;
    private LinearLayout m_fileLinearLayout;

    private File getParentFile() {
        if(m_currentDir.getAbsolutePath().equals( m_root.getAbsolutePath() ) ) {
            return null;
        }
        return m_currentDir.getParentFile();
    }

    public void setOnSelectListener(OnSelectListener callback) {
        m_callback = callback;
    }


    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {

        // Use the Builder class for convenient dialog construction
        Dialog fileDialog = new Dialog(getActivity());
        fileDialog.setContentView(R.layout.dialog_file);
        fileDialog.setCancelable(true);
        fileDialog.setCanceledOnTouchOutside(true);
        fileDialog.getWindow().setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);

        m_parentDirBtn = (Button) fileDialog.findViewById(R.id.dialog_file_parent_dir_btn);
        m_currentDirLbl = (TextView) fileDialog.findViewById(R.id.dialog_file_current_dir_lbl);
        m_dirLinearLayout = (LinearLayout) fileDialog.findViewById(R.id.dialog_file_dir_ll);
        m_fileLinearLayout = (LinearLayout) fileDialog.findViewById(R.id.dialog_file_file_ll);

        m_parentDirBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                File parent = getParentFile();
                if(null != parent) {
                    m_currentDir = parent;
                    updateDirListView();
                }
            }
        });

        updateDirListView();
        return fileDialog;
    }

    public void updateDirListView() {
        final String currentDir = m_currentDir.getAbsolutePath();

        m_dirLinearLayout.removeAllViews();
        m_fileLinearLayout.removeAllViews();

        if(null == getParentFile()) {
            m_parentDirBtn.setEnabled(false);
        }
        else {
            m_parentDirBtn.setEnabled(true);
        }

        m_currentDirLbl.setText( m_currentDir.getName() );


        MQTT_Client mqttClient = (MQTT_Client) MainActivity.Args.get(Arg.MQTT_CLIENT);
        mqttClient.getFileSystemStructure(currentDir, new MQTT_Client.OnResultListener() {
            @Override
            public void onSuccess(@Nullable Object result) {
                String[] paths = (String[]) result;
                for (int i = 0; i < paths.length; i++) {
                    int k = 0;
                    while (paths[i].charAt(k) != ';') {
                        k++;
                    }
                    if (paths[i].charAt(k + 1) != '1') {
                        // Folder
                        String folderName = paths[i].substring(0, k);
                        File tempFile = new File(currentDir+ "/" + folderName);
                        Button tmpButton = new FileDialogBtn( getActivity(), tempFile );

                        tmpButton.setLayoutParams( new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT) );
                        tmpButton.setText( tempFile.getName() );

                        tmpButton.setOnClickListener( new View.OnClickListener() {
                            @Override
                            public void onClick(View v) {
                                FileDialogBtn clickedBtn = (FileDialogBtn) v;
                                m_currentDir = clickedBtn.getFile();
                                updateDirListView();
                            }
                        });
                        m_dirLinearLayout.addView( tmpButton );
                    }
                    else {
                        // Files
                        String fileName = paths[i].substring(0, k);
                        File tempFile = new File(currentDir+ "/" + fileName);
                        Button tmpButton = new FileDialogBtn( getActivity(), tempFile );

                        tmpButton.setLayoutParams( new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT) );
                        tmpButton.setText( tempFile.getName() );

                        tmpButton.setOnClickListener(new View.OnClickListener() {
                            @Override
                            public void onClick(View v) {
                                FileDialogBtn clickedBtn = (FileDialogBtn) v;
                                if (null != m_callback) {
                                    m_callback.onSelect( clickedBtn.getFile() );
                                }
                                dismiss();
                            }
                        });
                        m_fileLinearLayout.addView( tmpButton );
                    }
                }
            }
            @Override
            public void onFailure(@Nullable Object result) {
            }
        });
    }
}
