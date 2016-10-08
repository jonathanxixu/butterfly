READ ME BEFORE USING.

1. This project is compiled by CMake, the minimus version is 3.0.0.

2. Tree of project butterfly.
	root
	|----3rdparty----include----easylogging
	|----build
	|----cmake
	|----data
	|----projects----image_filter
	|----test
	
3. Third part dependence
	a) easylogging
		It a standalone head file, stored in root/3rdparty/include/easylogging, you can get more details from offical web http://easylogging.muflihun.com/.
	b) opencv
		It is used as a standard, compared with the mothod I developed, you should download it from offical web and configure the env variable.
		
4. Build method.
	For now, only BuildVS2013x64 is provided, you can double click root/build/BuildVS2013x64.bat to start it.
	Also you can write your bat file to suit your IDE and operate system.
	
5. Test method.
	A b-scan image for clinical trail is offered in root/test.
	You can double click time_performance_test.bat in same folder to test, and the log file will be generated.
	Notice, there are several parameters in the bat file, you can change the filter core size, test image, 
		or determine to generate log or not, for your purpose. You can read Usage in bat file for more details.
	The b-scan image is also saved in root/data for a backup.
	
6. Implementation of Median Filter, the filter window size is N*N, image width is W, height is H.
	Assumption: the filter window size is N*N, image width is W, height is H.
	
	A. Get Median Value by Histogram
	Method 1: an O(1) method, filter for unsigned char
	(1) init a histogram array, its size is w*256, which means each col keeps a histogram
	(2) for each col, put first n pixel into histogram
	(3) sum the first n histogram in hist-array, we can get the result-histogram for the first point, then get the median pixel
	(4) move the filter window toward right, update result-histogram by subtracting the left histogram and adding the right histogram, then get the median pixel
	(5) when the calculation of a row is finished, go to next row
	(6) update the first n histogram in hist-array, by subtracting the upper point and adding the below point for each histogram, sum this first N histogram in hist-array,
		we can get the result-histogram for the first point in this row, then get the median pixel
	(7) move the filter window toward right, update the histogram keeping in next col in hist-array, then update result-histogram by subtracting the left
		histogram and adding the histogram we just updated
	(8) when the calculation of a row is finished, go to next row and repeat (6)(7), until finish each pixel
	Method 2: an O(n) method, filter for unsigned char, float
	if type is unsigned char, start from (4) directly.
	(1) sort all the pixel in input image by quick sort
	(2) get rid of the repeated value, the size of this unique sorted array is M
	(3) map the unique sorted array with input image, we can get a new sequence image
	(4) put the n*n pixel around the first pixel into result-histogram, then get the median pixel
	(5) move the filter window toward right, update result-histogram by subtracting the left col and adding the right col, then get the median pixel
	(6) when the calculation of a row is finished, go to next row and repeat (4)(5), until finish every point
	
	B. Get Median Value by local sorting
	Method 3, filter for unsigned char, float
	(1) allocate a buffer, size is N*N
	(2) put N*N pixels around first pixel into buffer, sorted it, then get median value
	(3) when we tried to get the next median value, we need to move the filter window towards right, so the values in left col will be subtracted from sorted buffer,
		and the values in a new right col will be added to sorted buffer. Which means the value in left col will be replaced by the value in new right col of the same row, then we resorted the buffer
	(4) when the calculation of a row is finished, go to next row and repeat (2)(3), until finish each pixel

7. So, how can we judge the code is CORRECT or WRONG?
	The image filtered by OpenCV will be used as the standard.  The value of each pixel in image filtered by my code will be check.
	If the sum of all pixel¡¯s difference is less than 0.1, then we can tell my code is CORRECT, otherwise is WRONG. 
