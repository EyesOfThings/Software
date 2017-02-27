package eu.eyesofthings.googleplayskeletonapp.UI.Page;

import android.app.Fragment;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Vector;
import java.util.regex.Pattern;

import eu.eyesofthings.googleplayskeletonapp.MQTT_Client;
import eu.eyesofthings.googleplayskeletonapp.MainActivity;
import eu.eyesofthings.googleplayskeletonapp.R;
import eu.eyesofthings.googleplayskeletonapp.UI.GUI;

public class UploadPage extends Fragment implements GUI {
    public static boolean UPLOADING = false;
    private static String ELF_FILE = null;
    private static Vector<String> FOLDERS = null;
    private static Vector<String> FILES = null;

    private static long TOTAL_BYTES_SIZE = 0;
    private static long SEND_BYTE_SIZE = 0;


    private OnUpdateListener mUpdateGUICallback;
    private View mView;

    protected Vector<String> getAssetFileNameList(String directory) {
        String[] tempList = null;
        try {
            tempList = getActivity().getAssets().list(directory);
        }
        catch( IOException e) {
        }
        Vector<String> assetList = new Vector<String>();
        for(String asset : tempList) {
            if( directory.isEmpty() ) {
                if(asset.equals("images") || asset.equals("sounds") || asset.equals("webkit")) {
                    continue;
                }
            }
            assetList.add(asset);
        }
        Vector<String> fileNameList = new Vector<String>();
        for( String asset : assetList ) {
            try {
                if( directory.isEmpty() ) {
                    getActivity().getAssets().open(asset).close();
                } else {
                    getActivity().getAssets().open(directory + "/" + asset).close();
                }
                fileNameList.add(asset);
            }
            catch( IOException e) {
            }
        }
        return fileNameList;
    }
    protected Vector<String> getAssetFolderNameList(String directory) {
        String[] tempList = null;
        try {
            tempList = getActivity().getAssets().list(directory);
        }
        catch( IOException e) {
        }
        Vector<String> assetList = new Vector<String>();
        for(String asset : tempList) {
            if( directory.isEmpty() ) {
                if(asset.equals("images") || asset.equals("sounds") || asset.equals("webkit")) {
                    continue;
                }
            }
            assetList.add(asset);
        }
        Vector<String> folderNameList = new Vector<String>();
        for( String asset : assetList ) {
            try {
                if( directory.isEmpty() ) {
                    getActivity().getAssets().open(asset).close();
                } else {
                    getActivity().getAssets().open(directory + "/" + asset).close();
                }
            }
            catch( IOException e) {
                folderNameList.add(asset);
            }
        }
        return folderNameList;
    }

    protected Vector<String> getAllSDCardFolders(String directory) {
        Vector<String> allFolders = new Vector<String>();
        allFolders.add(directory);

        Vector<String> subDirs = getAssetFolderNameList(directory);
        for(String subDir : subDirs) {
            Vector<String> subSubDirs = getAllSDCardFolders(directory + "/" + subDir);
            for(String subSubDir : subSubDirs) {
                allFolders.add(subSubDir);
            }
        }
        return allFolders;
    }
    protected Vector<String> getAllSDCardFiles(Vector<String> directories) {
        Vector<String> allFiles = new Vector<String>();
        for(String directory : directories) {
            Vector<String> files = getAssetFileNameList(directory);
            for(String file : files) {
                allFiles.add( directory + "/" + file );
            }
        }
        return allFiles;
    }
    protected long getFileSize(String filePath) {
        long sizeInBytes = 0;
        try {
            AssetFileDescriptor fileDescriptor = getActivity().getAssets().openFd(filePath);
            sizeInBytes = fileDescriptor.getLength();
            fileDescriptor.close();
        }
        catch( IOException e ) {
            try {
                InputStream inputStream = getActivity().getAssets().open(filePath);
                byte[] buffer = new byte[1024];
                int len = 0;
                while( (len = inputStream.read(buffer)) > 0 ) {
                    sizeInBytes += len;
                }
            }
            catch( IOException exception ) {
            }
        }
        return sizeInBytes;
    }
    protected long getSizeOfAllFiles() {
        long sizeOfAllFiles = 0;
        if(null != UploadPage.ELF_FILE) {
            long size = this.getFileSize(UploadPage.ELF_FILE);
            if (size <= 0) throw new AssertionError();
            sizeOfAllFiles += size;
        }
        if(null !=  UploadPage.FILES) {
            for(String file : UploadPage.FILES) {
                long size = this.getFileSize(file);
                if (size <= 0) throw new AssertionError();
                sizeOfAllFiles += size;
            }
        }
        return sizeOfAllFiles;
    }
    protected InputStream getInputStream(String filePath) {
        InputStream stream = null;
        try {
            stream = getActivity().getAssets().open(filePath);
        }
        catch( IOException e ) {
        }
        return stream;
    }

