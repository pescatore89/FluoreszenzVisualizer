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
    unsigned long sizeX;
    unsigned long sizeY;
    char *data;
};
typedef struct BMPImage BMPImage;

/* Function that reads in the image; first param is filename, second is image struct */
/* As side effect, sets w and h */
int BMPImageLoad(char* filename, BMPImage* image);
uint8_t BMP_ParseCommand(const unsigned char *cmd, bool *handled,
		const CLS1_StdIOType *io);
