package de.dfki.av.eotcontrolapp.UI;

import android.app.Fragment;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Set;

import de.dfki.av.eotcontrolapp.Arg;
import de.dfki.av.eotcontrolapp.MQTT_Client;
import de.dfki.av.eotcontrolapp.MainActivity;
import de.dfki.av.eotcontrolapp.R;

public class MQTTSubpagePublish extends Fragment {
    private static final String ARG_SPINNER_POSITION = "Spinner Position";
    private static final String ARG_Publish_TEXT = "Publish Text";
    private View mView;
    private String mText = new String();
    private int mSpinnerPosition;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        mView = inflater.inflate(R.layout.mqtt_subpage_publish, container, false);

        final Spinner selectTopics = (Spinner) mView.findViewById(R.id.spinner);
        final EditText messageEditText = (EditText) mView.findViewById(R.id.publish_message_et);
        final Button publishBtn = (Button) mView.findViewById(R.id.publish_btn);

        selectTopics.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mSpinnerPosition = position;
                update();
            }
            @Override
            public void onNothingSelected(AdapterView<?> parent) {
                mSpinnerPosition = 0;
                update();
            }
        });

        messageEditText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                mText = s.toString();
                update();
            }
        });

        publishBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                MQTT_Client mqttClient = (MQTT_Client) MainActivity.Args.get(Arg.MQTT_CLIENT);
                String topic = (String) selectTopics.getAdapter().getItem(mSpinnerPosition);

                mqttClient.publish(topic, 0, messageEditText.getText().toString().getBytes(), new MQTT_Client.OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                        mText = "";
                        update();
                    }
                    @Override
                    public void onFailure(@Nullable Object result) {
                        Toast message = Toast.makeText(getActivity(), (String)result, Toast.LENGTH_LONG);
                        message.show();
                    }
                });
            }
        });

        if(null != savedInstanceState) {
            mSpinnerPosition = savedInstanceState.getInt(MQTTSubpagePublish.ARG_SPINNER_POSITION, mSpinnerPosition);
            mText = savedInstanceState.getString(MQTTSubpagePublish.ARG_Publish_TEXT);
            messageEditText.setText(mText);
        }

        return mView;
    }
    @Override
    public void onStart() {
        super.onStart();

        update();
    }
    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        outState.putInt(MQTTSubpagePublish.ARG_SPINNER_POSITION, mSpinnerPosition);
        outState.putString(MQTTSubpagePublish.ARG_Publish_TEXT, mText);
    }

    protected void update() {
        final Spinner selectTopics = (Spinner) mView.findViewById(R.id.spinner);
        final EditText messageEditText = (EditText) mView.findViewById(R.id.publish_message_et);
        final Button publishBtn = (Button) mView.findViewById(R.id.publish_btn);

        HashMap<String, ArrayList<String>> topics = (HashMap<String, ArrayList<String>>) MainActivity.Args.get(Arg.MQTT_TOPICS);
        if( null == topics || topics.isEmpty() ) {
            // clean spinner
            selectTopics.setAdapter(new ArrayAdapter<String>(getActivity(), android.R.layout.simple_dropdown_item_1line, new String[0]));
        }
        else {
            selectTopics.setEnabled(true);
            // select spinner item
            Set<String> keys = topics.keySet();
            selectTopics.setAdapter(new ArrayAdapter<String>(getActivity(), android.R.layout.simple_dropdown_item_1line, keys.toArray(new String[keys.size()])));
            selectTopics.setSelection(mSpinnerPosition);
        }


        if( messageEditText.getText().toString().isEmpty() || (null == topics || topics.isEmpty())) {
            publishBtn.setEnabled(false);
        }
        else {
            publishBtn.setEnabled(true);
        }
    }
}
