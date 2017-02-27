package de.dfki.av.eotcontrolapp.UI.Dialog;

import android.app.Dialog;
import android.app.DialogFragment;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;


import de.dfki.av.eotcontrolapp.R;

public class CreateDialog extends DialogFragment {

    public interface OnCreateListener {
        void onCreate(String name);
    }

    private OnCreateListener m_callback;

    public void setOnCreateListener(OnCreateListener callback) {
        m_callback = callback;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {

        // Use the Builder class for convenient dialog construction
        Dialog fileDialog = new Dialog(getActivity());
        fileDialog.setContentView(R.layout.dialog_create);
        fileDialog.setCancelable(true);
        fileDialog.setCanceledOnTouchOutside(true);

        final EditText nameEditText = (EditText) fileDialog.findViewById(R.id.name_et);
        final Button okBtn = (Button) fileDialog.findViewById(R.id.ok_btn);
        okBtn.setEnabled(false);

        nameEditText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }
            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }
            @Override
            public void afterTextChanged(Editable s) {
                if(s.toString().length() > 0) {
                    okBtn.setEnabled(true);
                }
                else {
                    okBtn.setEnabled(false);
                }
            }
        });

        okBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (null != m_callback) {
                    m_callback.onCreate(nameEditText.getText().toString() );
                }
                dismiss();
            }
        });

        return fileDialog;
    }
}
