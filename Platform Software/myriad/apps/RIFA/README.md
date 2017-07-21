This example shows a modified version of the Rotation Invariant Face Detection algorithm from libccv.

This version uses the IMU of the Myriad 2 detecting its rotation angle. Afterwards, the captured picture is rotated using this information.

When a face is found the algorithm applies a local search procedure, searching for the same face in the neighbourhood. If ther is no face found, the algorithm is applied again to the whole picture.

A part of the algorithm has been modified through parallel computing. Each SHAVE will search faces in the same image but with different scale. 
With 480x256 as image size, this application uses 9 SHAVEs. The number of SHAVEs depends of the interval variable and image size.

double scale = pow(2., 1. / (params.interval + 1.));
int scale_upto = (int)(log((double)ccv_min(hr, wr)) / log(scale));

If more SHAVEs are necessary, they must be declared.

It is necessary to copy the detector's files to the SDCard (/mnt/sdcard/). These files can be found in testFiles folder.

#Execution

- Files required:
	- Rotation-invariant_faceDetector/face

Note: Be careful with the order in which WiFi and camera operations are done. A change in the order of this instructions can lead to camera or WiFi deadlocks.
