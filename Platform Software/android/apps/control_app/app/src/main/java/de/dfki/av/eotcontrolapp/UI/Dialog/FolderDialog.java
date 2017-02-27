package de.dfki.av.eotcontrolapp.UI.Dialog;

import android.app.Dialog;
import android.app.DialogFragment;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.io.File;
import java.io.FileFilter;

import de.dfki.av.eotcontrolapp.R;

public class FolderDialog extends DialogFragment {

    public interface OnSelectListener {
        void onSelect(File file);
    }

    private File m_currentDir = Environment.getExternalStorageDirectory();
    private OnSelectListener m_callback;

    private Button m_parentDirBtn;
    private TextView m_currentDirLbl;
    private LinearLayout m_dirLinearLayout;
    private Button m_acceptBtn;

    private File getParentFile() {
        return m_currentDir.getParentFile();
    }
    public void setOnSelectListener(OnSelectListener callback) {
        m_callback = callback;
    }


    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {

        // Use the Builder class for convenient dialog construction
        Dialog fileDialog = new Dialog(getActivity());
        fileDialog.setContentView(R.layout.dialog_only_folder);
        fileDialog.setCancelable(true);
        fileDialog.setCanceledOnTouchOutside(true);
        fileDialog.getWindow().setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);

        m_parentDirBtn = (Button) fileDialog.findViewById(R.id.dialog_file_parent_dir_btn);
        m_currentDirLbl = (TextView) fileDialog.findViewById(R.id.dialog_file_current_dir_lbl);
        m_dirLinearLayout = (LinearLayout) fileDialog.findViewById(R.id.dialog_file_dir_ll);
        m_acceptBtn = (Button) fileDialog.findViewById(R.id.dialog_accept_btn);

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

        m_acceptBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (null != m_callback) {
                    m_callback.onSelect( m_currentDir );
                }
                dismiss();
            }
        });

        updateDirListView();
        return fileDialog;
    }

    public void updateDirListView() {

        m_dirLinearLayout.removeAllViews();

        if(null == getParentFile()) {
            m_parentDirBtn.setEnabled(false);
        }
        else {
            m_parentDirBtn.setEnabled(true);
        }

        m_currentDirLbl.setText( m_currentDir.getName() );


        File fileList[];
        fileList = m_currentDir.listFiles();
        if(null == fileList) {
            return;
        }
        // folder
        for(int i=0; i<fileList.length; ++i) {
            if( fileList[i].isDirectory() ) {
                Button newButton = new FileDialogBtn( getActivity(), fileList[i] );
                newButton.setLayoutParams( new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT) );
                newButton.setText( fileList[i].getName() );

                newButton.setOnClickListener( new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        FileDialogBtn clickedBtn = (FileDialogBtn) v;
                        m_currentDir = clickedBtn.getFile();
                        updateDirListView();
                    }
                });
                m_dirLinearLayout.addView( newButton );
            }
        }
    }
}
