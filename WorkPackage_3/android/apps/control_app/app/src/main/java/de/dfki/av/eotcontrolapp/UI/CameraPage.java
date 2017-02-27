package de.dfki.av.eotcontrolapp.UI;

import android.app.Fragment;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;

import de.dfki.av.eotcontrolapp.Arg;
import de.dfki.av.eotcontrolapp.MQTT_Client;
import de.dfki.av.eotcontrolapp.MainActivity;
import de.dfki.av.eotcontrolapp.R;

public class CameraPage  extends Fragment implements GUI {
    private static final String ARG_IMAGE = "Snapshot";
    private OnUpdateListener mUpdateGUICallback;
    private View mView;
    private Bitmap mSnapshot;

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
        mView = inflater.inflate(R.layout.camera_page, container, false);

        if(null != savedInstanceState) {
            mSnapshot = savedInstanceState.getParcelable(CameraPage.ARG_IMAGE);
        }
        Button doCameraSnapshotBtn = (Button) mView.findViewById(R.id.get_camera_snapshot_btn);
        doCameraSnapshotBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                MQTT_Client mqttClient = (MQTT_Client) MainActivity.Args.get(Arg.MQTT_CLIENT);
                mqttClient.askSnapshot(new MQTT_Client.OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                        byte[] imageData = (byte[]) result;
                        mSnapshot = BitmapFactory.decodeByteArray(imageData, 0, imageData.length);
                        mUpdateGUICallback.onUpdate();
                    }
                    @Override
                    public void onFailure(@Nullable Object result) {
                        Toast message = Toast.makeText(getActivity(), (String) result, Toast.LENGTH_LONG);
                        message.show();
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

        outState.putParcelable(CameraPage.ARG_IMAGE, mSnapshot);
    }

    @Override
    public void update() {

        ImageView imageView = (ImageView) mView.findViewById(R.id.camera_iv);
        imageView.setImageBitmap(mSnapshot);

    }
}