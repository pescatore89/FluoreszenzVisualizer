/*
 *  readBMP.c
 *
 *  Created by Nina Amenta on Sun May 23 2004.
 *  Free to good home!
 *
 */

#include <stdio.h>      // Header file for standard file i/o.
#include <stdlib.h>     // Header file for malloc/free.
#include <ctype.h>
#include "readSD.h"
#include "FAT1.h"
#include "MINI1.h"
#include "CLS1.h"
#include "pollen.h"

#include "WS2812B\NeoPixel.h"
#include "WS2812B\NeoApp.h"
#include "Message.h"
#define INI_FILE_NAME			"Config.txt"
#define INI_SECTION_NAME_POWER	"POWER"
#define INI_SECTION_NAME_LED	"LED"
#define SECTION_NAME_MODE_1		"MODUS1"
#define SECTION_NAME_MODE_2		"MODUS2"
#define SECTION_NAME_MODE_3		"MODUS3"
#define IMAGES_FOLDER_NAME		"Bilder"
#define NUMBER_OF_LEDS					(576)
//static BMPImage* image = NULL;
static BMPImage* image2 = NULL;
static FIL bmpFile;
xQueueHandle queue_handler;
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

static void removeSuffixBMP(char* filename, uint8_t excitation) {
	uint8_t length = 0;
	length = strlen(filename);
	bool has_suffix = FALSE;
	const char ch = '.';
	char nExcitation;

	if (excitation == 1) {
		nExcitation = '1';
	} else if (excitation == 2) {
		nExcitation = '2';
	} else if (excitation == 3) {
		nExcitation = '3';
	}

	char * ret = filename;

	filename = strtok(ret, &nExcitation);

	//filename = ret;
}

uint8_t getDataArray(uint8_t* px) {
//	px = ImageDataBuffer;

}

uint8_t readDataFromSD(uint8_t excitation, DataMessage_t * pxData) {

	//BMPImage* image;
	pxData->image = &xBMPImage;
	DATA_t* charData = NULL;
	char* filename = pxData->name;
	char* polle = NULL;
	DATA_t * pointerData;
	pointerData = &xDATA;
	char nameArray[100];

	strcpy(nameArray, filename);

	char cd[4] = { "\\.." };
	char * cd_back = cd;

	uint8_t result;
	FRESULT res = FR_OK;

	CLS1_StdIOType *io = stdout;
	if (FAT1_ChangeDirectory(nameArray, io) != ERR_OK) {
		/*something wnet wrong*/
	}

	else {

		addSuffixBMP(nameArray, excitation);

		res = BMPImageLoadData(nameArray, pxData->image, pxData->color_data);
		if (res != FR_OK) {
			CLS1_SendStr((unsigned char*) "ERROR loading File  ",
					CLS1_GetStdio()->stdOut);
			return ERR_FAILED;
		} else {

			//	pxData->color_data = image->data;
			//	res = AddMessageToQueue(queue_handler, pxMessage);

		}

	}

	removeSuffixBMP(nameArray, excitation);
	result = readCharacteristicValues(nameArray, pointerData, excitation);
	pxData->char_data = pointerData;

	//FRTOS1_vPortFree(polle);

	if (FAT1_ChangeDirectory(cd_back, io) != ERR_OK) {
		/*something wnet wrong*/
	}

}

uint8_t readImageFromSD(DataMessage_t * pxData) {

	//BMPImage* image;
	pxData->image = &xBMPImage;
	DATA_t* charData = NULL;
	char* filename = pxData->name;
	char* polle = NULL;
	DATA_t * pointerData;
	pointerData = &xDATA;
	char nameArray[100];

	strcpy(nameArray, filename);

	char cd[4] = { "\\.." };
	char * cd_back = cd;

	uint8_t result;
	FRESULT res = FR_OK;

	CLS1_StdIOType *io = stdout;
	if (FAT1_ChangeDirectory(IMAGES_FOLDER_NAME, io) != ERR_OK) {
		/*something wnet wrong*/
	}

	else {

		uint8_t length = 0;
		bool has_suffix = FALSE;
		const char ch = '.';

		char * ret = nameArray;

		char * suffix = ".bmp";
		char * temp = ret;
		strcat(temp, suffix);


		res = BMPImageLoadData(temp, pxData->image, pxData->color_data);
		if (res != FR_OK) {
			CLS1_SendStr((unsigned char*) "ERROR loading File  ",
					CLS1_GetStdio()->stdOut);
			return ERR_FAILED;
		} else {

			//	pxData->color_data = image->data;
			//	res = AddMessageToQueue(queue_handler, pxMessage);

		}

	}

	//FRTOS1_vPortFree(polle);

	if (FAT1_ChangeDirectory(cd_back, io) != ERR_OK) {
		/*something wnet wrong*/
	}

}

