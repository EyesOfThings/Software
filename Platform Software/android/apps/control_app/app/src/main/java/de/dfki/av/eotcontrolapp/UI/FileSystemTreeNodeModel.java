package de.dfki.av.eotcontrolapp.UI;

public class FileSystemTreeNodeModel {

    public FileSystemTreeNodeModel(boolean isDirectory, String text) {
        this.isDirectory = isDirectory;
        this.text = text;
    }

    public boolean isDirectory;
    public String text;
}
