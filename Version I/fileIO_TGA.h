#ifndef	FILE_IO_TGA_H
#define	FILE_IO_TGA_H

#include "fileIO.h"

/**	No-frills function that reads an image file in the <b>uncompressed</b>, un-commented TARGA 
 *	(<tt>.tga</tt>) file format. If the image cannot be read (file not found, invalid format, etc.)
 *	the function simply terminates execution.
 *	@param	filePath	path to the file to read
 *	@return	 a properly initialized ImageStruct storing the image read
 */
ImageStruct readTGA(const char* filePath);

/**	Writes an image file in the <b>uncompressed</b>, un-commented TARGA (<tt>.tga</tt>) file format.
 *	@param	filePath	path to the file to write
 *	@param  info		pointer to the ImageStruct of the image to write into a .tga file.
 *	@return 1 if the image was read successfully, 0 otherwise.
 */
int writeTGA(char* filePath, ImageStruct* info);

#endif
