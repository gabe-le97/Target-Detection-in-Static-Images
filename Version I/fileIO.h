#ifndef	FILE_IO_H
#define	FILE_IO_H

/**	This enumerated type is used by the image reading code.  You shouldn't have
 *	to touch this
 */
typedef enum ImageFileType
{
		kUnknownType = -1,
		kTGA_COLOR,				//	24-bit color image
		kTGA_GRAY,
		kPPM,					//	24-bit color image
		kPGM					//	8-bit gray-level image
} ImageFileType;

/**	This is the enum type that refers to images loaded in memory, whether
 *	they were read from a file, played from a movie, captured from a live
 *	video stream, or the result of calculations.
 *	Feel free to edit and add types you need for your project.
 */
typedef enum ImageType
{
		/**	Color image with 4 bytes per pixel
		 */
		RGBA32_RASTER,

		/**	Gray image with 1 byte per pixel
		 */
		GRAY_RASTER,

		/**	Monochrome image (either gray or one color channel of a color image)
		 *	stored in a float raster
		 */
		FLOAT_RASTER
			
} ImageType;

/**	This is the data type to store all relevant information about an image.  After
 *	some thought, I have decided to store the 1D and 2D rasters as void* rather than
 *	having separate unsigned char and float pointers
 */
typedef	struct ImageStruct {
	/**	Type of image stored
	 */
	ImageType type;

	/**	Number of rows (height) of the image
	 */
	unsigned int nbRows;
	
	/**	Number of columns (width) of the image
	 */
	unsigned int nbCols;
	
	/**	Pixel depth
	 */
	unsigned int bytesPerPixel;
	
	/**	Number of bytes per row (which may be larger than
	 *	bytesPerPixel * nbCols if rows are padded to a particular
	 *	word length (e.g. multiple of 16 or 32))
	 */
	unsigned int bytesPerRow;
	
	/**	Pointer to the image data, cast to a void* pointer.  To
	 *	access the data, you would have to cast the pointer to the
	 *	proper type, e.g.
	 *	<ul>
	 *		<li><tt>(int*) raster</tt></li>
	 *		<li><tt>(unsigned char*) raster</tt></li>
	 *		<li><tt>(int*) raster</tt></li>
	 *		<li><tt>(float*) raster</tt></li>
	 *	</ul>
	 */
	void* raster;
	
	/** Similarly here the 2D raster was cast to a void* pointer
	 *  and would need to be cast back to the proper type to be used, e.g.
	 *	<ul>
	 *		<li><tt>(int**) raster</tt></li>
	 *		<li><tt>(unsigned char**) raster</tt></li>
	 *		<li><tt>(int**) raster</tt></li>
	 *		<li><tt>(float**) raster</tt></li>
	 *	</ul>
	 */
	void* raster2D;
} ImageStruct;


#endif
