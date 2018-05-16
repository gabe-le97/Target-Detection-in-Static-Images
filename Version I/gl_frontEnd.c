//
//  gl_frontEnd.c
//  GL threads
//
//  Created by Jean-Yves Hervé on 2017-12-04.
//	Modified 2018-05-01

 /*-------------------------------------------------------------------------+
 |	A graphic front end for displaying the output image.					|
 |																			|
 +-------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
//
#include "fileIO_TGA.h"
#include "gl_frontEnd.h"

extern int numLiveThreads;
extern ImageStruct imageOut;

//---------------------------------------------------------------------------
//  Private functions' prototypes
//---------------------------------------------------------------------------

void myResize(int w, int h);
void myMouse(int b, int s, int x, int y);
void myTimer(int d);
void setupCamera(int w, int h);

//---------------------------------------------------------------------------
//  Interface constants
//---------------------------------------------------------------------------

const int   INIT_WIN_X = 10,
            INIT_WIN_Y = 10;
const int   msecs = 20;

#if FOUR_QUADRANT_VERSION
	const int PADDING = 10;
	const int QUADRANT_WIDTH = 450;
	const int QUADRANT_HEIGHT = 300;
	const int WINDOW_WIDTH = 930;
	const int WINDOW_HEIGHT = 630;
#else
	const int WINDOW_WIDTH = 900;
	const int WINDOW_HEIGHT = 600;
#endif

//---------------------------------------------------------------------------
//  File-level global variables
//---------------------------------------------------------------------------
int	gMainWindow;

//	If we are in four-quadrant version, we need to define subwindows
//	and display functions for them
#if FOUR_QUADRANT_VERSION
	int gSubwindow[4];
#endif


//---------------------------------------------------------------------------
//	Drawing functions
//---------------------------------------------------------------------------


//	This callback function is called when the window is resized
//	(generally by the user of the application).
//	It is also called when the window is created, why I placed there the
//	code to set up the virtual camera for this application.
//
void myResize(int w, int h)
{
    glutSetWindow(gMainWindow);
	if ((w != WINDOW_WIDTH) || (h != WINDOW_HEIGHT))
	{
		glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
	}
	else
	{
		glutPostRedisplay();
	}
}

//	This function maps world coordinates to screen coordinates.  This is where
//	the projection mode would be defined.  Here I only want to have a 2D
//	rectangle where to draw 2D stuff on top of my image, so I define my camera
//	as an orthographic camera, and I set the world coordinates to coincide with
//	the pixel coordinates (this is what the gluOrtho2D call does).
void setupCamera(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glOrtho(	0.0f, (GLfloat) w, 0.0f, (GLfloat) h, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, w, h);
}

void myDisplay(void)
{
    glutSetWindow(gMainWindow);

    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT);

	#if FOUR_QUADRANT_VERSION
		displayUpperLeft();
		displayUpperRight();
		displayLowerLeft();
		displayLowerRight();
	#else
		displayImagePane();
	#endif
	
    glutSwapBuffers();
    glutSetWindow(gMainWindow);	
}




//	This function is called when a mouse event occurs
//
void myMouse(int button, int state, int x, int y)
{
	switch (button)
	{
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN)
			{
				//	do something
			}
			else if (state == GLUT_UP)
			{
				//	exit(0);
			}
			break;
			
		default:
			break;
	}

	glutSetWindow(gMainWindow);
	glutPostRedisplay();
}


void myTimer(int d)
{
	//	Nothing to do but display the current output image
    myDisplay();

	glutTimerFunc(msecs, myTimer, 0);
}


void initializeFrontEnd(int argc, char** argv)
{
	//	Initialize glut and create a new window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(INIT_WIN_X, INIT_WIN_Y);

	gMainWindow = glutCreateWindow("Blob Trackers -- CSC 412 - Spring 2018");
	glutDisplayFunc(myDisplay);
	glutReshapeFunc(myResize);
	glutMouseFunc(myMouse);
	glutKeyboardFunc(myKeyboard);
	glutTimerFunc(msecs, myTimer, 0);

	#if FOUR_QUADRANT_VERSION

		void (*quadrantDisplayFunc[4])(void) = {displayUpperLeft,
												displayUpperRight,
												displayLowerLeft,
												displayLowerRight};

		glClearColor(0.05f, 0.1f, 0.1f, 1.f);

		int winID = 0;
        for (int i=0; i<2; i++)
        {
            for (int j=0; j<2; j++)
            {
				glutSetWindow(gMainWindow);
				gSubwindow[winID] = glutCreateSubWindow(
                                        gMainWindow,
                                        j*(QUADRANT_WIDTH + PADDING) + PADDING,
                                        i*(QUADRANT_HEIGHT + PADDING) + PADDING,
                                        QUADRANT_WIDTH,
                                        QUADRANT_HEIGHT);
                glClearColor(1.f, 1.f, 0.f, 1.f);

                //	This *has* to be defined:  a "display" callback function for each subwindow
                glutDisplayFunc(quadrantDisplayFunc[winID++]);
                glutKeyboardFunc(myKeyboard);

                //	This calls maps "world" coordinates to "screen" coordinates.
                //	In this particular case I make them coincide.
                setupCamera(QUADRANT_WIDTH, QUADRANT_HEIGHT);
            }
		}
		glutSetWindow(gMainWindow);

	#else
	
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0f, WINDOW_WIDTH, 0.0f, WINDOW_HEIGHT, -1, 1);
		glClearColor(0.f, 0.f, 0.f, 1.f);

	#endif
	
}


