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
uint8_t BMPImageLoadData(const TCHAR *filename, BMPImage* image);
uint8_t BMPImageLoadHeader(const TCHAR *filename, BMPImage* image);
uint8_t Read_readBMP(const TCHAR *fileName, const CLS1_StdIOType *io);
uint8_t Display_BMP(const TCHAR *fileName, const CLS1_StdIOType *io);
uint8_t BMP_ParseCommand(const unsigned char *cmd, bool *handled,
		const CLS1_StdIOType *io);
uint8_t readCharacteristicValues(TCHAR *fileName,Message_t * pxMessage);
void addSuffixBMP(char* filename);
BMPImage* loadBMPData(TCHAR *filename,
		const CLS1_StdIOType *io);
void addSuffixTXT(char* filename);
uint8_t nOfDigits(const char*);
uint32_t getRealValue(const char *value);
