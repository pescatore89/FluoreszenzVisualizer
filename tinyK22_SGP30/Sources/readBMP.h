/*
 *  readBMP.h
 *
 *  Created by Nina Amenta on Sun May 23 2004.
 *  Free to good home!
 *
 */

#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "CLS1.h"
/* Image type - contains height, width, and data */
struct BMPImage {

    unsigned long biSize;  			//specifies the number of bytes required by the struct
    long biWidth;  					// [18] specifies width in pixels
    long biHeight;  				// [22] species height in pixels
    									/*      Ist der Wert positiv, so ist die Bitmap eine sogenannte "bottom-up"-Bitmap (die Bilddaten beginnen mit der untersten und enden mit der obersten Bildzeile). Dies ist die gebräuchlichste Variante.
        										Ist der Wert negativ, so ist die Bitmap eine "top-down"-Bitmap (die Bilddaten beginnen mit der obersten und enden mit der untersten Bildzeile).*/
    unsigned int bfOffBits;			// [10] Offset der Bilddaten in Byte vom Beginn der Datei an.
    unsigned short biPlanes;		//specifies the number of color planes, must be 1
    unsigned short biBitCount; 		// [28] specifies the number of bit per pixel
    unsigned long biCompression;	//spcifies the type of compression
    unsigned long biSizeImage;  	// [34] size of image in bytes
    long biXPelsPerMeter; 		 	//number of pixels per meter in x axis
    long biYPelsPerMeter;  			//number of pixels per meter in y axis
    unsigned long biClrUsed;  		//number of colors used by th ebitmap
    unsigned long biClrImportant;  	//number of colors that are important
    char *data;						// Farbwerte


};
typedef struct BMPImage BMPImage;

/* Function that reads in the image; first param is filename, second is image struct */
/* As side effect, sets w and h */
int BMPImageLoad(char* filename, BMPImage* image);
uint8_t Read_readBMP(const uint8_t *fileName, const CLS1_StdIOType *io);
uint8_t Display_BMP(const uint8_t *fileName, const CLS1_StdIOType *io);
uint8_t BMP_ParseCommand(const unsigned char *cmd, bool *handled,
		const CLS1_StdIOType *io);
