# max-pixels
Introduction:

A grayscale image from an image sensor is in memory with each 16-bit word equating to a pixel value. This program written in C finds the 50 pixels (including their location) with the highest pixel values. 

Assumptions:
1) Linux environment is considered for code compilation and execution.
(Note: I have used cygwin on win7 64-bit for compilation and testing.) 

2) This function is designed to be used as API in firmware with minimum modification.
The modification include change in function name, parameter list that includes details of input buffer (image), resolution, output buffers (Largest pixel values and its corresponding location details in image).

3) Multi-threaded approach is used for concurrent processing of Image parsing to collect ‘k’ largest pixels.
    Multiple threads are created only if the processor has multi-cores. Otherwise, single thread is created.
    Maximum threads are limited to 8.	 

4) 16-bit pixel is defined as ‘unsigned short’ data type.

5) Used temporary array method to search the largest pixels. 

6) Change DEBUG macro value to 1, to see the 3 largest pixels of 6x4 image. (Re-compile and execute for testing) 


Compilation:
1) Compile for gcc
	make clean
	make

2) Compile for other core (example for arm with compiler name "arm-linux-gcc")
	make clean
make CC=arm-linux-gcc

Compilation Notes:
1) To test 6x4 image for 3 largest pixels the DEBUG macro in pixel.h was set to ‘1’ and compiled.

2) To test 800x600 image (or for any user given value) for 50 largest pixels set DEBUG to ‘0’ and generate new executable.

3) If main() function is modified for an API then set TEST_NOUSER macro to ‘0’.


Execute
     ./maxpixel



Files in the package:
Totally 5 files are in the package
1. main.c
2. pixel.c
3. pixel.h
4. Makefile

Code Flow Details and Design Method
1. Given (set the) Image Buffer, Resolution, Output Buffer

2. Check number of cores in processor

3. Call the processImage(&imageData, cores) function 

4. Inside processImage()

	a) Create the threads equal to ‘cores’ and pass the argument with unique thread number.
	    Image gets divided into ‘cores’ number of parts and each thread processes one unique part.

	b) If any error found in creating thread then free the dynamically allocated argument structure

5. Join all the threads so that Main function can wait all threads to complete their execution before proceeding to process the resulting array

6. Thread function 

	a) Divides the image into blocks

b) Temporary array method is used to collect the ‘k’ largest pixels

	
  

7. All threads work on a common array but at different locations. The start address of the array is defined by (thread number * k). 

8 .After completion of execution of all threads the temporary array (and corresponding index arrays) contain partially sorted (threads * k) number of Larger pixels from the image.

9. Using Max-heap sorting method the array is sorted in descending order

10. Top ‘k’ elements (and their location index) are copied to the Output Buffers.

11. All dynamically allocated buffers are freed.

‘k’ Largest Pixels and their location are printed from Output Buffers.

Code Design:

1. The image processing functions are kept in pixel.c file and inaccessible to User by defining ‘static’ definition.

2. The user is hidden from the actual processing method used as it is assumed that User provides input and output buffers location with expected pixel counts. main() function in main.c can be modified to work as API function.

3. Makefile is created to support easy compilation.
