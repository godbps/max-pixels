/********************************************************************
* File Name: pixel.h
* Author	: bps
* Date  	: 05/05/2016
*
*
* Header file for Macros, datatypes and function declarations
*
********************************************************************/
#ifndef __PIXEL_H__
#define __PIXEL_H__

#define DEBUG	1
#define TEST_NOUSER 1
#define USER_DATA 1

#if DEBUG
#define MAX_PIXELS	3	//Number of Pixels to find
#else
#define MAX_PIXELS	50	//Number of Pixels to find
#endif

#define MAX_CORES	8 	//Place a limit to number of threads

/*Swap the elements*/
#define SWAP(a,b) do{int t=a; \
					a=b; \
					b=t; \
				}while(0)

/*Address of left and right nodes in heap*/
#define left(i) (2*i)+1
#define right(i) (2*i)+2

/*
* Datastructures
*/
struct imgLargePixel {
	unsigned short *resListNth;
	int *resRowIdx;
	int *resColIdx;
};

struct imageInfo {
	unsigned int width;	
	unsigned int height;
	int nLargest;
	unsigned short **imageBuf; //Raw Image Buffer
	int *outRowIdx;
	int *outColIdx;
	unsigned short *outListNth;
};

struct element {
	int val;
	int row;
	int col;
};

struct tImageData {
	struct imageInfo *iData;
	int tid;	
	int nCore;
	int *resRowIdx;
	int *resColIdx;
	unsigned short *resListNth;
};

/*Function declarations*/
void randFill(unsigned short **list, int r, int c);
void dumpMainArray(unsigned short** a, int r, int c, int size);
void dumpArray(unsigned short* a, int *row, int *col, int size);
void heap_sort(struct imgLargePixel *pixel,int size);
int processImage(struct imageInfo *imageData, int cores);


#endif
/* ------------ end of file -------------- */
