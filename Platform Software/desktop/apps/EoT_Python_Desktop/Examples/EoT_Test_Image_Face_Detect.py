from eot import Image
from eot import Camera
mCamera = Camera()
mSnapshot = mCamera.snapshot()
mSnapshot.show()

#Detect faces from snapshot
list_faces = mSnapshot.detect_face_bbf ("/mnt/sdcard/Rotation-invariant_faceDetector/face")
color = 0
for face in list_faces:
    mSnapshot.draw_rectangle(face[0], face[1], face[2], face[3], color)
mSnapshot.show()

#Detect faces from SDCard Image
lena = Image ("/mnt/sdcard/lena.png", Image.CCV_IO_GRAY)
lena.show()
list_faces = lena.detect_face_bbf ("/mnt/sdcard/Rotation-invariant_faceDetector/face")
for face in list_faces:
    lena.draw_rectangle(face[0], face[1], face[2], face[3], color)
lena.show()

