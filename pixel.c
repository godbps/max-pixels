/********************************************************************
* File Name: pixel.c 
* Author	: bps
* Date  	: 05/05/2016
*
*
* Consists of functions used for Large pixels extraction and dumping
*
********************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include "pixel.h"


/*Static function Declarations*/
static void max_heapify(struct imgLargePixel *pixel,int i,int size);
static void build_max_heap(struct imgLargePixel *pixel,int size);
static void addLargePixels(struct element *value, struct tImageData *imgData);
static void *findLargestPixels(void *targs);


/*
* For testing purpose - This to generate a random 2D array of 16-bit values for given resolution
* 					It represent the image file
*/
void randFill(unsigned short **list, int r, int c) 
{
	int i,j;
    srand ( time(NULL) );
	for(i=0;i<r;i++) {
		for(j=0;j<c;j++) {
			list[i][j] = (unsigned short)rand();
		}
	}
}

/*
* For testing purpose - To dump the image (2D array)
*/
void dumpMainArray(unsigned short** a, int r, int c, int size) 
{
	int i,j;
	
	for(i=0;i<r;i++) {
		for(j=0;j<c;j++) {
			printf("%d ", a[i][j]);
		}
		printf("\n");
	}	
		
    printf("\n\n");
}

/*
* For testing purpose - To dump the 1D array (result array)
*/
void dumpArray(unsigned short* a, int *row, int *col, int size) 
{
	int i = 1;
    while(size-- > 0) { 
		printf("%d) a[%d][%d] = %d \n",i, *row, *col, *a);
		a++; 
		row++;
		col++;
		i++;
	}
    printf("\n\n");
}

/*
* Max Heap sorting to sort the Largest Pixel array.
*/
/*
* max_heapify
*/
static void max_heapify(struct imgLargePixel *pixel,int i,int size)
{
	int l,r,largest;

	l=left(i);
	r=right(i);

	if((l<=size-1) && (pixel->resListNth[l]<pixel->resListNth[i]))
		largest=l;
	else
		largest=i;

	if(r<=size-1 && (pixel->resListNth[r]<pixel->resListNth[largest]))
		largest = r;

	if(largest != i)
	{
		SWAP(pixel->resListNth[i],pixel->resListNth[largest]);
		SWAP(pixel->resRowIdx[i],pixel->resRowIdx[largest]);
		SWAP(pixel->resColIdx[i],pixel->resColIdx[largest]);		
		
		max_heapify(pixel,largest,size);
	}
}

/*
* build_max_heap
*/
static void build_max_heap(struct imgLargePixel *pixel,int size)
{
	int i;
	for(i=(size/2)-1;i>=0;i--)
		max_heapify(pixel,i,size);
}

/*
* Heap sort
*/
void heap_sort(struct imgLargePixel *pixel,int size)
{
	int i;

	build_max_heap(pixel,size);
	for(i=size-1;i>0;i--)
	{
		SWAP(pixel->resListNth[i],pixel->resListNth[0]);
		SWAP(pixel->resColIdx[i],pixel->resColIdx[0]);
		SWAP(pixel->resRowIdx[i],pixel->resRowIdx[0]);

		size=size-1;
		max_heapify(pixel,0,size);
	}
}


/*Image Parser and Selector code*/

/*
* Function to arrange the array containing Largest values (k elements) in ascending order.
* The first element is always the minimum and is compared with values from a[k] to a[(m*n)] 
*/
static void addLargePixels(struct element *value, struct tImageData *imgData) 
{

	int i = 0;
	int size = imgData->iData->nLargest;
	int j = (imgData->tid * size);	

    while((i<size-1) && (value->val > imgData->resListNth[j+1])) {
        imgData->resListNth[j] = imgData->resListNth[j+1];
		imgData->resRowIdx[j] = imgData->resRowIdx[j+1];
		imgData->resColIdx[j] = imgData->resColIdx[j+1];
		
		//printf("newlist[%d]=%d\n",j,imgData->resListNth[j]);
		//printf("row[%d]=%d\n",j,imgData->resRowIdx[j]);
		//printf("col[%d]=%d\n",j,imgData->resColIdx[j]);
		i++;
		j++;
    }

	imgData->resListNth[j] = value->val;
	imgData->resRowIdx[j] = value->row;
	imgData->resColIdx[j] = value->col;
	
	//printf("newlist[%d]=%d\n",j,imgData->resListNth[j]);
}