    protected void uploadingAllFiles(final MQTT_Client.OnResultListener callback) {
        if( UploadPage.TOTAL_BYTES_SIZE == UploadPage.SEND_BYTE_SIZE ) {
            callback.onSuccess(null);
        }
        else if( null == UploadPage.ELF_FILE && (null == UploadPage.FILES || 0 == UploadPage.FILES.size()) ) {
            callback.onFailure(null);
        }
        else if( null != UploadPage.ELF_FILE ) {
            final long elfSize = getFileSize(UploadPage.ELF_FILE);
            BufferedInputStream elfFileStream = new BufferedInputStream(
                    getInputStream(UploadPage.ELF_FILE)
            );
            UploadPage.ELF_FILE = null;
            MQTT_Client mqttClient = (MQTT_Client)MainActivity.Args.get(getResources().getString(R.string.variableMQTTClient));
            mqttClient.uploadProgram(elfFileStream, elfSize, new MQTT_Client.OnResultListener() {
                @Override
                public void onSuccess(@Nullable Object result) {
                    final ProgressBar statusProgressBar = (ProgressBar) mView.findViewById(R.id.upload_status_pb);
                    UploadPage.SEND_BYTE_SIZE += elfSize;
                    statusProgressBar.setProgress((int)UploadPage.SEND_BYTE_SIZE);
                    uploadingAllFiles(callback);
                }
                @Override
                public void onFailure(@Nullable Object result) {
                    callback.onFailure(null);
                }
            });
        }
        else if( null != UploadPage.FOLDERS && 0 < UploadPage.FOLDERS.size() ) {
            String folder = UploadPage.FOLDERS.remove(0);
            if( UploadPage.FOLDERS.size() == 0 ) {
                UploadPage.FOLDERS = null;
            }
            MQTT_Client mqttClient = (MQTT_Client)MainActivity.Args.get(getResources().getString(R.string.variableMQTTClient));
            mqttClient.createFolder("/mnt/" + folder, new MQTT_Client.OnResultListener() {
                @Override
                public void onSuccess(@Nullable Object result) {
                    uploadingAllFiles(callback);
                }
                @Override
                public void onFailure(@Nullable Object result) {
                    callback.onFailure(null);
                }
            });
        }
        else if( null != UploadPage.FILES && 0 < UploadPage.FILES.size() ) {
            String file = UploadPage.FILES.remove(0);
            final long fileSize = getFileSize(file);
            BufferedInputStream fileStream = new BufferedInputStream(
                    getInputStream(file)
            );
            if( UploadPage.FILES.size() == 0 ) {
                UploadPage.FILES = null;
            }
            MQTT_Client mqttClient = (MQTT_Client)MainActivity.Args.get(getResources().getString(R.string.variableMQTTClient));
            mqttClient.uploadFile(fileStream, fileSize, "/mnt/" + file, new MQTT_Client.OnResultListener() {
                @Override
                public void onSuccess(@Nullable Object result) {
                    final ProgressBar statusProgressBar = (ProgressBar) mView.findViewById(R.id.upload_status_pb);
                    UploadPage.SEND_BYTE_SIZE += fileSize;
                    statusProgressBar.setProgress((int)UploadPage.SEND_BYTE_SIZE);
                    uploadingAllFiles(callback);
                }
                @Override
                public void onFailure(@Nullable Object result) {
                    callback.onFailure(null);
                }
            });
        }
    }

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
        mView = inflater.inflate(R.layout.upload_page, container, false);

        final ProgressBar statusProgressBar = (ProgressBar) mView.findViewById(R.id.upload_status_pb);
        final TextView descriptionTextView = (TextView) mView.findViewById(R.id.description_tv);
        final Button uploadBtn = (Button) mView.findViewById(R.id.upload_btn);

        // show description
        try {
            BufferedReader fileInputStream = new BufferedReader( new InputStreamReader( getActivity().getAssets().open("description.txt") ) );
            String line = "";
            StringBuilder builder = new StringBuilder();
            while ((line = fileInputStream.readLine()) != null) {
                builder.append(line);
            }
            fileInputStream.close();
            descriptionTextView.setText( builder.toString() );
        }
        catch( IOException e) {
            descriptionTextView.setText("");
        }

        // search ELF file
        if(null == UploadPage.ELF_FILE) {
            Vector<String> files = getAssetFileNameList("");
            for(String file : files) {
                if( Pattern.matches("^.*\\.elf$", file) ) {
                    UploadPage.ELF_FILE = file;
                    break;
                }
            }
        }

        // collect sdcard folders structure
        if(null == UploadPage.FOLDERS) {
            UploadPage.FOLDERS = getAllSDCardFolders( "sdcard" );
        }
        // collect sdcard files structure
        if(null == UploadPage.FILES) {
            UploadPage.FILES = getAllSDCardFiles(UploadPage.FOLDERS);
        }

        // add all files sizes and the elf app size together in bytes
        if(0 == UploadPage.TOTAL_BYTES_SIZE) {
            UploadPage.TOTAL_BYTES_SIZE = getSizeOfAllFiles();
        }

        if(null == UploadPage.ELF_FILE || UploadPage.TOTAL_BYTES_SIZE == UploadPage.SEND_BYTE_SIZE) {
            uploadBtn.setEnabled(false);
        }

        statusProgressBar.setMax((int)UploadPage.TOTAL_BYTES_SIZE);
        statusProgressBar.setVisibility(View.INVISIBLE);

        uploadBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                statusProgressBar.setVisibility(View.VISIBLE);
                UploadPage.UPLOADING = true;
                uploadBtn.setEnabled(false);
                mUpdateGUICallback.onUpdate();
                uploadingAllFiles(new MQTT_Client.OnResultListener() {
                    @Override
                    public void onSuccess(@Nullable Object result) {
                        statusProgressBar.setVisibility(View.INVISIBLE);
                        UploadPage.UPLOADING = false;
                        StatusPage.hasFeedback = true;
                        StatusPage.isSuccessful = true;
                        uploadBtn.setEnabled(true);
                        mUpdateGUICallback.onUpdate();
                        MainActivity.mainMenu.onListItemClick(null, null, 2, 0);
                    }
                    @Override
                    public void onFailure(@Nullable Object result) {
                        statusProgressBar.setVisibility(View.INVISIBLE);
                        UploadPage.UPLOADING = false;
                        StatusPage.hasFeedback = true;
                        StatusPage.isSuccessful = false;
                        uploadBtn.setEnabled(true);
                        mUpdateGUICallback.onUpdate();
                        MainActivity.mainMenu.onListItemClick(null, null, 2, 0);
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
    }
}
