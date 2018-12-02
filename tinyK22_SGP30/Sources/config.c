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
#include "readSD.h"
#include "stdlib.h"
#include <stdio.h>      // Header file for standard file i/o.
#include <stdlib.h>     // Header file for malloc/free.
#define INI_FILE_NAME			"Config.txt"
#define INI_FILE_NAME_POLLEN	"Pollen.txt"
#define INI_SECTION_NAME_POWER	"POWER"
#define INI_SECTION_NAME_SENSOR "LIGHTSENSOR"
#define INI_SECTION_NAME		"names"
#define INI_SECTION_NAME_LED	"LED"
#define MAX_NAME_LENGTH			 100

//#endif

static uint8_t PrintHelp(const CLS1_StdIOType *io) {
	CLS1_SendHelpStr((unsigned char*) "CONFIG",
			(unsigned char*) "Configurations on the SD-Card\r\n", io->stdOut);
	CLS1_SendHelpStr((unsigned char*) "  printConfig",
			(unsigned char*) "Prints the specific config File\r\n", io->stdOut);
	CLS1_SendHelpStr((unsigned char*) "  help",
			(unsigned char*) "Print help or status information\r\n",
			io->stdOut);
	return ERR_OK;
}

uint8_t CONFIG_ParseCommand(const unsigned char *cmd, bool *handled,
		const CLS1_StdIOType *io) {
	if (UTIL1_strcmp((char*) cmd, CLS1_CMD_HELP) == 0
			|| UTIL1_strcmp((char*) cmd, "BMP help") == 0) {
		*handled = TRUE;
		return PrintHelp(io);

	} else if (UTIL1_strncmp((char*) cmd, "CONFIG printConfig",
			sizeof("CONFIG printConfig ") - 1) == 0) { //reads the Config file on the SD-Card
		*handled = TRUE;
		//return Config_ReadPollen();
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
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr((unsigned char*) "Power enabled: ", CLS1_GetStdio()->stdOut);
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_LED, "nLines", "0", (char* ) buf,
			sizeof(buf), INI_FILE_NAME);
	lines = buf[0];
	CLS1_SendStr((unsigned char*) "Lines: ", CLS1_GetStdio()->stdOut);
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_LED, "nLEDs_per_Line", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	nLEDsPerLine = buf[0];
	CLS1_SendStr((unsigned char*) "LEDs per each line: ",
			CLS1_GetStdio()->stdOut);
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	return ERR_OK;

}

void initConfigData(void) {

	Config_Setup();			// sets up all the Configurations
	Config_ReadPollen();	// reads/stores all the names of the pollen
}

uint8_t Config_Setup(void) {

	int val;
	int power = -1;
	int lines = -1;

	uint8_t buf[32];

	val = MINI1_ini_gets(INI_SECTION_NAME_POWER, "Power_Connected", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	powerEnabled = buf[0];

	val = MINI1_ini_gets(INI_SECTION_NAME_SENSOR, "enabled", "0", (char* ) buf,
			sizeof(buf), INI_FILE_NAME);
	if (buf[0]) {
		lightSensor = TRUE;
	} else {
		lightSensor = FALSE;
	}

	return ERR_OK;

}

uint8_t Config_ReadPollen(void) {

	int val;
	int power = -1;
	int lines = -1;
	int nLEDsPerLine = -1;
	uint8_t buf[100];
	int quantity = 0;
	char key[2];
	char *key_p;
	key_p = key;
	int l = 1;

	CLS1_SendStr(
			(unsigned char*) "Loading names from " INI_FILE_NAME_POLLEN "\r\n",
			CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets("quantity", "n", "0", (char* ) buf, sizeof(buf),
			INI_FILE_NAME_POLLEN);
	quantity = buf[0] - '0';
	CLS1_SendStr((unsigned char*) "Anzahl Pollen: ", CLS1_GetStdio()->stdOut);
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);

	namelist = (char **) malloc(quantity * sizeof(char*));
	if (namelist == NULL) {
		// ups
		return ERR_FAULT;
	}

	for (int z = 0; z < quantity; z++) {
		namelist[z] = (char*) malloc(MAX_NAME_LENGTH * sizeof(char));
		if (namelist[z] == NULL) {
			/*ups*/
			return ERR_FAULT;
		}
	}

	for (int i = 1; i <= quantity; i++) {

		key[0] = i + '0';
		key[1] = '\0';
		CLS1_SendStr((unsigned char*) key_p, CLS1_GetStdio()->stdOut);
		MINI1_ini_gets("names", key_p, "-", (char* ) buf, sizeof(buf),
				INI_FILE_NAME_POLLEN);

		strcpy(namelist[i - 1], buf);

	}

	return ERR_OK;

}

bool getSensorEnabled(void) {

	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	bool temp = lightSensor;
	CS1_ExitCritical()
	;

	return temp;

}

void setSensorEnabled(bool enabled) {

	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	lightSensor = enabled;
	CS1_ExitCritical()
	;
}

char** getNamelist(void) {
	return namelist;
}

