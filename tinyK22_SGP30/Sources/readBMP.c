/*
 *  readBMP.c
 *
 *  Created by Nina Amenta on Sun May 23 2004.
 *  Free to good home!
 *
 */

#include <stdio.h>      // Header file for standard file i/o.
#include <stdlib.h>     // Header file for malloc/free.
#include "readBMP.h"
#include "FAT1.h"

#define INI_FILE_NAME			"Config.txt"
#define INI_SECTION_NAME_POWER	"POWER"
#define INI_SECTION_NAME_LED	"LED"
#define NUMBER_OF_LEDS					(576)
/* Simple BMP reading code, should be adaptable to many
 systems. Originally from Windows, ported to Linux, now works on my Mac
 OS system.

 NOTE!! only reads 24-bit RGB, single plane, uncompressed, unencoded
 BMP, not all BMPs. BMPs saved by xv should be fine. */

//
// This code was created by Jeff Molofee '99 
//  (www.demonews.com/hosted/nehe)
// Ported to Linux/GLUT by Richard Campbell '99
// Code and comments for adaptation to big endian/little endian systems 
// Nina Amenta '04
//
static uint8_t ReadBMPCmd(const unsigned char *cmd,
		const CLS1_ConstStdIOType *io) {
	/* precondition: cmd starts with "delete" */

#if 0

	uint8_t res = ERR_OK;
	int result = 0;
	FAT1_DEF_NAMEBUF(fileName);
	FAT1_INIT_NAMEBUF(fileName);
	BMPImage* img = NULL;
	img = (BMPImage*) malloc(sizeof(BMPImage));
	if (img != NULL) {
		img->data = malloc(NUMBER_OF_LEDS * sizeof(char));
		if (img->data != NULL) {
			if (UTIL1_ReadEscapedName(cmd + sizeof("delete"),
							FAT1_PTR_NAMEBUF(fileName), FAT1_SIZE_NAMEBUF(fileName),
							NULL,
							NULL,
							NULL) == ERR_OK) {
				CLS1_SendStr((unsigned char*) "reading Bitmap file:  ",
						io->stdOut);

				result = ImageLoad(FAT1_PTR_NAMEBUF(fileName), img);

				CLS1_SendStr((unsigned char*) FAT1_PTR_NAMEBUF(fileName),
						io->stdOut);

				CLS1_SendStr((unsigned char*) "\r\n", io->stdOut);

				/* ok, have now directory name */
			} else {
				CLS1_SendStr((unsigned char*) "reading Bitmap name failed!\r\n",
						io->stdErr);
				res = ERR_FAILED;
			}
		}
	}

	//FAT1_FREE_NAMEBUF(fileName);
	return res;

#endif

#if 0
	uint8_t res = ERR_OK;
	FAT1_DEF_NAMEBUF(fileName);

	FAT1_INIT_NAMEBUF(fileName);
	if (UTIL1_ReadEscapedName(cmd + sizeof("readBMP"),
					FAT1_PTR_NAMEBUF(fileName), FAT1_SIZE_NAMEBUF(fileName), NULL, NULL,
					NULL) == ERR_OK) {
		res = FAT1_readBMP(FAT1_PTR_NAMEBUF(fileName), io);
		// res = FAT1_PrintFile(FAT1_PTR_NAMEBUF(fileName), io);
	} else {
		CmdUsageError(cmd, (unsigned char*) "print fileName", io);
		res = ERR_FAILED;
	}
	FAT1_FREE_NAMEBUF(fileName);
	return res;
#endif
}

uint8_t Read_readBMP(const uint8_t *fileName, const CLS1_StdIOType *io) {

	uint8_t res = ERR_OK;
	int result = 0;
	BMPImage* img = NULL;
	img = malloc(sizeof(BMPImage));
	if (img != NULL) {
		img->data = malloc(NUMBER_OF_LEDS * sizeof(char));
		if (img->data != NULL) {
			CLS1_SendStr((unsigned char*) "reading Bitmap file:  ", io->stdOut);
			CLS1_SendStr((unsigned char*) fileName, io->stdOut);

			result = BMPImageLoad((char *) fileName, img);

			CLS1_SendStr((unsigned char*) "\r\n", io->stdOut);

			/* ok, have now directory name */
		} else {
			CLS1_SendStr((unsigned char*) "reading Bitmap name failed!\r\n",
					io->stdErr);
			res = ERR_FAILED;
		}
	} else {
		res = ERR_FAILED;
	}

	return res;
}

uint8_t BMP_ParseCommand(const unsigned char *cmd, bool *handled,
		const CLS1_StdIOType *io) {
	if (UTIL1_strcmp((char*) cmd, CLS1_CMD_HELP) == 0
			|| UTIL1_strcmp((char*) cmd, "BMP help") == 0) {
		*handled = TRUE;
		//return PrintHelp(io);
	} else if (UTIL1_strncmp((char*) cmd, "BMP readBMP ",
			sizeof("BMP readBMP ") - 1) == 0) {
		*handled = TRUE;
		return Read_readBMP(cmd + sizeof("BMP readBMP"), io);
	}
	return ERR_OK;
}

/* Reads a long 32 bit integer; comment out one or the other shifting line below, 
 whichever makes your system work right. */