uint8_t readCharacteristicValues(TCHAR *fileName, DATA_t* pxDATA,
		uint8_t excitation) {
//	uint8_t readCharacteristicValues(TCHAR *fileName, Message_t * pxDATA) {

	addSuffixTXT(fileName);

	int val;
	int power = -1;
	int lines = -1;
	int nLEDsPerLine = -1;
	uint32_t temp;
	uint8_t buff8[50];

	/*Read out all values beeing part of Excitation  1*/

	if (excitation == 1) {

		val = MINI1_ini_gets(SECTION_NAME_MODE_1, "color266", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->color_266 = getRealValue(buff8);

		val = MINI1_ini_gets(SECTION_NAME_MODE_1, "fadeout266", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->fadeout_266 = getRealValue(buff8);

		val = MINI1_ini_gets(SECTION_NAME_MODE_2, "266wavelength1", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->amplitude_266_1 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_2, "266wavelength2", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->amplitude_266_2 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_2, "266wavelength3", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->amplitude_266_3 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_2, "266wavelength4", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->amplitude_266_4 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_2, "266wavelength5", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->amplitude_266_5 = getRealValue(buff8);

		val = MINI1_ini_gets(SECTION_NAME_MODE_3, "266wavelength1", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->lifetime_266_1 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_3, "266wavelength2", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->lifetime_266_2 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_3, "266wavelength3", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->lifetime_266_3 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_3, "266wavelength4", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->lifetime_266_4 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_3, "266wavelength5", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->lifetime_266_5 = getRealValue(buff8);

	}

	else if (excitation == 2) { /*Read out all values beeing part of Excitation 2*/

		val = MINI1_ini_gets(SECTION_NAME_MODE_1, "color355", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->color_355 = getRealValue(buff8);

		val = MINI1_ini_gets(SECTION_NAME_MODE_1, "fadeout355", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->fadeout_355 = getRealValue(buff8);

		val = MINI1_ini_gets(SECTION_NAME_MODE_2, "355wavelength1", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->amplitude_355_1 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_2, "355wavelength2", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->amplitude_355_2 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_2, "355wavelength3", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->amplitude_355_3 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_2, "355wavelength4", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->amplitude_355_4 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_2, "355wavelength5", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->amplitude_355_5 = getRealValue(buff8);

		val = MINI1_ini_gets(SECTION_NAME_MODE_3, "355wavelength1", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->lifetime_355_1 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_3, "355wavelength2", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->lifetime_355_2 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_3, "355wavelength3", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->lifetime_355_3 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_3, "355wavelength4", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->lifetime_355_4 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_3, "355wavelength5", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->lifetime_355_5 = getRealValue(buff8);

	}

	else if (excitation == 3) { /*Read out all values beeing part of Excitation 3*/

		val = MINI1_ini_gets(SECTION_NAME_MODE_1, "color405", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->color_405 = getRealValue(buff8);

		val = MINI1_ini_gets(SECTION_NAME_MODE_1, "fadeout405", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->fadeout_405 = getRealValue(buff8);

		val = MINI1_ini_gets(SECTION_NAME_MODE_2, "405wavelength1", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->amplitude_405_1 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_2, "405wavelength2", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->amplitude_405_2 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_2, "405wavelength3", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->amplitude_405_3 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_2, "405wavelength4", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->amplitude_405_4 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_2, "405wavelength5", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->amplitude_405_5 = getRealValue(buff8);

		val = MINI1_ini_gets(SECTION_NAME_MODE_3, "405wavelength1", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->lifetime_405_1 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_3, "405wavelength2", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->lifetime_405_2 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_3, "405wavelength3", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->lifetime_405_3 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_3, "405wavelength4", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->lifetime_405_4 = getRealValue(buff8);
		val = MINI1_ini_gets(SECTION_NAME_MODE_3, "405wavelength5", "0",
				(char* ) buff8, sizeof(buff8), fileName);
		pxDATA->lifetime_405_5 = getRealValue(buff8);
	}
}

int getRawInt(char c) {
	if (isalpha(c)) {
		return toupper(c) - 'A' + 10;
	}
	return c - '0';
}







uint32_t getRealValue(const char *value) {
	uint32_t result = 0;
	int dec = 0;
	int power = 1;
	uint8_t nDig = strlen(value);
	uint8_t nDigits = nOfDigits(value);
	for (int k = (nDig - 1); k >= 0; k--) {
		dec += getRawInt(value[k]) * power;
		power *= 16;
	}
	return dec;

}

uint8_t nOfDigits(const char* value) {
	int i = 0;
	uint8_t cnt = 0;

	while (value[cnt] != '\0') {
		cnt++;

	}

	return cnt;

}

#if 0
uint8_t Display_BMP(const TCHAR *fileName, const CLS1_StdIOType *io) {

	BMPImage* image = NULL;
	uint32_t position = 0;
	uint8_t red = 0;
	uint8_t green = 0;
	uint8_t blue = 0;
	uint32_t colorValue = 0;
	uint8_t lane = 0;
	uint32_t color;
	FRESULT res = FR_OK;
	uint32_t cnt = 0;
	Message_t *pxMessage;
	pxMessage = &xMessage;
	unsigned long size;

	if (image == NULL) {
		image = FRTOS1_pvPortMalloc(sizeof(BMPImage));
		if (image == NULL) {
			CLS1_SendStr((unsigned char*) "malloc failed!\r\n", io->stdErr);
			return FR_NOT_ENOUGH_CORE;
			/*malloc failed*/
		} else {
			image->data = FRTOS1_pvPortMalloc(sizeof(char) * 2500);
			if (image->data == NULL) {
				CLS1_SendStr((unsigned char*) "malloc failed!\r\n", io->stdErr);
				return FR_NOT_ENOUGH_CORE;
				/*malloc failed*/
			}
		}
	}

	else if (image->data == NULL) {
		image->data = FRTOS1_pvPortMalloc(sizeof(char) * 2500);

	}
	res = BMPImageLoadData((char *) fileName, image);
	if (res != FR_OK) {
		CLS1_SendStr((unsigned char*) "ERROR loading File  ",
				CLS1_GetStdio()->stdOut);
	} else {

		pxMessage->data = image->data;
		//	res = AddMessageToQueue(queue_handler, pxMessage);
		if (res != QUEUE_OK) {
			return ERR_BUSY;
		}

	}

	return res;

}

#endif

uint8_t Read_readBMP(const TCHAR *fileName, const CLS1_StdIOType *io) {

	BMPImage* image;
	FRESULT res = ERR_OK;
	unsigned char* biCount = NULL;
	//image = malloc(sizeof(BMPImage));
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
		//return Display_BMP(cmd + sizeof("BMP Display"), io);
	}
	return ERR_OK;
}

void addSuffixTXT(char* filename) {

	uint8_t length = 0;
	length = strlen(filename);
	bool has_suffix = FALSE;
	const char ch = '.';
	char * ret;

	for (int i = 0; i < length; i++) {
		if (filename[i] == ch) {
			has_suffix = TRUE;
			break;
		}
	}

	if (has_suffix) {
		ret = strtok(filename, &ch);
	} else {
		ret = filename;
	}
	char * suffix = ".txt";
	char * temp = ret;
	strcat(temp, suffix);
	filename = temp;

}

void addSuffixBMP(char* filename, uint8_t excitation) {

	uint8_t length = 0;
	length = strlen(filename);
	bool has_suffix = FALSE;
	const char ch = '.';
	char* nExcitation;

	if (excitation == 1) {
		nExcitation = "1";
	} else if (excitation == 2) {
		nExcitation = "2";
	} else if (excitation == 3) {
		nExcitation = "3";
	}

	char * ret;

	for (int i = 0; i < length; i++) {
		if (filename[i] == ch) {
			has_suffix = TRUE;
			break;
		}
	}

	if (has_suffix) {
		ret = strtok(filename, &ch);
	} else {
		ret = filename;
	}

	char * suffix = ".bmp";
	char * temp = ret;
	strcat(temp, nExcitation);
	strcat(temp, suffix);
	filename = temp;

}
#if 0
BMPImage* loadBMPData(TCHAR *filename, const CLS1_StdIOType *io) {

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

	addSuffixBMP(filename);
	pxImage = &xImage;
	//pxImage = malloc(sizeof(BMPImage));
	if (pxImage != NULL) {
		res = BMPImageLoadData((char *) filename, pxImage);
		if (res != FR_OK) {
			CLS1_SendStr((unsigned char*) "ERROR loading File  ",
					CLS1_GetStdio()->stdOut);
		}

		//	free(image->data);		needs to be done somewhere else !!!
		//	free(image);			needs to be done somewhere else !!
	} else {
		CLS1_SendStr((unsigned char*) "malloc failed!\r\n", io->stdErr);

	}
	return pxImage;

}
#endif

uint8_t BMPImageLoadData(const TCHAR *filename, BMPImage* image, char* data) {
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
		buf[100];

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
			//image->biCompression = buf[30];
		}

		res = FAT1_lseek(file, image->bfOffBits); // filepointer wird an den Ort verschoben wo die Bilddaten beginnen
		if (res == FR_OK) {
			res = FAT1_read(file, data, sizeof(data) * 2500 - 1, &nof); //
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

	}

	if ((image->biBitCount) == 0x18) { /*24 Bit Farbtiefe*/
		size = ((image->biWidth) * (image->biHeight) * 3);
		for (i = 0; i < size; i += 3) { // reverse all of the colors. (bgr -> rgb)
			temp = data[i];
			data[i] = data[i + 2];
			data[i + 2] = temp;

		}
	} else if ((image->biBitCount) == 0x20) { /*32 Bit Farbtiefe*/
		size = ((image->biWidth) * (image->biHeight) * 4);
		for (i = 0; i < size; i += 4) { // reverse all of the colors. (bgr -> rgb)
			temp = data[i];
			data[i] = data[i + 2];
			data[i + 2] = temp;

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
			//image->biCompression = buf[30];
		}

		res = FAT1_close(file);

	}

	return res;
}
