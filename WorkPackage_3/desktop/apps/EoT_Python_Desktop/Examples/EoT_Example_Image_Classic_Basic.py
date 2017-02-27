from eot import Camera
from eot import Image
cam = Camera()
im1 = cam.snapshot()
im1.show()
img2 = cam.snapshot()
img2.show()
print(img2)
im3 = img2.sobel(0,3,3)
im3.show()
print(im3)
theta, gradient = img2.gradient(0, 0, 1, 3)
gradient.show()
img5 = img2.flip(0 , Image.CCV_FLIP_X)
img5.show()
imgblur = img2.blur(0, 3)
imgblur.show()
imhog = img2.hog(0, 9, 8)
print (imhog)
canny = img2.canny(0, 3, 3, 3*36)
canny.show()
img2.otsu(20,150)
imsample_down = img2.sample_down(0, 2, 2)
imsample_down.show()
imsample_up = img2.sample_up(0,2,2)
imsample_up.show()



