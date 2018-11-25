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
#include "WS2812B\NeoPixel.h"
#include "WS2812B\NeoApp.h"

#define INI_FILE_NAME			"Config.txt"
#define INI_SECTION_NAME_POWER	"POWER"
#define INI_SECTION_NAME_LED	"LED"
#define NUMBER_OF_LEDS					(576)
static BMPImage* image = NULL;
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

}

uint8_t Display_BMP(const TCHAR *fileName, const CLS1_StdIOType *io) {

	uint32_t position = 0;
	uint8_t red = 0;
	uint8_t green = 0;
	uint8_t blue = 0;
	uint32_t colorValue = 0;
	uint8_t lane = 0;
	uint32_t color;
	FRESULT res = FR_OK;
	uint32_t cnt = 0;
	unsigned long size;

	image = malloc(sizeof(BMPImage));
	if (image != NULL) {
		res = BMPImageLoadData((char *) fileName, image);
		if (res != FR_OK) {
			CLS1_SendStr((unsigned char*) "ERROR loading File  ",
					CLS1_GetStdio()->stdOut);
		} else {

			res = NEOA_Display_Image(image);
		}

		free(image->data);
		free(image);
	} else {
		CLS1_SendStr((unsigned char*) "malloc failed!\r\n", io->stdErr);
		return FR_NOT_ENOUGH_CORE;
	}
	return res;

}

uint8_t Read_readBMP(const TCHAR *fileName, const CLS1_StdIOType *io) {

	FRESULT res = ERR_OK;
	unsigned char* biCount = NULL;
	image = malloc(sizeof(BMPImage));
	if (image != NULL) {

		CLS1_SendStr((unsigned char*) "reading Bitmap file:  ", io->stdOut);
		CLS1_SendStr((unsigned char*) fileName, io->stdOut);

		res = BMPImageLoadHeader((char *) fileName, image);
		if (res != FR_OK) {
			CLS1_SendStr((unsigned char*) "ERROR reading File  ",
					CLS1_GetStdio()->stdOut);
		} else {
			CLS1_SendStr((unsigned char*) "\r\n", io->stdOut);
			CLS1_SendStr((unsigned char*) "Breite des Bitmap files:  ",
					CLS1_GetStdio()->stdOut);
			CLS1_SendCh(image->biWidth, CLS1_GetStdio()->stdOut);
			CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
			CLS1_SendStr((unsigned char*) "Höhe des Bitmap files:  ",
					CLS1_GetStdio()->stdOut);
			CLS1_SendCh(image->biHeight, CLS1_GetStdio()->stdOut);
			CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
			if (image->biBitCount == 0x18) {
				biCount = "24 Bit Farbtiefe ";
			} else if (image->biBitCount == 0x20) {
				biCount = "Farbtiefe beträgt 32 Bit";
			} else if ((image->biBitCount == 0x01)
					|| (image->biBitCount == 0x04)
					|| (image->biBitCount == 0x08)
					|| (image->biBitCount == 0x10)) {
				biCount = "Falsche Farbtiefe";
			}
			CLS1_SendStr(biCount, CLS1_GetStdio()->stdOut);
			CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
		}

		free(image);
		/* ok, have now directory name */
	} else {
		CLS1_SendStr((unsigned char*) "malloc failed!\r\n", io->stdErr);
		res = ERR_FAILED;
	}

	return res;
}

static uint8_t PrintHelp(const CLS1_StdIOType *io) {
	CLS1_SendHelpStr((unsigned char*) "BMP",
			(unsigned char*) "Group of Bitmap commands\r\n", io->stdOut);
	CLS1_SendHelpStr((unsigned char*) "  readBMP <filename>",
			(unsigned char*) "Returns the width and height of the specific Bitmap file\r\n",
			io->stdOut);
	CLS1_SendHelpStr((unsigned char*) "  Display <filenam>",
			(unsigned char*) "Displays the specific BMP file\r\n", io->stdOut);
	CLS1_SendHelpStr((unsigned char*) "  help|status",
			(unsigned char*) "Print help or status information\r\n",
			io->stdOut);
	return ERR_OK;
}

#if 0
static uint8_t PrintStatus(const CLS1_StdIOType *io) {
	uint8_t buf[32];
	uint8_t res;

	CLS1_SendStatusStr((unsigned char*) "BMP", (unsigned char*) "\r\n",
			io->stdOut);
	UTIL1_Num8uToStr(buf, sizeof(buf), NEOA_LightLevel);
	UTIL1_strcat(buf, sizeof(buf),
			NEOA_isAutoLightLevel ? " (auto)\r\n" : " (fix)\r\n");
	CLS1_SendStatusStr("  lightlevel", buf, io->stdOut);
	UTIL1_strcpy(buf, sizeof(buf),
			NEOA_useGammaCorrection ? "on\r\n" : "off\r\n");
	CLS1_SendStatusStr("  gamma", buf, io->stdOut);
	return ERR_OK;
}
#endif

