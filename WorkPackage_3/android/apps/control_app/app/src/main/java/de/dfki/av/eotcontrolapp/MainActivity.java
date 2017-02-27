package de.dfki.av.eotcontrolapp;

import android.Manifest;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Timer;
import java.util.TimerTask;

import de.dfki.av.eotcontrolapp.UI.AppPage;
import de.dfki.av.eotcontrolapp.UI.CameraPage;
import de.dfki.av.eotcontrolapp.UI.LoginPage;
import de.dfki.av.eotcontrolapp.UI.MQTTPage;
import de.dfki.av.eotcontrolapp.UI.Menu;
import de.dfki.av.eotcontrolapp.UI.GUI;
import de.dfki.av.eotcontrolapp.UI.SDCardPage;
import de.dfki.av.eotcontrolapp.UI.TimePage;
import de.dfki.av.eotcontrolapp.UI.WiFiPage;

public class MainActivity extends Activity
        implements Menu.OnMenuSelectedListener, GUI.OnUpdateListener,
        MQTT_Client.OnConnectionLostListener, MQTT_Client.OnErrorListener, MQTT_Client.OnMessageListener {

    public static HashMap<String, Object> Args;
    private Timer mTimer = null;

    protected boolean isPhone() {
        return (getResources().getConfiguration().screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK) < Configuration.SCREENLAYOUT_SIZE_LARGE;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if( isPhone() ) {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        }

        setContentView(R.layout.activity_main);
        // Check whether the activity is using the layout version with
        // the fragment_container FrameLayout. If so, we must add the first fragment
        if (findViewById(R.id.one_pane_container) != null) {
            // However, if we're being restored from a previous state,
            // then we don't need to do anything and should return or else
            // we could end up with overlapping fragments.
            if (savedInstanceState != null) {
                return;
            }
            Menu menuFragment = new Menu();
            // In case this activity was started with special instructions from an Intent,
            // pass the Intent's extras to the fragment as arguments
            menuFragment.setArguments(getIntent().getExtras());
            // Add the fragment to the 'fragment_container' FrameLayout
            getFragmentManager().beginTransaction()
                    .add(R.id.one_pane_container, menuFragment).commit();;
        }
        if(null == MainActivity.Args) {
            MainActivity.Args = new HashMap<String, Object>();
            onMenuItemSelected(0);
        }

        if(null != savedInstanceState) {
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            // check permissions
            ArrayList<String> permissions = new ArrayList<String>();

            if( PackageManager.PERMISSION_GRANTED != checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) ) {
                permissions.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
            }
//            if( PackageManager.PERMISSION_GRANTED != checkSelfPermission(Manifest.permission.READ_PHONE_STATE) ) {
//                permissions.add(Manifest.permission.READ_PHONE_STATE);
//            }
            if( permissions.size() > 0 ) {
                requestPermissions(permissions.toArray(new String[permissions.size()]), 0);
            }
        }

    }

    @Override
    public void onMenuItemSelected(int position) {
        Bundle bundle = new Bundle();
        Fragment page;
        // Create fragment and give arguments
        switch(position) {
            case 0: // Login
                page = new LoginPage();
                break;
            case 1: // MQTTClientThread Client
                page = new MQTTPage();
                break;
            case 2: // WiFi
                page = new WiFiPage();
                break;
            case 3: // Time & Date
                page = new TimePage();
                break;
            case 4: // App
                page = new AppPage();
                break;
            case 5: // SD Card
                page = new SDCardPage();
                break;
            case 6: // Camera
                page = new CameraPage();
                break;
            default:
                page = new LoginPage();
                break;
        }
        page.setArguments(bundle);

        FragmentTransaction transaction = getFragmentManager().beginTransaction();
        if (findViewById(R.id.one_pane_container) != null) {
            // one-pane layout
            // If the fragment is not available, we're in the one-pane layout and must swap fragments...
            // Replace whatever is in the one_pane_container view with this fragment,
            // and add the transaction to the back stack so the user can navigate back
            transaction.replace(R.id.one_pane_container, page);
            transaction.addToBackStack(null);
        } else {
            // two-pane layout
            // Replace whatever is in the fragment_container view with this fragment,
            transaction.replace(R.id.two_pane_container, page);
        }
        // Commit the transaction
        transaction.commit();
    }

    @Override
    public void onUpdate() {
        if( null != mTimer ) {
            return;
        }
        mTimer = new Timer();
        mTimer.schedule(new TimerTask() {
            @Override
            public void run() {

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (findViewById(R.id.one_pane_container) != null) {
                            try {
                                GUI fragment = (GUI) getFragmentManager().findFragmentById(R.id.one_pane_container);
                                fragment.update();
                            } catch (ClassCastException e) {
                                Log.v("ClassCastException", e.toString());
                            }
                        } else {
                            try {
                                GUI fragment = (GUI) getFragmentManager().findFragmentById(R.id.menu_fragment);
                                fragment.update();
                            } catch (ClassCastException e) {
                                Log.v("ClassCastException", e.toString());
                            }
                            try {
                                GUI fragment = (GUI) getFragmentManager().findFragmentById(R.id.two_pane_container);
                                fragment.update();
                            } catch (ClassCastException e) {
                                Log.v("ClassCastException", e.toString());
                            }
                        }
                        mTimer = null;
                    }
                });

            }
        }, 100);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String[] permissions, int[] grantResults) {
        switch(requestCode) {
            case 0:
                for(int i=0; i<grantResults.length; ++i) {
                    if(grantResults[i] != PackageManager.PERMISSION_GRANTED) {
                        finish();
                    }
                }
                break;
            default:
                break;
        }
    }

    @Override
    public void onError(String message) {
        Toast.makeText(getApplicationContext(), message, Toast.LENGTH_SHORT).show();
    }
    @Override
    public void onConnectionLost(Throwable cause) {
        MainActivity.Args.put(Arg.MQTT_CLIENT, null);
        onMenuItemSelected(0);
    }
    @Override
    public void onIncomingMessage(String topic, String message) {
        HashMap<String, ArrayList<String>> topics = (HashMap<String, ArrayList<String>>) MainActivity.Args.get(Arg.MQTT_TOPICS);
        if(null == topics) {
            topics = new HashMap<String, ArrayList<String>>();
            MainActivity.Args.put(Arg.MQTT_TOPICS, topics);
        }
        ArrayList<String> messages = topics.get(topic);
        if(null == messages) {
            messages = new ArrayList<String>();
        }
        messages.add(message);
        topics.put(topic, messages);
    }
}
