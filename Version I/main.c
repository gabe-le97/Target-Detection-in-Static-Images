/*
 **********************************************************************************
 * File: main.c
 * Authors: Created by Jean-Yves Hervé on 2018-05-01.
 *          Modified by Gabe Le & Nathan Larson on 2018-05-9.
 * Due Date: 5/13/18
 *..................................................................................
 * Blob Detection in static images ->
 *
 * Reads two images: a background image & another that has other objects on top of it.
 *  We detect objects by using background subtraction - computing the difference between
 *  the background image and the frame image. This is done by converting each rgb pixel
 *  to its respective grey-scale values and finding the absolute value of the difference.
 *  The difference in the two images is saved to an output tga file. Then we search for
 *  blobs within the image which are connected pixels in an image and show them on the
 *  image. 
 *=====================================================================================
 * This is how I compiled my program on Mac ->
 *  gcc -Wall main.c gl_frontEnd.c fileIO_TGA.c Blob.c -lm -framework OpenGL -framework GLUT -w -o blob
 *
 **********************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
//-----------------------
#include "gl_frontEnd.h"
#include "fileIO_TGA.h"
#include "Blob.h"

//==================================================================================
// Thread data type
//==================================================================================

typedef struct ThreadInfo {
    pthread_t threadID;
    int index;
    int row;
} ThreadInfo;

//==================================================================================
// Function prototypes
//==================================================================================

void initializeApplication(void);
void* threadFunction(void* arg);

void convertRGBToGreyscale(ImageStruct oldImage, int** pixel2D, int row);
void overrideImage(ImageStruct imageIn, int** pixel2D, int row);
void computeAbsoluteValue(int** pixel2DOld, int** pixel2DNew, int** pixel2DDifference, int row);
void detectBlobs(ImageStruct blobImage, int** imagePixels);
int findLeft(int x, int y, int** imagePixels, int** isChecked);
int findRight(int x, int y, int maxCol, int** imagePixels, int** isChecked);
void* threadFunc(void* arg);

//==================================================================================
// Application-level global variables
//==================================================================================

// Don't touch. These are defined in the front end source code
extern int gMainWindow;
extern const int WINDOW_WIDTH, WINDOW_HEIGHT;

#if FOUR_QUADRANT_VERSION
    extern const int QUADRANT_WIDTH;
    extern const int QUADRANT_HEIGHT;

    extern int gSubwindow[4];
#endif

// read a single image file and use it as output.
// The scale factors are computed so that the entore image is displayed
//  fit to the window's dimensions.
ImageStruct oldImage, newImage, differenceImage;
float scaleX, scaleY;
int initDone = 0;
int** pixel2DOld;
int** pixel2DNew;
int** pixel2DDifference;
int** differencePixel;

// A hand-initialized list of blobs, for rendering testing
Blob* blobList;
unsigned int nbBlobs = 0;

//------------------------------------------------------------------
// The constants defined here are for you to modify and add to
//------------------------------------------------------------------
#define IN_PATH     "./DataSets/Series02/"
#define OUT_PATH    "./Output/"

// You can change newImagePath to frame01 or frame02 
#define oldImagePath "../Data/Part I/background.tga"
#define newImagePath "../Data/Part I/frame02.tga"

//==================================================================================
// These are the functions that tie the computation with the rendering.
// Some parts are "don't touch."  Other parts need your intervention
//  to make sure that access to critical section is properly synchronized
//==================================================================================

#if FOUR_QUADRANT_VERSION
    // In four-quadrant mode, we need a different rendering function for each
    //  quadrant
    void displayUpperLeft(void) {
	printf("\t displayUpperLeft \n");

        glutSetWindow(gSubwindow[UPPER_LEFT]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (initDone) {
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glPixelZoom(scaleX, scaleY);
            //==============================================
            // This is OpenGL/glut magic. Replace, if you
            //  want, but don't modify
            //==============================================
            if (oldImage.type == RGBA32_RASTER) {
                glDrawPixels(oldImage.nbCols, oldImage.nbRows,
                              GL_RGBA,
                              GL_UNSIGNED_BYTE,
                              oldImage.raster);
            }
            else if (newImage.type == GRAY_RASTER) {
                glDrawPixels(oldImage.nbCols, oldImage.nbRows,
                              GL_LUMINANCE,
                              GL_UNSIGNED_BYTE,
                              oldImage.raster);
            }
        }
        glutSetWindow(gMainWindow);
    }

    void displayUpperRight(void) {
        glutSetWindow(gSubwindow[UPPER_RIGHT]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (initDone) {
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glPixelZoom(scaleX, scaleY);

            //==============================================
            // This is OpenGL/glut magic. Replace, if you
            //  want, but don't modify
            //==============================================
            if (newImage.type == RGBA32_RASTER) {
                glDrawPixels(newImage.nbCols, newImage.nbRows,
                              GL_RGBA,
                              GL_UNSIGNED_BYTE,
                              newImage.raster);
            }
            else if (newImage.type == GRAY_RASTER) {
                glDrawPixels(newImage.nbCols, newImage.nbRows,
                              GL_LUMINANCE,
                              GL_UNSIGNED_BYTE,
                              newImage.raster);
            }
        }
        glutSetWindow(gMainWindow);
    }

    void displayLowerLeft(void) {
        glutSetWindow(gSubwindow[LOWER_LEFT]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (initDone) {
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glPixelZoom(scaleX, scaleY);

           glDrawPixels(oldImage.nbCols, newImage.nbRows,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         newImage.raster);
        }
        glutSetWindow(gMainWindow);
    }

    void displayLowerRight(void) {
        glutSetWindow(gSubwindow[LOWER_RIGHT]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (initDone) {
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glPixelZoom(scaleX, scaleY);

           glDrawPixels(oldImage.nbCols, newImage.nbRows,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         newImage.raster);
        }

        glutSetWindow(gMainWindow);

    }

#else
    // This is the rendering function in single-pane mode
    void displayImagePane(void) {
        //==============================================
        // This is OpenGL/glut magic. Don't touch
        //==============================================
        glutSetWindow(gMainWindow);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (initDone) {
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glPixelZoom(scaleX, scaleY);

            //==============================================
            //    This is OpenGL/glut magic.  Replace, if you
            //    want, but don't modify
            //==============================================

            if (newImage.type == RGBA32_RASTER) {
            	// going to draw new photo with blobs on top
            	newImage = readTGA(newImagePath);
                glDrawPixels(newImage.nbCols, newImage.nbRows,
                              GL_RGBA,
                              GL_UNSIGNED_BYTE,
                              newImage.raster);
                
            }
            else if (newImage.type == GRAY_RASTER) {
                glDrawPixels(newImage.nbCols, newImage.nbRows,
                              GL_LUMINANCE,
                              GL_UNSIGNED_BYTE,
                              newImage.raster);
            }

            //--------------------------------------------------------
            // If there are blobs, render them as well

            glPushMatrix();
            glScalef(scaleX, scaleY, 1.f);

            for (unsigned int k=0; k<nbBlobs; k++) {
                renderBlob(blobList + k);
            }
            glPopMatrix();
        }

        glutSetWindow(gMainWindow);
    }
#endif

/*
 *------------------------------------------------------------------------
 * This callback function is called when a keyboard event occurs
 * You can change things here if you want to have keyboard input
 *------------------------------------------------------------------------
 */
