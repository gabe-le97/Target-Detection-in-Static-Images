//
//  gl_frontEnd.h
//  GL threads
//
//  Created by Jean-Yves Hervé on 2017-04-24.
//  Modified 2018-05-03
//

#ifndef GL_FRONT_END_H
#define GL_FRONT_END_H

//------------------------------------------------------------------------------
//	Find out whether we are on Linux or macOS (sorry, Windows people)
//	and load the OpenGL & glut headers.
//	For macOS, lets us choose which glut to use
//------------------------------------------------------------------------------
#if (defined(__dest_os) && (__dest_os == __mac_os )) || \
	defined(__APPLE_CPP__) || defined(__APPLE_CC__)
	//	Either use the Apple-provided---but deprecated---glut
	//	or the third-party freeglut install
	#if 1
		#include <GLUT/GLUT.h>
	#else
		#include <GL/freeglut.h>
		#include <GL/gl.h>
	#endif
#elif defined(linux)
	#include <GL/glut.h>
#else
	#error unknown OS
#endif

//-----------------------------------------------------------------------------
//	Interface layout decision
//-----------------------------------------------------------------------------

//	The interface can be created in single-pane and 4-quadrants version.
//	The 4 quadrants let you display different things at the same time, but
//	the single pane shows more details of whatever you are showing, obviously
#define FOUR_QUADRANT_VERSION	0

#if FOUR_QUADRANT_VERSION
	#define	UPPER_LEFT	0
	#define UPPER_RIGHT	1
	#define LOWER_LEFT	2
	#define LOWER_RIGHT	3
#endif


//-----------------------------------------------------------------------------
//	Function prototypes
//-----------------------------------------------------------------------------

//	Implemented in gl_frontEnd.c
//-------------------------------
/**	Initializes the glut-based front end for the application
 *	@param	argc	console input number of arguments
 *	@param	argv	console input arguments
 */
void initializeFrontEnd(int argc, char** argv);

//	Implemented in main.c
//-------------------------------

/**	Glut callback functon for keyboard events
 *	@param	c	character of the key hit
 *	@param	x	pointer x location
 *	@param	y	pointer y location
 */
void myKeyboard(unsigned char c, int x, int y);

#if FOUR_QUADRANT_VERSION

	/**	Rendering function for the upper-left quadrant
	 */
	void displayUpperLeft(void);

	/**	Rendering function for the upper-left quadrant
	 */
	void displayUpperRight(void);

	/**	Rendering function for the upper-left quadrant
	 */
	void displayLowerLeft(void);

	/**	Rendering function for the upper-left quadrant
	 */
	void displayLowerRight(void);

#else

	/**	Rendering function in single-pane interface
	 */
	void displayImagePane(void);

#endif


#endif // GL_FRONT_END_H