uint8_t BMP_ParseCommand(const unsigned char *cmd, bool *handled,
		const CLS1_StdIOType *io) {
	if (UTIL1_strcmp((char*) cmd, CLS1_CMD_HELP) == 0
			|| UTIL1_strcmp((char*) cmd, "BMP help") == 0) {
		*handled = TRUE;
		return PrintHelp(io);
	} else if (UTIL1_strncmp((char*) cmd, "BMP readBMP ",
			sizeof("BMP readBMP ") - 1) == 0) {
		*handled = TRUE;
		return Read_readBMP(cmd + sizeof("BMP readBMP"), io);
	} else if (UTIL1_strncmp((char*) cmd, "BMP Display ",
			sizeof("BMP Display ") - 1) == 0) {
		*handled = TRUE;
		return Display_BMP(cmd + sizeof("BMP Display"), io);
	}
	return ERR_OK;
}

static FIL bmpFile;

uint8_t BMPImageLoadData(const TCHAR *filename, BMPImage* image) {
	FILE *filee;
	unsigned long size;                 // size of the image in bytes.
	unsigned long i;                    // standard counter.
	unsigned short int planes;     	// number of planes in image (must be 1)
	unsigned short int bpp;         // number of bits per pixel (must be 24)
	char temp;            // temporary color storage for bgr-rgb conversion.
	FIL* file = NULL;
	FRESULT res = FR_OK;
	file = &bmpFile;

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
		uint8_t buf[2400];

		res = FAT1_read(file, buf, sizeof(buf) - 1, &nof);
		if (res != FR_OK) {
			CLS1_SendStr((unsigned char*) "Error reading BMP file ",
					CLS1_GetStdio()->stdOut);
			return res;
		} else {
			buf[nof] = '\0'; /* terminate buffer */
			image->biWidth = buf[18];
			image->biHeight = buf[22];
			image->bfOffBits = buf[10];
			image->biBitCount = buf[28];
			image->biSizeImage = buf[34];
			image->biCompression = buf[30];
		}



		image->data = malloc(
				image->biWidth * image->biHeight * sizeof(char) * ((image->biBitCount)/8));
		if (image->data != NULL) {
			res = FAT1_lseek(file, image->bfOffBits); // filepointer wird an den Ort verschoben wo die Bilddaten beginnen
			if (res == FR_OK) {
				res = FAT1_read(file, image->data, sizeof(buf) - 1, &nof); //
				if (res == FR_OK) {
					res = FAT1_close(file);
				} else {
					res = FR_DENIED; 	// close failed
					return res;
				}
			} else {
				res = FR_NOT_ENABLED; 	// seek failed
				return res;
			}
		} else {
			res = FR_NOT_ENOUGH_CORE;	// malloc failed
			return res;
		}
	}

	if ((image->biBitCount) == 0x18) { /*24 Bit Farbtiefe*/
		size = ((image->biWidth) * (image->biHeight) * 3);
		for (i = 0; i < size; i += 3) { // reverse all of the colors. (bgr -> rgb)
			temp = image->data[i];
			image->data[i] = image->data[i + 2];
			image->data[i + 2] = temp;

		}
	} else if ((image->biBitCount) == 0x20) { /*32 Bit Farbtiefe*/
		size = ((image->biWidth) * (image->biHeight) * 4);
		for (i = 0; i < size; i += 4) { // reverse all of the colors. (bgr -> rgb)
			temp = image->data[i];
			image->data[i] = image->data[i + 2];
			image->data[i + 2] = temp;

		}
	}

	return res;
}

uint8_t BMPImageLoadHeader(const TCHAR *filename, BMPImage* image) {
	FILE *filee;
	unsigned long size;                 // size of the image in bytes.
	unsigned long i;                    // standard counter.
	unsigned short int planes;     	// number of planes in image (must be 1)
	unsigned short int bpp;         // number of bits per pixel (must be 24)
	char temp;            // temporary color storage for bgr-rgb conversion.
	FIL* file = NULL;
	FRESULT res = FR_OK;
	file = &bmpFile;

	res = FAT1_open(file, filename, FA_READ);
	if (res != FR_OK) {
		//error occured
		CLS1_SendStr((unsigned char*) "Error reading BMP file Header ",
				CLS1_GetStdio()->stdOut);
	} else {
		CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
		CLS1_SendStr((unsigned char*) "File opened ", CLS1_GetStdio()->stdOut);
		CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
		UINT nof = 0;
		UINT bfOffBits = 0;
		UINT br; /* Pointer to number of bytes read */
		uint8_t buf[50];

		res = FAT1_read(file, buf, sizeof(buf) - 1, &nof);
		if (res != FR_OK) {
			CLS1_SendStr((unsigned char*) "Error reading BMP file ",
					CLS1_GetStdio()->stdOut);
			return res;
		} else {
			CLS1_SendStr((unsigned char*) "Reading BMP file ",
					CLS1_GetStdio()->stdOut);
			buf[nof] = '\0'; /* terminate buffer */
			image->biWidth = buf[18];
			image->biHeight = buf[22];
			image->bfOffBits = buf[10];
			image->biBitCount = buf[28];
			image->biSizeImage = buf[34];
			image->biCompression = buf[30];
		}

		res = FAT1_close(file);

	}

	return res;
}