void myKeyboard(unsigned char c, int x, int y) {
    int ok = 0;

    switch (c) {
        // 'esc' to quit
        case 27:
        	writeTGA("../Data/Part I/outImg.tga", &differenceImage);
            exit(0);
            break;

        default:
            ok = 1;
            break;
    }
    if (!ok) {
        // do something?
    }

    //==============================================
    // This is OpenGL/glut magic.  Don't touch
    //==============================================
    glutSetWindow(gMainWindow);
    glutPostRedisplay();
}


/*
 *------------------------------------------------------------------------
 * Convert each image pixel to its respective grey-level value
 *------------------------------------------------------------------------
 */
void convertRGBToGreyscale(ImageStruct oldImage, int** pixel2D, int row) {
    int color = 0;
    for(int j = 0; j < oldImage.nbCols; j++) {
        color = pixel2D[row][j];
        unsigned char red = color & 0x000000FF;
        unsigned char green = (color & 0x0000FF00) >> 8;
        unsigned char blue = (color & 0x00FF0000) >> 16;

        int greyScaleValue = (red + green + blue) / 3;
        red = green = blue = greyScaleValue;

        pixel2D[row][j] = red | (green <<8) | (blue<<16) | 0xFF000000;
	}
}


/*
 *------------------------------------------------------------------------
 * Converts a copy of a TGA and overwrite it with the array of difference
 *  pixels computed by comparing the grey-level image absolute values 
 *------------------------------------------------------------------------
 */
