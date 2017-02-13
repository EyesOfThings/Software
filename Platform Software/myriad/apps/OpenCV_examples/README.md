# libccv_examples

This application runs several examples using the OpenCV library. In most cases the user needs to check the results saved in the SD card. Assertions check some common errors.

The contents of the testFiles folder (OpenCVTests) must be copied to the SD card (/mnt/sdcard/) resulting in ==/mnt/sdcard/OpenCVTests==.

## List of examples

1. Canny edge detector.
	- Files required:
		- File: /mnt/sdcard/OpenCVTests/data/nature.png
	- Output:
		- File: /mnt/sdcard/OpenCVTests/results/resultEdge.png
2. Haar-based Cascade Face Detector
	- Files required:
		- /mnt/sdcard/OpenCVTests/data/lena.png
		- /mnt/sdcard/OpenCVTests/data/haarface.xml
	- Output:
		- /mnt/sdcard/OpenCVTests/results/resultFaceDetection.png
3. K-means algorithm
	- Output:
		- /mnt/sdcard/OpenCVTests/results/resultTestKmeans.png
4. Contours
	- Files required:
		- /mnt/sdcard/OpenCVTests/results/resultTestContours_1.png
	- Output:
		- /mnt/sdcard/OpenCVTests/results/resultTestContours_1.png
		- /mnt/sdcard/OpenCVTests/results/resultTestContours_2.png
5. Histogram/LUT
	- Files required:
		- /mnt/sdcard/OpenCVTests/data/baboon.png
	- Output:
		- /mnt/sdcard/OpenCVTests/results/resultDemHist.png
6. Delaunay Triangulation
	- Output:
		- /mnt/sdcard/OpenCVTests/results/delaunay/delaunay[n].png ([n]={0,1,2})
		- /mnt/sdcard/OpenCVTests/results/delaunay/delaunay[n]_s.png ([n]={0,1,2})
		- /mnt/sdcard/OpenCVTests/results/delaunay/voronoi.png
7. testPyramidSegmentation
	- Files required:
		- /mnt/sdcard/OpenCVTests/data/fruits.png
	- Output:
		- /mnt/sdcard/OpenCVTests/results/resultPyramidSegmentation.png
8. testSquareDetector
	- Files required:
		- /mnt/sdcard/OpenCVTests/data/pic[n].png ([n]={1,2,3,4,5,6})
	- Output:
		- /mnt/sdcard/OpenCVTests/results/squares/ResultSquarespic[n].png ([n]={1,2,3,4,5,6})
9. Watershed
	- Files required:
		- /mnt/sdcard/OpenCVTests/data/fruits.png
	- Output:
		- /mnt/sdcard/OpenCVTests/results/resultWatershed.png
10. Morphological Operations
	- Files required:
		- /mnt/sdcard/OpenCVTests/data/baboon.png
	- Output:
		- /mnt/sdcard/OpenCVTests/results/morphology/baboon_E_OC.png
		- /mnt/sdcard/OpenCVTests/results/morphology/baboon_E_ED.png
		- /mnt/sdcard/OpenCVTests/results/morphology/baboon_R_OC.png
		- /mnt/sdcard/OpenCVTests/results/morphology/baboon_R_ED.png
		- /mnt/sdcard/OpenCVTests/results/morphology/baboon_C_OC.png
		- /mnt/sdcard/OpenCVTests/results/morphology/baboon_C_ED.png
11. Fourier
	- Files required:
		- /mnt/sdcard/OpenCVTests/data/suit.png
	- Output:
		- /mnt/sdcard/OpenCVTests/results/dftres2.png
12. Inpainting
	- Files required:
		- /mnt/sdcard/OpenCVTests/data/lena.png
	- Output:
		- /mnt/sdcard/OpenCVTests/results/inpainted.png
13. Min Area Rectangle
	- Output:
		- /mnt/sdcard/OpenCVTests/results/rectCircle.png
14. Lucas Kanade
	- Files required:
		- /mnt/sdcard/OpenCVTests/data/suit.png
	- Output:
		- /mnt/sdcard/OpenCVTests/results/LKDemo.png
15. DOG
	- Files required:
		- /mnt/sdcard/OpenCVTests/data/DunLoghaire_320x240.png
	- Output:
		- /mnt/sdcard/OpenCVTests/results/DOG.png
16. Dilation
	- Files required:
		- /mnt/sdcard/OpenCVTests/data/lena_512x512_luma.png
	- Output:
		- /mnt/sdcard/OpenCVTests/results/dilate2.png
17. Harris Corners
	- Files required:
		- /mnt/sdcard/OpenCVTests/data/lena_512x512_luma.png
	- Output:
		- /mnt/sdcard/OpenCVTests/results/cornerharris.png
18. Median Filter
	- Files required:
		- /mnt/sdcard/OpenCVTests/data/ref_chroma_median_out_512x512_P444_8bpp.png
	- Output:
		- /mnt/sdcard/OpenCVTests/results/medianfilter.png