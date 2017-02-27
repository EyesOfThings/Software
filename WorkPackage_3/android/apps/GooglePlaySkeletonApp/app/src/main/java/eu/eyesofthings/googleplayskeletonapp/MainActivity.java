package eu.eyesofthings.googleplayskeletonapp;

import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentTransaction;

import android.os.Bundle;

import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.widget.Toast;

import java.io.IOException;
import java.util.HashMap;
import java.util.Timer;
import java.util.TimerTask;

import eu.eyesofthings.googleplayskeletonapp.UI.GUI;
import eu.eyesofthings.googleplayskeletonapp.UI.Menu;
import eu.eyesofthings.googleplayskeletonapp.UI.Page.ConnectPage;
import eu.eyesofthings.googleplayskeletonapp.UI.Page.CreditsPage;
import eu.eyesofthings.googleplayskeletonapp.UI.Page.StatusPage;
import eu.eyesofthings.googleplayskeletonapp.UI.Page.UploadPage;

public class MainActivity extends Activity
        implements Menu.OnMenuSelectedListener, GUI.OnUpdateListener,
        MQTT_Client.OnConnectionLostListener, MQTT_Client.OnErrorListener, MQTT_Client.OnMessageListener {

    public static HashMap<String, Object> Args;
    public static Menu mainMenu = null;

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
        else {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }

        // set layout
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
                    .add(R.id.one_pane_container, menuFragment).commit();

            MainActivity.mainMenu  = menuFragment;
        }
        else {
            MainActivity.mainMenu = (Menu) getFragmentManager().findFragmentById(R.id.menu_fragment);
        }


        if(null == MainActivity.Args) {
            MainActivity.Args = new HashMap<String, Object>();
            MainActivity.Args.put(getResources().getString(R.string.variableMQTTClient), null);
            onMenuItemSelected(0);
        }

        if(null != savedInstanceState) {
        }
    }

    // Menu callbacks
    @Override
    public void onMenuItemSelected(int position) {
        Bundle bundle = new Bundle();
        Fragment page;
        // Create fragment and give arguments
        switch(position) {
            case 1: // Upload App to EoT device
                page = new UploadPage();
                break;
            case 2: // Feedback
                page = new StatusPage();
                break;
            case 3: // Credits
                page = new CreditsPage();
                break;
            case 0: // Connect to EoT device
            default:
                page = new ConnectPage();
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
    // Menu callbacks
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
//                                Log.v("ClassCastException", e.toString());
                            }
                        } else {
                            try {
                                GUI fragment = (GUI) getFragmentManager().findFragmentById(R.id.menu_fragment);
                                fragment.update();
                            } catch (ClassCastException e) {
//                                Log.v("ClassCastException", e.toString());
                            }
                            try {
                                GUI fragment = (GUI) getFragmentManager().findFragmentById(R.id.two_pane_container);
                                fragment.update();
                            } catch (ClassCastException e) {
//                                Log.v("ClassCastException", e.toString());
                            }
                        }
                        mTimer = null;
                    }
                });

            }
        }, 100);
    }


    // MQTT_CLIENT callbacks
    @Override
    public void onError(String message) {
        Toast.makeText(getApplicationContext(), message, Toast.LENGTH_SHORT).show();
    }
    @Override
    public void onConnectionLost(Throwable cause) {
        MainActivity.Args.put(getResources().getString(R.string.variableMQTTClient), null);
        onMenuItemSelected(0);
    }
    @Override
    public void onIncomingMessage(String topic, String message) {
//        HashMap<String, ArrayList<String>> topics = (HashMap<String, ArrayList<String>>) MainActivity.Args.get(Arg.MQTT_TOPICS);
//        if(null == topics) {
//            topics = new HashMap<String, ArrayList<String>>();
//            MainActivity.Args.put(Arg.MQTT_TOPICS, topics);
//        }
//        ArrayList<String> messages = topics.get(topic);
//        if(null == messages) {
//            messages = new ArrayList<String>();
//        }
//        messages.add(message);
//        topics.put(topic, messages);
    }
}
