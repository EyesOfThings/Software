from eot import Image
from eot import Camera
mCamera = Camera()
lenaColor = Image ("/mnt/sdcard/lena.png", Image.CCV_IO_RGB_COLOR)
lenaColor.show()
mSnapshot = mCamera.snapshot()
mSnapshot.show()
lenaGray = Image ("/mnt/sdcard/lena.png", Image.CCV_IO_GRAY)
lenaGray.show()