unsigned int endianReadInt(FILE* file) {
	unsigned char b[4];
	unsigned int i;

	if (fread(b, 1, 4, file) < 4)
		return 0;
	/* LITTLE VS BIG ENDIAN LINES - comment out one or the other */
	i = (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0]; // big endian
	//i = (b[0]<<24) | (b[1]<<16) | (b[2]<<8) | b[3]; // little endian
	return i;
}

/* Reads a 16 bit integer; comment out one or the other shifting line below, 
 whichever makes your system work right. */
unsigned short int endianReadShort(FILE* file) {
	unsigned char b[2];
	unsigned short s;

	if (fread(b, 1, 2, file) < 2)
		return 0;
	/* LITTLE VS BIG ENDIAN LINES - comment out one or the other */
	s = (b[1] << 8) | b[0]; // big endian
	//s = (b[0]<<8) | b[1]; // little endian
	return s;
}

// quick and dirty bitmap loader...for 24 bit bitmaps with 1 plane only.  
// See http://www.dcs.ed.ac.uk/~mxr/gfx/2d/BMP.txt for more info.

static FIL bmpFile;

int BMPImageLoad(char* filename, BMPImage* image) {
	FILE *filee;
	unsigned long size;                 // size of the image in bytes.
	unsigned long i;                    // standard counter.
	unsigned short int planes;     // number of planes in image (must be 1)
	unsigned short int bpp;         // number of bits per pixel (must be 24)
	char temp;            // temporary color storage for bgr-rgb conversion.
	FIL* file = NULL;
	FRESULT res = FR_OK;
	BMPImage img;
	file = &bmpFile;

#if 0
	if (UTIL1_strcmp(filename,SAVEFILE) == 0) {
		file = &dsaveFileDesc;
	} else if (UTIL1_strcmp(filename,TEXTFILE) == 0) {
		file = &dtextcFileDesc;
	}
	else {
		file = NULL;
	}

#endif

	res = FAT1_open(file, filename, FA_READ);
	if (res != FR_OK) {
		//error occured
		CLS1_SendStr((unsigned char*) "Error reading BMP file ",
				CLS1_GetStdio()->stdOut);
	} else {
		CLS1_SendStr((unsigned char*) "File opened ", CLS1_GetStdio()->stdOut);
		CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
		UINT size;
		UINT nof = 0;
		UINT bfOffBits = 0;

		UINT br; /* Pointer to number of bytes read */

		uint8_t buf[330];

		res = FAT1_read(file, buf, sizeof(buf) - 1, &nof);
		if (res != FR_OK) {
			CLS1_SendStr((unsigned char*) "Error reading BMP file ",
					CLS1_GetStdio()->stdOut);

			//error occured
		} else {
			buf[nof] = '\0'; /* terminate buffer */
			img.sizeX = buf[18];
			img.sizeY = buf[22];
			uint8_t width = buf[18];
			uint8_t height = buf[22];
			bfOffBits = buf[10];											// number of Offsetbits until first Colorvalue
			CLS1_SendStr((unsigned char*) "Breite des Bitmap files:  ",
					CLS1_GetStdio()->stdOut);
			CLS1_SendCh(img.sizeX, CLS1_GetStdio()->stdOut);

			CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
			CLS1_SendStr((unsigned char*) "Höhe des Bitmap files:  ",
					CLS1_GetStdio()->stdOut);
			CLS1_SendCh(img.sizeY, CLS1_GetStdio()->stdOut);


		}

	}

// make sure the file is there.
	if ((filee = fopen(filename, "rb")) == NULL) {
		printf("File Not Found : %s\n", filename);
		return 0;
	}

// seek through the bmp header, up to the width/height:
	fseek(filee, 18, SEEK_CUR);

// read the width
	if (!(image->sizeX = endianReadInt(filee))) {
		printf("Error reading width from %s.\n", filename);
		return 0;
	}
	printf("Width of %s: %lu\n", filename, image->sizeX);

// read the height
	if (!(image->sizeY = endianReadInt(filee))) {
		printf("Error reading height from %s.\n", filename);
		return 0;
	}
	printf("Height of %s: %lu\n", filename, image->sizeY);

// calculate the size (assuming 24 bits or 3 bytes per pixel).
	size = image->sizeX * image->sizeY * 3;

// read the planes
	if (!(planes = endianReadShort(filee))) {
		printf("Error reading planes from %s.\n", filename);
		return 0;
	}
	if (planes != 1) {
		printf("Planes from %s is not 1: %u\n", filename, planes);
		return 0;
	}

// read the bits per pixel
	if (!(bpp = endianReadShort(file))) {
		printf("Error reading bpp from %s.\n", filename);
		return 0;
	}
	if (bpp != 24) {
		printf("Bpp from %s is not 24: %u\n", filename, bpp);
		return 0;
	}

// seek past the rest of the bitmap header.
	fseek(file, 24, SEEK_CUR);

// read the data.
	image->data = (char *) malloc(size);
	if (image->data == NULL) {
		printf("Error allocating memory for color-corrected image data");
		return 0;
	}

	if ((i = fread(image->data, size, 1, file)) != 1) {
		printf("Error reading image data from %s.\n", filename);
		return 0;
	}

	for (i = 0; i < size; i += 3) { // reverse all of the colors. (bgr -> rgb)
		temp = image->data[i];
		image->data[i] = image->data[i + 2];
		image->data[i + 2] = temp;
	}

// we're done.
	return 1;
}