/*
* Thread function - fills the result array to k elements from its start location (tid*k)
*/
static void *findLargestPixels(void *targs)
{	  	
	struct tImageData *imgData = (struct tImageData *)targs;

  	int tid = imgData->tid;
	unsigned int k = imgData->iData->nLargest; 	
	unsigned int start;
	unsigned short **list = imgData->iData->imageBuf;

	int i,j;
	int r = ((imgData->iData->height/imgData->nCore)*(tid+1));
	int c = imgData->iData->width;
    unsigned short *listNth = imgData->resListNth;

	struct element value;

	start = (tid * k); //Result Array location pointed 

	i = ((imgData->iData->height/imgData->nCore)*(tid));
	j = 0;
		
	while(k) {		
		value.val = list[i][j];
		value.row = i;
		value.col = j;
		addLargePixels(&value, imgData);
		j++;
		if(j == c) {
			j=0;
			i++;
		} else if(i==r) {
			break;
		}
		k--;		
    }	
	
	for(; i<r; i++) {
		for(;j<c; j++) {
        	if(list[i][j] > *(listNth+start)) {
				value.val = list[i][j];
				value.row = i;
				value.col = j;
				addLargePixels(&value, imgData);
        	}
		}
		j=0;
    }
	
#if DEBUG
	printf("Thread - %d generated Larger values: \n",tid);
    dumpArray((imgData->resListNth+start), (imgData->resRowIdx+start), (imgData->resColIdx+start), imgData->iData->nLargest);   
#endif

	return NULL; //To clear compiler warning: control reaches end of non-void function [-Wreturn-type]
}

/*
* Function to create threads
*/
int processImage(struct imageInfo *imageData, int cores)
{
	int rc, i;
	struct imgLargePixel lPixel;
	pthread_t threads[MAX_CORES];	

	/*
	* The resultant array size is = Num of threads * k
	* where k = Largest pixels to be found
	* Each thread fills this array with Largest pixels from the image it processes
	* The start location array for each thread = (tid * k) where tid = thread number (range is 0 to nCore)
	*/
	unsigned short *rListNth = (unsigned short*) calloc (cores*imageData->nLargest,sizeof(rListNth));
	int *rRowIdx = (int*) calloc (cores*imageData->nLargest,sizeof(rRowIdx));
	int *rColIdx = (int*) calloc (cores*imageData->nLargest,sizeof(rColIdx));

	/* spawn the threads */
	for (i=0; i<cores; i++) {
		/*Important to create the datastructure on heap as 'tid' passed should be unique for each thread
		*Otherwise, sometimes same tid is used by multiple threads thus corrupting the result array data
		*/
		struct tImageData *tImgData = (struct tImageData *)malloc(sizeof(*tImgData));
		tImgData->tid = i;
		tImgData->iData = imageData; 	
		tImgData->resListNth = rListNth;
		tImgData->resRowIdx = rRowIdx;
		tImgData->resColIdx = rColIdx;
		tImgData->nCore = cores;

		rc = pthread_create(&threads[i], NULL, findLargestPixels, (void *) tImgData);      
		if(rc) { 
			free(tImgData);
			printf("Error -%d in creating thread %d\n",rc,i);
		}		
	}

	/* Main function execution waits until all threads complete their execution. 
	* When they finish the result array will have valid data for further processing
	*/
	for (i=0; i<cores; i++) {
		rc = pthread_join(threads[i], NULL);
	}


	/*Print the resulting array of large pixels modified by all threads*/
	//printf("Output of threads (%d unsorted largest pixels) -\n",(cores*imageData->nLargest));
	//dumpArray(rListNth,rRowIdx,rColIdx,(cores*imageData->nLargest));

	/*Sort the result array in descending order and Collect the top 'k' elements representing Largest Pixel values with location */
	/* Heap sort to get 'k' largest pixels */
	lPixel.resListNth = rListNth;
	lPixel.resRowIdx = rRowIdx;
	lPixel.resColIdx = rColIdx;

	heap_sort(&lPixel,(cores * imageData->nLargest));


	/*Copy the 'k' elements to output array*/
	for(i=0; i<imageData->nLargest; i++) {
		imageData->outListNth[i] = rListNth[i];
		imageData->outRowIdx[i] = rRowIdx[i];
		imageData->outColIdx[i] = rColIdx[i];
	}

	/*Free all the dynamically allocated buffers */
	free(rListNth);
	free(rRowIdx);
	free(rColIdx);

	return 0;

}

/* ------------ end of file -------------- */