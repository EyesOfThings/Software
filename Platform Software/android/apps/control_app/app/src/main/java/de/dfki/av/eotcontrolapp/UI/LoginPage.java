package de.dfki.av.eotcontrolapp.UI;


import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import de.dfki.av.eotcontrolapp.Arg;
import de.dfki.av.eotcontrolapp.MQTT_Client;
import de.dfki.av.eotcontrolapp.MainActivity;
import de.dfki.av.eotcontrolapp.R;

public class LoginPage extends Fragment implements GUI {

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
        mView = inflater.inflate(R.layout.login_page, container, false);

        Button connectButton = (Button) mView.findViewById(R.id.login_connect_btn);
        Button disconnectButton = (Button) mView.findViewById(R.id.login_disconnect_btn);

        connectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                EditText brokerIpEditText = (EditText) mView.findViewById(R.id.login_broker_address_et);
                EditText brokerPortEditText = (EditText) mView.findViewById(R.id.login_broker_port_et);
                try {
                    final MQTT_Client mqttClient = new MQTT_Client(getActivity(), brokerIpEditText.getText().toString(), Integer.parseInt(brokerPortEditText.getText().toString()));
                    mqttClient.setOnConnectionLostListener((MQTT_Client.OnConnectionLostListener) getActivity());
                    mqttClient.setOnErrorListener((MQTT_Client.OnErrorListener) getActivity());
                    mqttClient.connect(new MQTT_Client.OnResultListener() {
                        @Override
                        public void onSuccess(@Nullable Object result) {
                            MainActivity.Args.put(Arg.MQTT_CLIENT, mqttClient);
                            mUpdateGUICallback.onUpdate();
                        }
                        @Override
                        public void onFailure(@Nullable Object result) {
//                            mUpdateGUICallback.onUpdate();
                        }
                    });
                    MainActivity.Args.put(Arg.MQTT_BROKER_IP, brokerIpEditText.getText().toString());
                    MainActivity.Args.put(Arg.MQTT_BROKER_Port, brokerPortEditText.getText().toString());
                }
                catch (Exception e) {
                    Log.v("onClick", e.getMessage());
                }

            }
        });
        disconnectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                MQTT_Client mqttClient = (MQTT_Client) MainActivity.Args.get(Arg.MQTT_CLIENT);
                mqttClient.disconnect(new MQTT_Client.OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                        MainActivity.Args.put(Arg.MQTT_CLIENT, null);
                        MainActivity.Args.put(Arg.MQTT_BROKER_IP, null);
                        MainActivity.Args.put(Arg.MQTT_BROKER_Port, null);
                        mUpdateGUICallback.onUpdate();
                    }
                    @Override
                    public void onFailure(@Nullable Object result) {
                    }
                });


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
        MQTT_Client mqttClient = (MQTT_Client) MainActivity.Args.get(Arg.MQTT_CLIENT);

        Button connectButton = (Button) mView.findViewById(R.id.login_connect_btn);
        Button disconnectButton = (Button) mView.findViewById(R.id.login_disconnect_btn);
        EditText brokerIpEditText = (EditText) mView.findViewById(R.id.login_broker_address_et);
        EditText brokerPortEditText = (EditText) mView.findViewById(R.id.login_broker_port_et);

        String brokerIp = (String) MainActivity.Args.get(Arg.MQTT_BROKER_IP);
        String brokerPort = (String) MainActivity.Args.get(Arg.MQTT_BROKER_Port);
        if(null != brokerIp) {
            brokerIpEditText.setText(brokerIp);
        }
        if(null != brokerPort) {
            brokerPortEditText.setText(brokerPort);
        }

        if(null == mqttClient) {
            connectButton.setEnabled(true);
            disconnectButton.setEnabled(false);
            brokerIpEditText.setEnabled(true);
            brokerPortEditText.setEnabled(true);
        } else {
            connectButton.setEnabled(false);
            disconnectButton.setEnabled(true);
            brokerIpEditText.setEnabled(false);
            brokerPortEditText.setEnabled(false);
        }
    }
}
