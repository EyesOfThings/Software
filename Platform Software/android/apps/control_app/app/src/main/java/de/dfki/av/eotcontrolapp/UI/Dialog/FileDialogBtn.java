package de.dfki.av.eotcontrolapp.UI.Dialog;

import android.content.Context;
import android.widget.Button;

import java.io.File;

public class FileDialogBtn extends Button {

    private File m_file;

    public FileDialogBtn(Context context, File file) {
        super(context);
        setFile(file);
    }

    public File getFile() {
        return m_file;
    }

    public void setFile(File m_file) {
        this.m_file = m_file;
    }
}