package de.dfki.av.eotcontrolapp.UI;

import android.app.Fragment;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Toast;

import com.unnamed.b.atv.model.TreeNode;
import com.unnamed.b.atv.view.AndroidTreeView;

import de.dfki.av.eotcontrolapp.Arg;
import de.dfki.av.eotcontrolapp.MQTT_Client;
import de.dfki.av.eotcontrolapp.MainActivity;
import de.dfki.av.eotcontrolapp.R;
import de.dfki.av.eotcontrolapp.UI.Dialog.CreateDialog;

public class SDCardSubpageOverview extends Fragment {

    private View mView;
    private TreeNode mParentTreeNode;
    private CreateDialog mCreateDialog = new CreateDialog();
    private String mCurrentSelectedPath;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        mView = inflater.inflate(R.layout.sdcard_subpage_overview, container, false);
        final LinearLayout sdcardPageManagementLinearLayout = (LinearLayout) mView.findViewById(R.id.container);
        final Button addBtn = (Button) mView.findViewById(R.id.addButton);
        final Button deleteBtn = (Button) mView.findViewById(R.id.deleteButton);

        //define SDCard root folder
        TreeNode rootTreeNode = TreeNode.root();
        mParentTreeNode = new TreeNode(new FileSystemTreeNodeModel(true, "/mnt/sdcard")).setViewHolder(new FileSystenTreeNodeIcon(getActivity()));
//        mParentTreeNode.setClickListener(new TreeNode.TreeNodeClickListener() {
//            @Override
//            public void onClick(TreeNode node, Object value) {
//                FileSystemTreeNodeModel model = (FileSystemTreeNodeModel) value;
//                if (model.isDirectory) {
//                    addBtn.setEnabled(true);
//                } else {
//                    addBtn.setEnabled(false);
//                }
//            }
//        });
        rootTreeNode.addChild(mParentTreeNode);
        final AndroidTreeView treeView = new AndroidTreeView(getActivity(), rootTreeNode);
        sdcardPageManagementLinearLayout.addView(treeView.getView());

        MQTT_Client mqttClient = (MQTT_Client) MainActivity.Args.get(Arg.MQTT_CLIENT);
        mqttClient.getFileSystemStructure("/mnt/sdcard", new MQTT_Client.OnResultListener() {
            @Override
            public void onSuccess(@Nullable Object result) {
                String[] paths = (String[]) result;
                for (int i = 0; i < paths.length; i++) {
                    int k = 0;
                    while (paths[i].charAt(k) != ';') {
                        k++;
                    }
                    if (paths[i].charAt(k + 1) == '1') {
                        // File
                        mParentTreeNode.addChild(
                            new TreeNode(new FileSystemTreeNodeModel(false, paths[i].substring(0, k))).setViewHolder(new FileSystenTreeNodeIcon(getActivity()))
                        );
                    } else {
                        // Folder
                        mParentTreeNode.addChild(
                            new TreeNode(new FileSystemTreeNodeModel(true, paths[i].substring(0, k))).setViewHolder(new FileSystenTreeNodeIcon(getActivity()))
                        );
                    }
                }
            }
            @Override
            public void onFailure(@Nullable Object result) {
                Toast message = Toast.makeText(getActivity(), (String) result, Toast.LENGTH_LONG);
                message.show();
            }
        });


        addBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mCreateDialog.setOnCreateListener(new CreateDialog.OnCreateListener() {
                    @Override
                    public void onCreate(String name) {

                    }
                });
                mCreateDialog.show(getFragmentManager(), "Dialog Create");
            }
        });

        return mView;
    }
    @Override
    public void onStart() {
        super.onStart();
    }
    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
    }

}