package de.dfki.av.eotcontrolapp.UI;

import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import de.dfki.av.eotcontrolapp.Arg;
import de.dfki.av.eotcontrolapp.MQTT_Client;
import de.dfki.av.eotcontrolapp.MainActivity;
import de.dfki.av.eotcontrolapp.R;

public class WiFiPage extends Fragment implements GUI {
    private final static String ARG_SSID = "SSID";
    private final static String ARG_PASSWORD = "Password";
    private final static String ARG_RADIO_ID = "Radio ID";
    private final static String ARG_CHANNEL_INDEX = "Channel Index";
    private final static String ARG_SECURITY_INDEX = "Security Index";

    private OnUpdateListener mUpdateGUICallback;
    private View mView;
    private String mSSID = new String();
    private String mPassword = new String();
    private int mSelectedRadioButtonId;
    private int mSelectedChannelIdx;
    private int mSelectedSecurityIdx;

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
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        mView = inflater.inflate(R.layout.wifi_page, container, false);

        final RadioGroup radioGroup = (RadioGroup) mView.findViewById(R.id.radioGroup);
        final EditText ssidEditText = (EditText) mView.findViewById(R.id.ssid_et);
        final EditText passwordEditText = (EditText) mView.findViewById(R.id.password_et);
        final Spinner channelSpinner = (Spinner) mView.findViewById(R.id.channel_sp);
        final Spinner securitySpinner = (Spinner) mView.findViewById(R.id.security_sp);
        final Button configureBtn = (Button) mView.findViewById(R.id.configure_btn);
        final Button resetConfigBtn = (Button) mView.findViewById(R.id.reset_configure_btn);

        mSelectedRadioButtonId = radioGroup.getCheckedRadioButtonId();
        radioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                mSelectedRadioButtonId = checkedId;
                mUpdateGUICallback.onUpdate();
            }
        });
        ssidEditText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                mSSID = s.toString();
                mUpdateGUICallback.onUpdate();
            }
        });
        passwordEditText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }
            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }
            @Override
            public void afterTextChanged(Editable s) {
                mPassword = s.toString();
                mUpdateGUICallback.onUpdate();
            }
        });
        channelSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mSelectedChannelIdx = position;
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
                mSelectedChannelIdx = 0;
            }
        });
        securitySpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mSelectedSecurityIdx = position;
            }
            @Override
            public void onNothingSelected(AdapterView<?> parent) {
                mSelectedSecurityIdx = 0;
            }
        });

        configureBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                MQTT_Client mqttClient = (MQTT_Client) MainActivity.Args.get(Arg.MQTT_CLIENT);
                switch(mSelectedRadioButtonId) {
                    case R.id.create_ap_rb:
                        mqttClient.createAP(ssidEditText.getText().toString(),
                                Integer.toString(securitySpinner.getSelectedItemPosition()),
                                passwordEditText.getText().toString(),
                                Integer.toString(channelSpinner.getSelectedItemPosition() + 1),
                                new MQTT_Client.OnResultListener() {
                                    @Override
                                    public void onSuccess(@Nullable Object result) {
                                        configureBtn.setEnabled(false);
                                    }
                                    @Override
                                    public void onFailure(@Nullable Object result) {
                                        Toast toast = Toast.makeText(getActivity(), (String)result, Toast.LENGTH_LONG);
                                        toast.show();
                                    }
                                });
                        break;
                    case R.id.connect_to_ap_rb:
                        mqttClient.connectToAP(ssidEditText.getText().toString(),
                                Integer.toString(securitySpinner.getSelectedItemPosition()),
                                passwordEditText.getText().toString(),
                                new MQTT_Client.OnResultListener() {
                                    @Override
                                    public void onSuccess(@Nullable Object result) {
                                        configureBtn.setEnabled(false);
                                    }

                                    @Override
                                    public void onFailure(@Nullable Object result) {
                                        Toast toast = Toast.makeText(getActivity(), (String) result, Toast.LENGTH_LONG);
                                        toast.show();
                                    }
                                });
                        break;
                    default:
                        break;
                }
            }
        });
        resetConfigBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                MQTT_Client mqttClient = (MQTT_Client) MainActivity.Args.get(Arg.MQTT_CLIENT);
                mqttClient.resetAPConfig(new MQTT_Client.OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                        resetConfigBtn.setEnabled(false);
                    }
                    @Override
                    public void onFailure(@Nullable Object result) {
                        Toast toast = Toast.makeText(getActivity(), (String) result, Toast.LENGTH_LONG);
                        toast.show();
                    }
                });

            }
        });


        if(null != savedInstanceState) {
            mSSID = savedInstanceState.getString(WiFiPage.ARG_SSID);
            mPassword = savedInstanceState.getString(WiFiPage.ARG_PASSWORD);
            mSelectedRadioButtonId = savedInstanceState.getInt(WiFiPage.ARG_RADIO_ID);
            mSelectedChannelIdx = savedInstanceState.getInt(WiFiPage.ARG_CHANNEL_INDEX);
            mSelectedSecurityIdx = savedInstanceState.getInt(WiFiPage.ARG_SECURITY_INDEX);

            radioGroup.check(mSelectedRadioButtonId);
            ssidEditText.setText(mSSID);
            passwordEditText.setText(mPassword);
            channelSpinner.setSelection(mSelectedChannelIdx);
            securitySpinner.setSelection(mSelectedSecurityIdx);
        }
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

        outState.putString(WiFiPage.ARG_SSID, mSSID);
        outState.putString(WiFiPage.ARG_PASSWORD, mPassword);
        outState.putInt(WiFiPage.ARG_RADIO_ID, mSelectedRadioButtonId);
        outState.putInt(WiFiPage.ARG_CHANNEL_INDEX, mSelectedChannelIdx);
        outState.putInt(WiFiPage.ARG_SECURITY_INDEX, mSelectedSecurityIdx);
    }

    @Override
    public void update() {

        final RadioGroup radioGroup = (RadioGroup) mView.findViewById(R.id.radioGroup);
        final EditText ssidEditText = (EditText) mView.findViewById(R.id.ssid_et);
        final EditText passwordEditText = (EditText) mView.findViewById(R.id.password_et);
        final Spinner channelSpinner = (Spinner) mView.findViewById(R.id.channel_sp);
        final Spinner securitySpinner = (Spinner) mView.findViewById(R.id.security_sp);
        final Button configureBtn = (Button) mView.findViewById(R.id.configure_btn);
        final Button resetConfigBtn = (Button) mView.findViewById(R.id.reset_configure_btn);

//        radioGroup.check(mSelectedRadioButtonId);
//        ssidEditText.setText(mSSID);
//        passwordEditText.setText(mPassword);
//        channelSpinner.setSelection(mSelectedChannelIdx);
//        securitySpinner.setSelection(mSelectedSecurityIdx);

        if(mSSID.isEmpty()) {
            configureBtn.setEnabled(false);
        }
        else {
            configureBtn.setEnabled(true);
        }
        resetConfigBtn.setEnabled(true);
    }
}
