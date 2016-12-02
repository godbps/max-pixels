/********************************************************************************
* Filename: main.c
* Author	: bps
* Date	: 05/05/2016
*
* Max pixels from Grey scale image
* 
* A grayscale image from an image sensor is in memory with each 16-bit word
* equating to a pixel value. This program written in C finds the 50 pixels
* (including their location) with the highest pixel values. 
*
*
* Assumptions 
* 1. Multi-core processor - concurrent processing (multi-threads). 
* 2. Linux Environment
* 3. Used temporary array method to search the Largest pixels. 
* 4. Change DEBUG macro value to 1, to see the 3 Largest pixels of 6x4 image. (recompile and execute for testing) 
*
* Compile for gcc
* 	make 
* Compile for other core (example for arm with compiler name "arm-linux-gcc")
* 	make CC=arm-linux-gcc
*
* Execute
*      ./maxpixel
*
*********************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "pixel.h"

/*
* Main function
* The code can be modified to collect the command line arguments which can be
* Image frame buffer memory address, resolution (m,n), 'k' Large elements to find
* 
* This function can be modified to use as API in larger project 
* For that add the appropriate function parameters to collect the buffers,resolution,k values
* 
*/

int main(int argc, char *argv[])
{
	struct imageInfo imageData;

	
#if DEBUG	
/*
*	Below was used for testing purpose
*/
	int cores = 1;
	int m = 6, n = 4;
	int i;
	int listSize = m*n;
#else	
	int cores = 1;
	int m = 800, n = 600;
	int i;	
#endif

#if TEST_NOUSER
	/*Create a 2D array - Image is represented by this */
	unsigned short **list;
	unsigned short *oList; 
	int *oRow;
	int *oCol;
#endif

	/*
	* The raw data from Image sensor is stored in RAM.
	* Get the buffer location and the resolution details to set the 2D array parameters
	* For test purpose a 2D array is created and random filled with 16-bit data for a resolution 
	* 800x600 pixels.
	* In practical scenario this buffer is assigned to fill Image sensor data
	*/

#if USER_DATA 
	/*User provided data*/
	imageData.height = m;
	imageData.width = n;	
	imageData.nLargest = MAX_PIXELS;
#endif

#if TEST_NOUSER
	list = (unsigned short **)malloc(m * sizeof(*list));				
	for(i = 0; i < m; i++)
		list[i] = (unsigned short*) malloc (n * sizeof(*(list[i])));
    randFill(list, m, n);
	oList = (unsigned short *) calloc (imageData.nLargest,sizeof(oList));
	oRow = (int *) calloc (imageData.nLargest,sizeof(oRow));
	oCol = (int *) calloc (imageData.nLargest,sizeof(oCol));
#endif

/*Modify and fill with user defined data*/
#if USER_DATA 
	/*User provided data*/
	imageData.imageBuf = list;
	imageData.outListNth = oList;
	imageData.outRowIdx = oRow;
	imageData.outColIdx = oCol;
#endif

	/* Considered a multi-core processor and concurrent processing of image 
	* Number of Threads = Number of CPU cores (nCore)
	* The image is equally divided along the vertical, ie, height/nCore
	* Below API in Linux system reads the cpu cores on the processor
	*/
	cores = sysconf(_SC_NPROCESSORS_ONLN);
	printf("\nCPU cores = %d\n",cores);

	if((cores != 1) && (cores > MAX_CORES))
		cores = MAX_CORES;
	
#if DEBUG
    dumpMainArray(list, m, n, listSize);
#endif

    processImage(&imageData, cores);

	/*Print the resulting array of large pixels modified after sorting*/
	printf("\nTop %d Largest pixels with location from Raw Image (%dx%d) are - \n\n",imageData.nLargest,m,n);
	dumpArray(imageData.outListNth,imageData.outRowIdx,imageData.outColIdx,imageData.nLargest);


#if TEST_NOUSER	
	for (i = 0; i < m; i++) {
		free(list[i]);
	}
	free(list);
#endif

  	return 0;

}



/* ------------ end of file -------------- */