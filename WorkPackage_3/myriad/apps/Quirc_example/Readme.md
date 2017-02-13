#Quirc_examples

This application runs several examples using the Quirc library. For each test, the output will be the number of QR codes detected and its content. If an error occurs , the output will display its type.

The content of the testFiles folder must be copied to the SD card (/mnt/sdcard/) resulting in ==/mnt/sdcard/QuircTest/==

##List of examples

1. TestQR.
	- Files required:
		- File: /mnt/sdcard/QuircTest/TestQR.png
	- Output:
		- Text: num_codes: 1 Data: EoT

2. TestText.
	- Files required:
		- File: /mnt/sdcard/QuircTest/TestTex.png
	- Output:
		- Text: num_codes: 1 Data: http://www.qrdroid.com

3. Test500x500.
	- Files required:
		- File: /mnt/sdcard/QuircTest/Test500x500.png
	- Output:
		- Text: num_codes: 1 Data: EoT 500 x 500 px

4. TestRotated.
	- Files required:
		- File: /mnt/sdcard/QuircTest/TestRotated.png
	- Output:
		- Text: num_codes: 1 Data: Rotated Code EoT

5. TestRotatedError.
	- Files required:
		- File: /mnt/sdcard/QuircTest/TestRotatedError.png
	- Output:
		- Error: num_codes: 1 Data: ECC failure

6. TestRedCode.
	- Files required:
		- File: /mnt/sdcard/QuircTest/TestRedCode.png
	- Output:
		- Text: num_code: 1 Data: Red Code EoT

7. TestTwoQR.
	- Files required:
		- File: /mnt/sdcard/QuircTest/TestTwoQR.png
	- Output:
		- Text: num_codes: 2 Data: EoT Data: http://eyesofthings.eu/

8. TestTooSmall.
	- Files required:
		- File: /mnt/sdcard/QuircTest/TestTooSmall.png
	- Output:
		- Error: num_codes: 1 Data: Invalid version
    