package de.dfki.av.eotcontrolapp.UI;

import android.app.Fragment;
import android.content.Context;
import android.util.DisplayMetrics;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.Space;
import android.widget.TextView;

import com.unnamed.b.atv.model.TreeNode;

import de.dfki.av.eotcontrolapp.R;

public class FileSystenTreeNodeIcon extends TreeNode.BaseNodeViewHolder<FileSystemTreeNodeModel> {

    private FileSystemTreeNodeModel m_model;
    private TextView m_expandIcon;
    private View m_view;

    public FileSystenTreeNodeIcon(Context context) {
        super(context);
    }

    public int dpToPx(int dp) {
        DisplayMetrics displayMetrics = context.getResources().getDisplayMetrics();
        int px = Math.round(dp * (displayMetrics.xdpi / DisplayMetrics.DENSITY_DEFAULT));
        return px;
    }

    @Override
    public View createNodeView(TreeNode node, FileSystemTreeNodeModel model) {
        m_model = model;

        LayoutInflater inflater = LayoutInflater.from(context);
        m_view = inflater.inflate(R.layout.filesystem_tree_node_icon, null, false);

        ViewGroup.LayoutParams params = null;
        Space space = (Space) m_view.findViewById(R.id.space);
        params = space.getLayoutParams();
        params.width = (node.getLevel() - 1) * dpToPx(20);
        space.setLayoutParams( params );

        TextView nodeLabel = (TextView) m_view.findViewById(R.id.sdcard_icon_tree_node_label_tv);
        nodeLabel.setText(m_model.text);
        if (m_model.isDirectory) {
            nodeLabel.setCompoundDrawablesWithIntrinsicBounds(R.drawable.ic_folder_open, 0, 0, 0);
            m_expandIcon = (TextView) m_view.findViewById(R.id.sdcard_icon_tree_node_expand_tv);
            m_expandIcon.setCompoundDrawablesWithIntrinsicBounds(R.drawable.ic_keyboard_arrow_right, 0, 0, 0);
        } else {
            nodeLabel.setCompoundDrawablesWithIntrinsicBounds(R.drawable.ic_insert_drive_file, 0, 0, 0);
        }
        return m_view;
    }

    @Override
    public void toggle(boolean active) {
        if (m_model.isDirectory) {
            if (active) {
                m_expandIcon = (TextView) m_view.findViewById(R.id.sdcard_icon_tree_node_expand_tv);
                m_expandIcon.setCompoundDrawablesWithIntrinsicBounds(R.drawable.ic_keyboard_arrow_down, 0, 0, 0);
            } else {
                m_expandIcon = (TextView) m_view.findViewById(R.id.sdcard_icon_tree_node_expand_tv);
                m_expandIcon.setCompoundDrawablesWithIntrinsicBounds(R.drawable.ic_keyboard_arrow_right, 0, 0, 0);
            }
        }
        LinearLayout layout = (LinearLayout) m_view.findViewById(R.id.sdcard_icon_tree_node_ll);
        if (active) {
            layout.setBackgroundColor(0xFFFF4081);
        } else {
            layout.setBackgroundColor(0x00FFFFFF);
        }
    }
}