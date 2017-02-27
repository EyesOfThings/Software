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
import android.widget.TextView;
import android.widget.Toast;

import org.w3c.dom.Text;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Set;

import de.dfki.av.eotcontrolapp.Arg;
import de.dfki.av.eotcontrolapp.MQTT_Client;
import de.dfki.av.eotcontrolapp.MainActivity;
import de.dfki.av.eotcontrolapp.R;

public class MQTTSubpageSubscriptions extends Fragment {
    private static final String ARG_SPINNER_POSITION = "Spinner Position";
    private View mView;
    private int mSpinnerPosition;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        mView = inflater.inflate(R.layout.mqtt_subpage_subscriptions, container, false);

        final EditText topicEditText = (EditText) mView.findViewById(R.id.topic_et);
        final Button addTopicBtn = (Button) mView.findViewById(R.id.add_topic_btn);
        final Spinner selectTopics = (Spinner) mView.findViewById(R.id.select_topics_sp);
        final Button removeTopicBtn = (Button) mView.findViewById(R.id.remove_topic_btn);

        topicEditText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                if (s.toString().isEmpty()) {
                    addTopicBtn.setEnabled(false);
                } else {
                    HashMap<String, ArrayList<String>> topics = (HashMap<String, ArrayList<String>>) MainActivity.Args.get(Arg.MQTT_TOPICS);
                    if (null != topics && topics.containsKey(s.toString())) {
                        addTopicBtn.setEnabled(false);
                    } else {
                        addTopicBtn.setEnabled(true);
                    }
                }
            }
        });

        addTopicBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                MQTT_Client mqttClient = (MQTT_Client) MainActivity.Args.get(Arg.MQTT_CLIENT);

                mqttClient.subscribe(topicEditText.getText().toString(), 0, new MQTT_Client.OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                        HashMap<String, ArrayList<String>> topics = (HashMap<String, ArrayList<String>>) MainActivity.Args.get(Arg.MQTT_TOPICS);
                        if(null == topics) {
                            topics = new HashMap<String, ArrayList<String>>();
                            MainActivity.Args.put(Arg.MQTT_TOPICS, topics);
                        }
                        topics.put(topicEditText.getText().toString(), new ArrayList<String>());
                        topicEditText.setText("");
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

        removeTopicBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final HashMap<String, ArrayList<String>> topics = (HashMap<String, ArrayList<String>>) MainActivity.Args.get(Arg.MQTT_TOPICS);
                final String topicName = (String) selectTopics.getAdapter().getItem(mSpinnerPosition);
                MQTT_Client mqttClient = (MQTT_Client) MainActivity.Args.get(Arg.MQTT_CLIENT);
                mqttClient.unsubscribe(topicName, new MQTT_Client.OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                        topics.remove(topicName);
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
            mSpinnerPosition = savedInstanceState.getInt(MQTTSubpageSubscriptions.ARG_SPINNER_POSITION);
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

        outState.putInt(MQTTSubpageSubscriptions.ARG_SPINNER_POSITION, mSpinnerPosition);
    }

    protected void update() {
        EditText topicEditText = (EditText) mView.findViewById(R.id.topic_et);
        Button addTopicBtn = (Button) mView.findViewById(R.id.add_topic_btn);
        Spinner selectTopics = (Spinner) mView.findViewById(R.id.select_topics_sp);
        Button removeTopicBtn = (Button) mView.findViewById(R.id.remove_topic_btn);
        EditText receivedMessagesEditText = (EditText) mView.findViewById(R.id.received_messages);

        if( topicEditText.getText().toString().isEmpty() ) {
            addTopicBtn.setEnabled(false);
        }
        else {
            addTopicBtn.setEnabled(true);
        }

        HashMap<String, ArrayList<String>> topics = (HashMap<String, ArrayList<String>>) MainActivity.Args.get(Arg.MQTT_TOPICS);
        if( null == topics || topics.isEmpty() ) {
            removeTopicBtn.setEnabled(false);
            selectTopics.setEnabled(false);
            // clean spinner
            selectTopics.setAdapter(new ArrayAdapter<String>(getActivity(),android.R.layout.simple_dropdown_item_1line, new String[0]));
            // reset text field
            receivedMessagesEditText.setText("");
        }
        else {
            removeTopicBtn.setEnabled(true);
            selectTopics.setEnabled(true);
            // select spinner item
            Set<String> keys = topics.keySet();
            selectTopics.setAdapter(new ArrayAdapter<String>(getActivity(), android.R.layout.simple_dropdown_item_1line, keys.toArray(new String[keys.size()])));
            selectTopics.setSelection(mSpinnerPosition);
            // fill text field
            String currentSpinnerValue = (String) selectTopics.getAdapter().getItem(mSpinnerPosition);
            ArrayList<String> messages = topics.get(currentSpinnerValue);
            StringBuilder messagesInOneString = new StringBuilder();
            for(int i=0; i<messages.size(); ++i) {
                messagesInOneString.append(messages.get(i));
                messagesInOneString.append( "\n" );
            }
            receivedMessagesEditText.setText( messagesInOneString.toString() );
        }
    }
}