void overrideImage(ImageStruct imageIn, int** pixel2D, int row) {
	for(int j = 0; j < imageIn.nbCols; j++) {
		differencePixel = (int**) imageIn.raster2D;
		differencePixel[row][j] = pixel2D[row][j];
	}
}


/*
 *------------------------------------------------------------------------
 * compute the absolute value of the difference between these two 
 *  gray-level images
 *------------------------------------------------------------------------
 */
void computeAbsoluteValue(int** pixel2DOld, int** pixel2DNew, int** pixel2DDifference, int row) {
	int pixelOld;
	int pixelNew;
	int difference;
	int threshhold = 70;

	// images are both the same size so we can use either one
	for(int j = 0; j < oldImage.nbCols; j++) {
		pixelOld = pixel2DOld[row][j];
		pixelNew = pixel2DNew[row][j];

        // doing the actual comparison 
		unsigned char oldImagePixel = pixelOld & 0x000000FF;
		unsigned char newImagePixel = pixelNew & 0x000000FF;
		difference = abs(oldImagePixel - newImagePixel);

		if(difference < threshhold)
			difference = 0;
		else 
			difference = 255;
		
		pixel2DDifference[row][j] = difference;
	}

}


/*
 *------------------------------------------------------------------------
 * Function to scan the image and detect blobs based off the blob
 *  detection algorithm
 *------------------------------------------------------------------------
 */
void detectBlobs(ImageStruct blobImage, int** imagePixels) {
    // 2D array to track whether or not each pixel has been checked, and given their respective blob label
    int** isChecked = (int**) malloc(blobImage.nbRows * sizeof(int*));

    for(int i=0; i < blobImage.nbRows; i++) {
        isChecked[i] = (int*) malloc(blobImage.nbCols * sizeof(int));
    }

    int prevRow = 0;
    for(int i=0; i < blobImage.nbRows; i++) {
        for(int j=0; j < blobImage.nbCols; j++) {
            // if the current pixel has a valid difference
            if((imagePixels[i][j] & 0xFF) != 0) {
                isChecked[i][j] = 1;                                                        
                int left = findLeft(j, i, imagePixels, isChecked);                          
                int right = findRight(j, i, blobImage.nbCols-1, imagePixels, isChecked);
                // if current row/blob is not connected to previous (or first)
                if(abs(i - prevRow) > 1 || (nbBlobs) == 0) {
                    nbBlobs += 1;                                               
                    // reallocate memory for longer list
                    blobList = (Blob*) realloc(blobList, nbBlobs*sizeof(Blob));
                    // create a new blob
                    blobList[nbBlobs-1] = newBlob();
                    // set color of new blob (red)
                    blobList[nbBlobs-1].red = 0xFF;
                }
                // add the new segment to blob
                addSegmentToBlob(blobList + (nbBlobs-1), left, right, i);
                // set "previous" row to current row
                prevRow = i; 
            }
        }
    }

    // free memory from isChecked array
    for (unsigned int i=0; i< blobImage.nbRows; i++)
        free(isChecked[i]);

    free(isChecked);
}


