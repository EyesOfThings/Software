package de.dfki.av.eotcontrolapp.UI;

import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import org.w3c.dom.Text;

import java.util.Calendar;
import java.util.Timer;
import java.util.TimerTask;

import de.dfki.av.eotcontrolapp.Arg;
import de.dfki.av.eotcontrolapp.MQTT_Client;
import de.dfki.av.eotcontrolapp.MainActivity;
import de.dfki.av.eotcontrolapp.R;

public class TimePage extends Fragment implements GUI {
    private static final String ARG_EOT_DEVICE_TIME = "EoT Device Time";
    private static final String ARG_EOT_DEVICE_DATE = "EoT Device Date";

    private OnUpdateListener mUpdateGUICallback;
    private View mView;
    private String mEoTDeviceTime = new String();
    private String mEoTDeviceDate = new String();
    private Timer mTimer = new Timer();

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
        mView = inflater.inflate(R.layout.time_page, container, false);

        final TextView currentTimeTextView = (TextView) mView.findViewById(R.id.current_time_tv);
        final TextView currentDateTimeView = (TextView) mView.findViewById(R.id.current_date_tv);
        final Button setCurrentTimeBtn = (Button) mView.findViewById(R.id.set_current_time_btn);

        final TextView eotDeviceTimeTextView = (TextView) mView.findViewById(R.id.eot_device_current_time_tv);
        final TextView eotDeviceDateTextView = (TextView) mView.findViewById(R.id.eot_device_current_date_tv);
        final Button getDeviceTimeBtn = (Button) mView.findViewById(R.id.get_device_time_btn);


        mTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                getActivity().runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Calendar currentTimeDate = Calendar.getInstance();
                        currentTimeTextView.setText(
                                String.format("%02d:%02d:%02d", currentTimeDate.get(Calendar.HOUR_OF_DAY),
                                currentTimeDate.get(Calendar.MINUTE),
                                currentTimeDate.get(Calendar.SECOND))
                        );
                        currentDateTimeView.setText(
                                String.format("%02d.%02d.%4d", currentTimeDate.get(Calendar.DAY_OF_MONTH),
                                        currentTimeDate.get(Calendar.MONTH)+1,
                                        currentTimeDate.get(Calendar.YEAR))
                        );
                    }
                });
            }
        }, 0, 1000);

        setCurrentTimeBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Calendar currentTimeDate = Calendar.getInstance();
                String year = Integer.toString(currentTimeDate.get(Calendar.YEAR));
                String month = Integer.toString(currentTimeDate.get(Calendar.MONTH) + 1);
                String day = Integer.toString(currentTimeDate.get(Calendar.DAY_OF_MONTH));
                String hour = Integer.toString(currentTimeDate.get(Calendar.HOUR_OF_DAY));
                String minute = Integer.toString(currentTimeDate.get(Calendar.MINUTE));
                String second = Integer.toString(currentTimeDate.get(Calendar.SECOND));

                MQTT_Client mqttClient = (MQTT_Client) MainActivity.Args.get(Arg.MQTT_CLIENT);
                mqttClient.updateDate(year, month, day, hour, minute, second, new MQTT_Client.OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                        int status = ((Integer) result).intValue();
                        if (status != -1) {
                            Toast message = Toast.makeText(getActivity(), "Date updated.", Toast.LENGTH_LONG);
                            message.show();
                        } else {
                            Toast message = Toast.makeText(getActivity(), "Error updating the date.", Toast.LENGTH_LONG);
                            message.show();
                        }
                    }

                    @Override
                    public void onFailure(@Nullable Object result) {
                        Toast message = Toast.makeText(getActivity(), (String) result, Toast.LENGTH_LONG);
                        message.show();
                    }
                });
            }
        });

        getDeviceTimeBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                MQTT_Client mqttClient = (MQTT_Client) MainActivity.Args.get(Arg.MQTT_CLIENT);
                mqttClient.getDate(new MQTT_Client.OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                        Calendar deviceTimeDate = (Calendar) result;

                        mEoTDeviceTime = String.format("%02d:%02d:%02d", deviceTimeDate.get(Calendar.HOUR_OF_DAY),
                                deviceTimeDate.get(Calendar.MINUTE),
                                deviceTimeDate.get(Calendar.SECOND));
                        mEoTDeviceDate = String.format("%02d.%02d.%4d", deviceTimeDate.get(Calendar.DAY_OF_MONTH),
                                deviceTimeDate.get(Calendar.MONTH)+1,
                                deviceTimeDate.get(Calendar.YEAR));
                        eotDeviceTimeTextView.setText( mEoTDeviceTime );
                        eotDeviceDateTextView.setText( mEoTDeviceDate );
                    }
                    @Override
                    public void onFailure(@Nullable Object result) {
                        Toast message = Toast.makeText(getActivity(), (String) result, Toast.LENGTH_LONG);
                        message.show();
                    }
                });
            }
        });


        if(null != savedInstanceState) {
            mEoTDeviceTime = savedInstanceState.getString(TimePage.ARG_EOT_DEVICE_TIME);
            mEoTDeviceDate = savedInstanceState.getString(TimePage.ARG_EOT_DEVICE_DATE);
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
    public void onPause() {
        super.onPause();
        mTimer.cancel();
    }
    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        outState.putString(TimePage.ARG_EOT_DEVICE_TIME, mEoTDeviceTime);
        outState.putString(TimePage.ARG_EOT_DEVICE_DATE, mEoTDeviceDate);
    }

    @Override
    public void update() {
        final TextView eotDeviceTimeTextView = (TextView) mView.findViewById(R.id.eot_device_current_time_tv);
        final TextView eotDeviceDateTextView = (TextView) mView.findViewById(R.id.eot_device_current_date_tv);

        eotDeviceTimeTextView.setText(mEoTDeviceTime);
        eotDeviceDateTextView.setText(mEoTDeviceDate);
    }
}
