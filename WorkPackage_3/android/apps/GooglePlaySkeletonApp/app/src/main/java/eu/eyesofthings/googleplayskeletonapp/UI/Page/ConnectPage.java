package eu.eyesofthings.googleplayskeletonapp.UI.Page;

import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Switch;

import eu.eyesofthings.googleplayskeletonapp.MQTT_Client;
import eu.eyesofthings.googleplayskeletonapp.MainActivity;
import eu.eyesofthings.googleplayskeletonapp.R;
import eu.eyesofthings.googleplayskeletonapp.UI.GUI;

public class ConnectPage extends Fragment implements GUI {

    private OnUpdateListener mUpdateGUICallback;
    private View mView;

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
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        mView = inflater.inflate(R.layout.connect_page, container, false);

        Button connectButton = (Button) mView.findViewById(R.id.login_connect_btn);
        Switch settingsSwitch = (Switch) mView.findViewById(R.id.settings_sw);
        settingsSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                LinearLayout settingsLayout = (LinearLayout) mView.findViewById(R.id.settings_llayout);
                if(b) {
                    settingsLayout.setVisibility(View.VISIBLE);
                }
                else {
                    settingsLayout.setVisibility(View.INVISIBLE);
                }

            }
        });

        connectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                EditText brokerIpEditText = (EditText) mView.findViewById(R.id.login_broker_address_et);
                EditText brokerPortEditText = (EditText) mView.findViewById(R.id.login_broker_port_et);
                try {
                    final MQTT_Client mqttClient = new MQTT_Client(getActivity(), brokerIpEditText.getText().toString(), Integer.parseInt(brokerPortEditText.getText().toString()));
                    mqttClient.setOnConnectionLostListener((MQTT_Client.OnConnectionLostListener) getActivity());
                    mqttClient.setOnErrorListener((MQTT_Client.OnErrorListener) getActivity());
                    mqttClient.setOnMessageListener((MQTT_Client.OnMessageListener) getActivity());
                    mqttClient.connect(new MQTT_Client.OnResultListener() {
                        @Override
                        public void onSuccess(@Nullable Object result) {
                            MainActivity.Args.put(getResources().getString(R.string.variableMQTTClient), mqttClient);
                            mUpdateGUICallback.onUpdate();
                            MainActivity.mainMenu.onListItemClick(null, null, 1, 0);
                        }
                        @Override
                        public void onFailure(@Nullable Object result) {
//                            mUpdateGUICallback.onUpdate();
                        }
                    });
                    MainActivity.Args.put(getResources().getString(R.string.variableMQTTBrokerIp), brokerIpEditText.getText().toString());
                    MainActivity.Args.put(getResources().getString(R.string.variableMQTTBrokerPort), brokerPortEditText.getText().toString());
                }
                catch (Exception e) {
//                    Log.v("onClick", e.getMessage());
                }

            }
        });

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
    }

    @Override
    public void update() {
        MQTT_Client mqttClient = (MQTT_Client) MainActivity.Args.get(getResources().getString(R.string.variableMQTTClient));

        Button connectButton = (Button) mView.findViewById(R.id.login_connect_btn);
        EditText brokerIpEditText = (EditText) mView.findViewById(R.id.login_broker_address_et);
        EditText brokerPortEditText = (EditText) mView.findViewById(R.id.login_broker_port_et);

        String brokerIp = (String) MainActivity.Args.get(getResources().getString(R.string.variableMQTTBrokerIp));
        String brokerPort = (String) MainActivity.Args.get(getResources().getString(R.string.variableMQTTBrokerPort));
        if(null != brokerIp) {
            brokerIpEditText.setText(brokerIp);
        }
        if(null != brokerPort) {
            brokerPortEditText.setText(brokerPort);
        }

        if(null == mqttClient) {
            connectButton.setEnabled(true);
            brokerIpEditText.setEnabled(true);
            brokerPortEditText.setEnabled(true);
        } else {
            connectButton.setEnabled(false);
            brokerIpEditText.setEnabled(false);
            brokerPortEditText.setEnabled(false);
        }
    }
}
