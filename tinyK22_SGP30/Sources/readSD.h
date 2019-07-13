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
#include "ff.h"
#include "my_types.h"
#include "Message.h"
/* Function that reads in the image; first param is filename, second is image struct */
/* As side effect, sets w and h */
uint8_t BMPImageLoadData(const TCHAR *filename, BMPImage* image, char* data);
uint8_t BMPImageLoadHeader(const TCHAR *filename, BMPImage* image);
uint8_t Read_readBMP(const TCHAR *fileName, const CLS1_StdIOType *io);
uint8_t Display_BMP(const TCHAR *fileName, const CLS1_StdIOType *io);
uint8_t BMP_ParseCommand(const unsigned char *cmd, bool *handled,
		const CLS1_StdIOType *io);
uint8_t readDataFromSD(uint8_t excitation, DataMessage_t * pxData,DATA_t * charData);
uint8_t readImageFromSD(DataMessage_t * pxData);
uint8_t readCharacteristicValues(TCHAR *fileName, DATA_t *pxDATA, uint8_t excitation);
//uint8_t readCharacteristicValues(TCHAR *fileName, Message_t *pxDATA);
void addSuffixBMP(char* filename, uint8_t excitation);
BMPImage* loadBMPData(TCHAR *filename,
		const CLS1_StdIOType *io);
void addSuffixTXT(char* filename);
uint8_t nOfDigits(const char*);
uint32_t getRealValue(const char *value);
// uint8_t ImageDataBuffer[2500];
uint8_t buf[100];
BMPImage* pxImage;
BMPImage xImage;

 uint8_t getDataArray(uint8* px);
