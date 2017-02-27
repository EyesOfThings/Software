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

public class FileDialog extends DialogFragment {

    public interface OnSelectListener {
        void onSelect(File file);
    }

    private File m_root = Environment.getExternalStorageDirectory();
    private FileFilter m_filter;
    private OnSelectListener m_callback;

    private Button m_parentDirBtn;
    private TextView m_currentDirLbl;
    private LinearLayout m_dirLinearLayout;
    private LinearLayout m_fileLinearLayout;

    public void setRootDirectory(File root) {
        if( null != root && root.isDirectory() ) {
            m_root = root;
        }
    }
    public void setFilter(FileFilter filter) {
        m_filter = filter;
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
                File parent = m_root.getParentFile();
                if(null != parent) {
                    m_root = parent;
                    updateDirListView();
                }
            }
        });

        updateDirListView();
        return fileDialog;
    }

    public void updateDirListView() {
        m_dirLinearLayout.removeAllViews();
        m_fileLinearLayout.removeAllViews();

        if(null == m_root.getParentFile()) {
            m_parentDirBtn.setEnabled(false);
        }
        else {
            m_parentDirBtn.setEnabled(true);
        }


        File fileList[];
        if(null != m_filter) {
            fileList = m_root.listFiles(m_filter);
        }
        else {
            fileList = m_root.listFiles();
        }

        if(null == fileList) {
            return;
        }
        m_currentDirLbl.setText( m_root.getName() );

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
                        m_root = clickedBtn.getFile();
                        updateDirListView();
                    }
                });
                m_dirLinearLayout.addView( newButton );
            }
        }
        // files
        for(int i=0; i<fileList.length; ++i) {
            if( fileList[i].isFile() ) {
                Button newButton = new FileDialogBtn( getActivity(), fileList[i] );
                newButton.setLayoutParams( new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT) );
                newButton.setText(fileList[i].getName());

                newButton.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        FileDialogBtn clickedBtn = (FileDialogBtn) v;
                        if (null != m_callback) {
                            m_callback.onSelect( clickedBtn.getFile() );
                        }
                        dismiss();
                    }
                });
                m_fileLinearLayout.addView(newButton );
            }
        }
    }
}
