/*
 * config.c
 *
 *  Created on: 23.11.2018
 *      Author: Pescatore
 */


#include "config.h"
//#if PL_CONFIG_HAS_SD_CARD
#include "FAT1.h"
#include "PORT_PDD.h"
#include "MINI1.h"
#include "readBMP.h"
#include "stdlib.h"
#include <stdio.h>      // Header file for standard file i/o.
#include <stdlib.h>     // Header file for malloc/free.
#define INI_FILE_NAME			"Config.txt"
#define INI_SECTION_NAME_POWER	"POWER"
#define INI_SECTION_NAME_LED	"LED"
//#endif



uint8_t CONFIG_ParseCommand(const unsigned char *cmd, bool *handled,
		const CLS1_StdIOType *io) {
	if (UTIL1_strcmp((char*) cmd, CLS1_CMD_HELP) == 0
			|| UTIL1_strcmp((char*) cmd, "BMP help") == 0) {
		*handled = TRUE;
		//return PrintHelp(io);

	} else if (UTIL1_strncmp((char*) cmd, "CONFIG printConfig",
			sizeof("CONFIG printConfig ") - 1) == 0) { //reads the Config file on the SD-Card
		*handled = TRUE;
		return Config_ReadIni(io);
	}
	return ERR_OK;
}

uint8_t Config_ReadIni(const CLS1_StdIOType *io) {

	int val;
	int power = -1;
	int lines = -1;
	int nLEDsPerLine = -1;
	uint8_t buf[32];
	CLS1_SendStr((unsigned char*) "Loading values from " INI_FILE_NAME "\r\n",
			CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_POWER, "Power_Connected", "0",
			(char*) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr((unsigned char*) "Power enabled: ", CLS1_GetStdio()->stdOut);
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_LED, "nLines", "0", (char*) buf,
			sizeof(buf), INI_FILE_NAME);
	lines = buf[0];
	CLS1_SendStr((unsigned char*) "Lines: ", CLS1_GetStdio()->stdOut);
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_LED, "nLEDs_per_Line", "0",
			(char*) buf, sizeof(buf), INI_FILE_NAME);
	nLEDsPerLine = buf[0];
	CLS1_SendStr((unsigned char*) "LEDs per each line: ",
			CLS1_GetStdio()->stdOut);
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	return ERR_OK;

}



