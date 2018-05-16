//-----------------------------------------------------------------
//	Implementation of the Blob data type and supporting functions
//  Jean-Yves Hervé, URI 3D Group.  2018-05-02
//-----------------------------------------------------------------

#ifndef BLOB_H
#define BLOB_H

/**	An extent is simply a horizontal segment
 */
typedef struct Extent
{
	/**	x (column) coordinate of the extent's left endpoint
	 */
	unsigned int xL;

	/**	x (column) coordinate of the extent's right endpoint
	 */
	unsigned int xR;
	
	/**	y (row) coordinate of the extent
	 */
	unsigned int y;
	
} Extent;

/**	A sorted list of Extent structs is implemented as an array + size
 */
typedef struct ExtentList
{
	/**	Number of elements in the list
	 */
	unsigned int nbSegs;
	
	/**	The list of segments proper.
	 */
	Extent* segList;
	
} ExtentList;

/**
 *  A Blob  is a data structure to store (and manipulate) a list of
 *	connected pixels.  If you want to get technical, in C++, I would
 *	implement a Blob as a deque of ordered lists of pointers to Extent structs,
 *  a deque being basically like a vector, except that you can add elements
 *	to the tail or to the head of the deque (only to the tail on a vector).
 *
 *	Here, in C, I implement the deque and lists as simple arrays.  This means
 *	that I need to "resize" my arrays when I need to add an element.  This is
 *	not the most efficient way to do it, but it's the simplest and safest.
 */
typedef struct Blob
{
	/**	Number of segments in the blob
	 */
	unsigned int nbSegs;

	/**	Number of pixels in the list
	 */
	unsigned int nbPixels;

	/**	y coordinate of the top part of the blob
	 */
	int yTop;

	/**	y coordinate of the bottom part of the blob
	 */
	int yBottom;

	/**	The array (deque implementation) of lists
	 */
	ExtentList* deque;
	
	/** red channel of the color in which to render the blob
	 */
	unsigned char red;
	
	/** green channel of the color in which to render the blob
	 */
	unsigned char green;

	/** blue channel of the color in which to render the blob
	 */
	unsigned char blue;

} Blob;


#define STACK_STORAGE_INCR	10

/**	Implementation of a stack of Extent structs
 */
typedef struct ExtentStack
{
	/**	Size of storage space allocated for this stack
	 */
	unsigned int storageSize;
	
	/**	Index of the next insertion point on the stack
	 */
	unsigned int stackTop;
	
	Extent* stack;

} ExtentStack;


//----------------------------------------------------------
//	Support functions for ExtentList
//----------------------------------------------------------

/** Add an extent to a list
 *	@param	list 	pointer to the list to add the segment to (ordered
 *					left to right by x coordinate)
 *	@param	seg		the extent to add to the list
 *	@return	1 if all went well, 0 if the extent cannot be added to the list
 *				(its y coordinate is not the same as that of the other extent
 *				in the list).
 */
int addExtentToList(ExtentList* list, Extent seg);

/** Add a segment to a list
 *	@param	list 	pointer to the list to add the segment to (ordered
 *					left to right by x coordinate)
 *	@param	xL		x (column) coordinate of the extent's left endpoint
 *	@param  xR		x (column) coordinate of the extent's right endpoint
 *	@param	y		y (row) coordinate of the extent
 *	@return	1 if all went well, 0 if the segment cannot be added to the list
 *				(its y coordinate is not the same as that of the other extent
 *				in the list).
 */
int addSegmentToList(ExtentList* list, unsigned int xL, unsigned int xR,
					 unsigned int y);

//----------------------------------------------------------
//	Support functions for ExtentStack
//----------------------------------------------------------

/**	Produces a new extent stack properly initialized
 *	@return a new extent stack properly initialized
 */
ExtentStack newExtentStack(void);

/** Add an extent to a stack
 *	@param	eStack 	pointer to the extent stack to push the extent on
 *	@param	seg		the extent to add to the list
 *	@return	1 if all went well, 0 if the extent cannot be added to the list
 *				(its y coordinate is not the same as that of the other segments
 *				in the list).
 */
int addExtentToStack(ExtentStack* eStack, Extent seg);

/** Add a segment to a stack
 *	@param	eStack 	pointer to the segment stack to push the segment on
 *	@param	xL		x (column) coordinate of the extent's left endpoint
 *	@param  xR		x (column) coordinate of the extent's right endpoint
 *	@param	y		y (row) coordinate of the extent
 *	@return	1 if all went well, 0 if the segment cannot be added to the list
 *				(its y coordinate is not the same as that of the extents
 *				in the list).
 */
int addSegmentToStack(ExtentStack* eStack, unsigned int xL, unsigned int xR,
					 unsigned int y);

/** Checks if a stack is empty
 *	@param	eStack 	pointer to the segment stack to test
 *	@return	1 if the stack is empty; 0 otherwise
 */
int stackIsEmpty(ExtentStack* eStack);

/** Pops the top element of an extent stack.  Attempting to pop from an empty
 *	stack will result in termination of the program.
 *	@param	eStack 	pointer to the segment stack to push the segment on
 *	@return	the top segment on the stack. That element is removed from the stack.
 */
Extent popStack(ExtentStack* eStack);

//----------------------------------------------------------
//	Support functions for Blob
//----------------------------------------------------------

/**	Produces a new blob properly initialized
 *	@return a new blob properly initialized
 */
Blob newBlob(void);


/** Add an extent to a blob
 *	@param	blob 	pointer to the blob to add the segment to
 *	@param	seg		the extent to add to the blob
 *	@return	1 if all went well, 0 if the extent cannot be added to the list
 *				(because it is not connected to one of the segments already in
 *				the list).
 */
int addExtentToBlob(Blob* blob, Extent seg);

/** Add a segment to a blob
 *	@param	blob 	pointer to the blob to add the segment to
 *	@param	xL		x (column) coordinate of the extent's left endpoint
 *	@param  xR		x (column) coordinate of the extent's right endpoint
 *	@param	y		y (row) coordinate of the extent
 *	@return	1 if all went well, 0 if the segment cannot be added to the list
 *				(because it is not connected to one of the segments already in
 *				the list).
 */
int addSegmentToBlob(Blob* blob, unsigned int xL, unsigned int xR,
					 unsigned int y);

/**	Render a blob in its assigned color. All the OpenGL setup (e.g. scaling)
 *	is assumed to have been performed before this call is made.
 *
 *	@param	blob pointer to the blob that should be rendered.
 */
void renderBlob(Blob* blob);

/**	Prints out the list of segments in a blob
 *
 *	@param	blob pointer to the blob that should be printed out.
 */
void printoutBlob(Blob* blob);


/**	Delete a blob (frees all heap memory allocated to store it)
 *	@param blob 	pointer to the blob to delete
 */
void deleteBlob(Blob* blob);

#endif //	BLOB_H
