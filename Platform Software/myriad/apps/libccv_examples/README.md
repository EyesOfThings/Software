# libccv_examples

This application runs several examples using the libccv library. Some of them compare the obtained result vs expected result, giving an assertion if the values are different.

The contents of the testFiles folder (data and samples) must be copied to the SD card (/mnt/sdcard/).

## List of examples

1. Canny edge detector.
	- Files required: 
		- samples/blackbox.png 
		- data/blackbox.canny.bin
	- Output: 
		- File: samples/test1.png (1 if border has been detected, 0 if not)

2. Pedestrian cascade detector using Integral Channel Features.
	- Files required: 
		- samples/pedestrian.icf 
		- samples/pedestrian2.png
	- Output:
		- Text: Number of pedestrians detected. For each detection, classification confidence and x, y, width and height of the rectangle.

3. Matrix addition operation.

4. Sobel operations.
	- Files required: 
		- samples/chessbox.bmp
		- data/chessbox.sobel.x.bin
		- data/chessbox.sobel.y.bin
		- data/chessbox.sobel.u.bin
		- data/chessbox.sobel.v.bin
		- data/chessbox.sobel.x.3.bin
		- data/chessbox.sobel.y.3.bin
		- data/chessbox.sobel.x.5.bin
	- Output: 
		- samples/test4.png (partial derivative on y within 5x5 window)

5. Otsu threshold calculation.

6. Image contrast modification.
	- Files required: 
		- samples/nature.bmp
		- data/nature.contrast.0.5.bin
		- data/nature.contrast.1.5.bin
	- Output: 
		- samples/test6_1.png (reducing contrast to 0.5)
		- samples/test6_2.png (increasing contrast 1.5 times)

7. Perspective transform on a picture.
	- Files required: 
		- samples/chessbox.bmp
		- data/chessbox.perspective.transform.bin
	- Output: 
		- samples/test7.png (picture rotated along y-axis for 30º)

8. Histogram of gradients calculation.
	- Files required: 
		- samples/nature.bmp
	- Output: 
		- samples/test8.png (values obtained)

9. Scale Invariant Feature Transform.
	- Files required: 
		- samples/book.png
	- Output: 
		- Text: List of keypoints matching.

10. CBLAS matrix multiplication.

11. Convolutional network of 11x11 on 225x225 with uniform weights.

12. Convolutional network of 5x5 on 27x27 with non-uniform weights.

13. Convolutional network of 5x5x4 on 27x27x8 partitioned by 2.

14. Stroke Width Transform (text detection).
	- Files required: 
		- samples/text-detect.png
	- Output: 
		- Text: Number of texts detected. For each detection, x, y, width and height of the rectangle.

15. Brightness Binary Feature (face detection).
	- Files required: 
		- samples/face (folder with cascade.txt and stage-0.txt to stage-15.txt files)
		- samples/suit.png
	- Output: 
		- Text: Number of faces detected. For each detection, classification confidence and x, y, width and height of the rectangle.