/*
 *------------------------------------------------------------------------
 * Function to find the farthest left (first) valid difference pixel in blobs current row
 *------------------------------------------------------------------------
 */
int findLeft(int x, int y, int** imagePixels, int** isChecked) {
    int counter = x - 1;
    if(counter < 0)
        return x;
    while(counter > 0) {
        if((imagePixels[y][counter] & 0xFF) != 0) {
            isChecked[y][counter] = 1;
            counter -= 1;
        }
        else
            break;
    }
    return counter;
}


/*
 *------------------------------------------------------------------------
 * Function to find the farthest right (last) valid difference pixel in blobs current row
 *------------------------------------------------------------------------
 */
int findRight(int x, int y, int maxCol, int** imagePixels, int** isChecked) {
    int counter = x + 1;
    if(counter > maxCol)
        return maxCol;
    while(counter < maxCol) {
        if((imagePixels[y][counter] & 0xFF) != 0) {
            isChecked[y][counter] = 1;
            counter += 1;
        }
        else
            break;
    }
    return counter;
}

/*
 *------------------------------------------------------------------------
 * Each thread will run background subtraction on their respective rows
 *------------------------------------------------------------------------
 */
void* threadFunc(void* arg) {
	ThreadInfo* info = (ThreadInfo *) arg;
	int row = info->row;
	convertRGBToGreyscale(oldImage, pixel2DOld, row);
	convertRGBToGreyscale(newImage, pixel2DNew, row);
    computeAbsoluteValue(pixel2DOld, pixel2DNew, pixel2DDifference, row);
    overrideImage(differenceImage, pixel2DDifference, row);
    convertRGBToGreyscale(differenceImage, differencePixel, row);
    return NULL;
}


/*
 *------------------------------------------------------------------------
 *   Main function where the threads are created
 *------------------------------------------------------------------------
 */
int main(int argc, char** argv) {
    srand((unsigned int) time(NULL));

    initializeFrontEnd(argc, argv);

    // Now we can do application-level initialization
    initializeApplication();

    int numThreads = oldImage.nbRows;
    pixel2DOld = (int**) oldImage.raster2D;
    pixel2DNew = (int**) newImage.raster2D;
    // allocating output file to the same size as the two images compared
    pixel2DDifference = (int**) oldImage.raster2D;
    // array for all the threads for easy access
    ThreadInfo threads[numThreads];
    int errCode;

    // create a thread to background subtract the pixels - one for each row
    for(int i = 0; i < numThreads; i++) {
    	threads[i].index = i+1;
    	threads[i].row = i;
	    errCode = pthread_create(&threads[i].threadID, NULL, threadFunc, threads + i);
	      // stop if we could not create a thread
        if (errCode != 0) {
            exit (EXIT_FAILURE);
        }
    }
    // threads have finished background subtracting their respective rows
    for(int i=0; i < numThreads; i++) {
        pthread_join(threads[i].threadID, NULL);
    }

    detectBlobs(newImage, pixel2DDifference);

    //==============================================
    //    This is OpenGL/glut magic.  Don't touch
    //==============================================
    glutMainLoop();

    // This will probably never be executed (the exit point will be in one of the
    //  call back functions).
    return 0;
}


/*
 *------------------------------------------------------------------------
 * Read the TGA files and build the visual application
 *------------------------------------------------------------------------
 */
void initializeApplication(void) {
    oldImage = readTGA(oldImagePath);
    newImage = readTGA(newImagePath);
    differenceImage = readTGA(oldImagePath);

    blobList = (Blob*) malloc(nbBlobs*sizeof(Blob));

    #if FOUR_QUADRANT_VERSION
        scaleX = (1.f*QUADRANT_WIDTH)/newImage.nbCols;
        scaleY = (1.f*QUADRANT_HEIGHT)/newImage.nbRows;
    #else
        scaleX = (1.f*WINDOW_WIDTH)/newImage.nbCols;
        scaleY = (1.f*WINDOW_HEIGHT)/newImage.nbRows;
    #endif

    initDone = 1;

}