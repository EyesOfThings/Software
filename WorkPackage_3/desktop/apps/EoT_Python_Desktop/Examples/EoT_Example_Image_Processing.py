from eot import Image

lenaColor = Image ("/mnt/sdcard/lena.png", Image.CCV_IO_RGB_COLOR)
lenaColor.show()

lenaColorTransform = lenaColor.color_transform(0)
lenaColorTransform.show()

lenaSaturation = lenaColor.saturation(0, 5)
lenaSaturation.show()

lenaContrast = lenaColor.contrast(0,5)
lenaContrast.show()
