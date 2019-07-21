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
#define INI_FILE_NAME_IMAGES	"Images.txt"
#define INI_SECTION_NAME_POWER	"POWER"
#define INI_SECTION_NAME_TIMING	"TIMING"
#define INI_SECTION_NAME_COLOR	"COLOR"
#define INI_SECTION_NAME_LETTER	"LETTER"
#define INI_SECTION_NAME_SENSOR "LIGHTSENSOR"
#define INI_SECTION_NAME		"names"
#define INI_SECTION_NAME_LED	"LED"
#define INI_SECTION_NAME_LCD	"turnOffTime"
#define INI_SECTION_NAME_NO_POLLEN "CONFIG"
#define MAX_NAME_LENGTH			 100

//#endif
static uint8_t powerEnabled;
static uint8_t maxNoOfPollen;
static uint8_t letter_A;
static uint8_t letter_T;
static uint32_t trailSpeed;
static uint32_t currentPowerSupply;
static uint32_t LCD_turn_off_time;
static uint32_t Screensaver_time;
static uint8_t maxCurrentPerLEDPixel;
static uint32_t seq2_color[10]; /*Color for the sequence 2*/
static uint32_t seq3_color[10]; /*Color for the sequence 3*/
static uint32_t timing[100]; /*0 --> timingDelayBetweenSeq1_2;
 1 --> timingDisplaySeq2
 2 --> timingDelayBetweenSeq2_3
 ...	*/

static char** namelist;	// Platzhalter für die namen der Pollen
static char** imageslist; //Namen der Bilder

static int quantity;		// Anzahl der gespeicherten Pollen
static int quantityimages = 0;	// Anzahl der gespeicherten Bilder

static uint8_t PrintHelp(const CLS1_StdIOType *io) {
	CLS1_SendHelpStr((unsigned char*) "CONFIG",
			(unsigned char*) "Configurations on the SD-Card\r\n", io->stdOut);
	CLS1_SendHelpStr((unsigned char*) "  printConfig original",
			(unsigned char*) "Prints the specific config File on the SD-Card\r\n",
			io->stdOut);
	CLS1_SendHelpStr((unsigned char*) "  load original",
			(unsigned char*) "load original values from Config.txt\r\n",
			io->stdOut);
	CLS1_SendHelpStr((unsigned char*) "  help",
			(unsigned char*) "Print help or status information\r\n",
			io->stdOut);
	CLS1_SendHelpStr((unsigned char*) "  set <Section> <key> <value>",
			(unsigned char*) "Possibility to set new values\r\n", io->stdOut);

	return ERR_OK;
}

static void setTiming(uint32_t * value) {

	timing[0] = value[0];
	timing[1] = value[1];
	timing[2] = value[2];
	timing[3] = value[3];

}

static void setLetterEnabled(char letter, uint8_t enabled) {
	if (letter == 'A') {
		letter_A = enabled;
	} else if (letter == 'T') {
		letter_T = enabled;
	}
}

void setPowerConnected(uint8_t val) {
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	powerEnabled = val;
	CS1_ExitCritical()
	;

}

void setPowerSupplyCurrent(uint32_t val) {
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	currentPowerSupply = val;
	CS1_ExitCritical()
	;

}



void setLCDturnOffTime(uint32_t val){
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	LCD_turn_off_time = val;
	CS1_ExitCritical()
	;
}


void setMaxNoOfPollen(uint8_t val){
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	maxNoOfPollen = val;
	CS1_ExitCritical()
	;
}


void setScreensaverTime(uint32_t val){
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	Screensaver_time = val;
	CS1_ExitCritical()
	;
}


void setCurrentPerPixel(uint8_t val) {
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	maxCurrentPerLEDPixel = val;
	CS1_ExitCritical()
	;

}

static void setTrailSpeed(uint32_t speed) {
	trailSpeed = speed;
}

static void setSequenzColor(uint8_t sequenz, uint8_t pos, uint32_t color) {

	if (sequenz == 2) {
		seq2_color[pos] = color;
	} else if (sequenz == 3) {
		seq3_color[pos] = color;
	}

}

static uint8_t setNewValue(char *section, char* key, char* value) {

	uint32_t Tbuf[32];
	uint8_t res = ERR_OK;
	if (!(strcmp(INI_SECTION_NAME_POWER, section))) {
		if (!(strcmp("Power_Connected", key))) {
			setPowerConnected(getRealValue(value));
		} else if (!(strcmp("Max_LED_Current", key))) {
			setCurrentPerPixel(getRealValue(value));
		} else if (!(strcmp("Power_Supply_Current", key))) {
			setPowerSupplyCurrent(getRealValue(value));
		} else {
			res = ERR_FAILED;
		}
#if 0
	} else if (!(strcmp(INI_SECTION_NAME_LED, section))) {
#endif
	} else if (!(strcmp(INI_SECTION_NAME_SENSOR, section))) {
		if (!(strcmp("enabled", key))) {
			setSensorEnabled(getRealValue(value));
		} else {
			res = ERR_FAILED;
		}

	} else if (!(strcmp(INI_SECTION_NAME_TIMING, section))) {
		if (!(strcmp("timeBetweenSeq1_2", key))) {
			timing[0] = getRealValue(value);
		} else if (!(strcmp("timeDisplaySeq2", key))) {
			timing[1] = getRealValue(value);
		} else if (!(strcmp("timeBetweenSeq2_3", key))) {
			timing[2] = getRealValue(value);
		} else if (!(strcmp("timeBetweenSeq3_1", key))) {
			timing[3] = getRealValue(value);
		} else if (!(strcmp("Trail_Speed", key))) {
			setTrailSpeed(getRealValue(value));
		}

		else {
			res = ERR_FAILED;
		}

	} else if (!(strcmp(INI_SECTION_NAME_COLOR, section))) {

		if (!(strcmp("SEQ2_COLOR1", key))) {
			setSequenzColor(2, 0, getRealValue(value));
		} else if (!(strcmp("SEQ2_COLOR2", key))) {
			setSequenzColor(2, 1, getRealValue(value));
		} else if (!(strcmp("SEQ2_COLOR3", key))) {
			setSequenzColor(2, 2, getRealValue(value));
		} else if (!(strcmp("SEQ2_COLOR4", key))) {
			setSequenzColor(2, 3, getRealValue(value));
		} else if (!(strcmp("SEQ2_COLOR5", key))) {
			setSequenzColor(2, 4, getRealValue(value));
		}

		else if (!(strcmp("SEQ3_COLOR1", key))) {
			setSequenzColor(3, 0, getRealValue(value));
		} else if (!(strcmp("SEQ3_COLOR2", key))) {
			setSequenzColor(3, 1, getRealValue(value));
		} else if (!(strcmp("SEQ3_COLOR3", key))) {
			setSequenzColor(3, 2, getRealValue(value));
		} else if (!(strcmp("SEQ3_COLOR4", key))) {
			setSequenzColor(3, 3, getRealValue(value));
		} else {
			res = ERR_FAILED;
		}

	} else if (!(strcmp(INI_SECTION_NAME_LETTER, section))) {
		setLetterEnabled(key[0], getRealValue(value));

	} else {
		res = ERR_FAILED;
	}

	return res;

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
	char** res;
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	res = namelist;
	CS1_ExitCritical()
	;

	return res;
}

int getQuantity(void) {
	return quantity;
}

char** getImagesList(void) {
	char** res;
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	res = imageslist;
	CS1_ExitCritical()
	;

	return res;
}
int getQuantityOfImages(void) {
	return quantityimages;
}

uint8_t getLetterEnabled(char letter) {
	if (letter == 'A') {
		return letter_A;
	} else if (letter == 'T') {
		return letter_T;
	}

}

uint32_t getSequenzColor(uint8_t sequenz, uint8_t pos) {

	if (sequenz == 2) {
		return seq2_color[pos - 1];
	} else if (sequenz == 3) {
		return seq3_color[pos - 1];
	}

}

uint32_t getTiming(uint8_t pos) {
	return timing[pos];

}









uint8_t CONFIG_ParseCommand(const unsigned char *cmd, bool *handled,
		const CLS1_StdIOType *io) {
	const uint8_t *p;
	int32_t tmp, lane, pos, x, y, holo;
	char * section;
	char* key;
	char* value;

	uint8_t res = ERR_OK;

	const char space[] = "\x20 ";

	char val[100];

	if (UTIL1_strcmp((char*) cmd, CLS1_CMD_HELP) == 0
			|| UTIL1_strcmp((char*) cmd, "CONFIG help") == 0) {
		*handled = TRUE;
		return PrintHelp(io);

	} else if (UTIL1_strncmp((char*) cmd, "CONFIG printConfig original",
			sizeof("CONFIG printConfig original ") - 1) == 0) { //reads the Config file on the SD-Card
		*handled = TRUE;
		//return Config_ReadPollen();
		return Config_ReadIni(io);

	} else if (UTIL1_strncmp((char*) cmd, "CONFIG load original",
			sizeof("CONFIG load original ") - 1) == 0) { //reads the Config file on the SD-Card
		*handled = TRUE;
		//return Config_ReadPollen();
		return Config_Setup();


	} else if (UTIL1_strncmp((char* )cmd, "CONFIG set",
			sizeof("CONFIG set") - 1) == 0) {
		p = cmd + sizeof("CONFIG set ") - 1;

		strcpy(val, p);
		section = strtok(val, &space);
		key = strtok(NULL, &space);
		value = strtok(NULL, &space);
		*handled = TRUE;
		res = setNewValue(section, key, value);

	}

	else {
		*handled = FALSE;
		res = ERR_FAILED;
	}

	return res;
}



uint8_t Config_ReadIni(const CLS1_StdIOType *io) {

	int val;
	int power = -1;
	int lines = -1;
	int nLEDsPerLine = -1;
	uint8_t buf[32];
	CLS1_SendStr((unsigned char*) "Loading values from " INI_FILE_NAME "\r\n",
			CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "[POWER]\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "Power_Connected = ",
			CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_POWER, "Power_Connected", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "Max_LED_Current = ",
			CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_POWER, "Max_LED_Current", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "Power_Supply_Current  = ",
			CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_POWER, "Power_Supply_Current", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "[LED]\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "nLines  = ", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_LED, "nLines", "0", (char* ) buf,
			sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "nLEDs_per_Line  = ",
			CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_LED, "nLEDs_per_Line", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "[LIGHTSENSOR]\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "enabled  = ", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_SENSOR, "enabled", "-1", (char* ) buf,
			sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "[TIMING]\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "timeBetweenSeq1_2  = ",
			CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_TIMING, "timeBetweenSeq1_2", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "timeDisplaySeq2  = ",
			CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_TIMING, "timeDisplaySeq2", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "timeBetweenSeq2_3  = ",
			CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_TIMING, "timeBetweenSeq2_3", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "timeBetweenSeq3_1  = ",
			CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_TIMING, "timeBetweenSeq3_1", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "Trail_Speed  = ", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_TIMING, "Trail_Speed", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "[COLOR]\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "SEQ2_COLOR1  = ", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ2_COLOR1", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "SEQ2_COLOR2  = ", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ2_COLOR2", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "SEQ2_COLOR3  = ", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ2_COLOR3", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "SEQ2_COLOR4  = ", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ2_COLOR4", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "SEQ2_COLOR5  = ", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ2_COLOR5", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "SEQ3_COLOR1  = ", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ3_COLOR1", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "SEQ3_COLOR2  = ", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ3_COLOR2", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "SEQ3_COLOR3  = ", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ3_COLOR3", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "SEQ3_COLOR4  = ", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ3_COLOR4", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "[LETTER]\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "A  = ", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_LETTER, "A", "0", (char* ) buf,
			sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

	CLS1_SendStr((unsigned char*) "T  = ", CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets(INI_SECTION_NAME_LETTER, "T", "0", (char* ) buf,
			sizeof(buf), INI_FILE_NAME);
	power = buf[0];
	CLS1_SendStr(buf, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\r\n", CLS1_GetStdio()->stdOut);

#if 0

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
#endif
	return ERR_OK;

}

void initConfigData(void) {


	Config_Setup();			// sets up all the Configurations
	Config_ReadPollen();	// reads/stores all the names of the pollen
	Config_ReadImages();	// reads/stores all the names of the Images
//	Config_StorePollen(pxDataMessage);
}

uint8_t getMaxNoOfPollen(void){
	uint8_t res;
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	res = maxNoOfPollen;
	CS1_ExitCritical()
	;

	return res;
}


uint8_t getPowerConnected(void) {

	uint8_t res;
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	res = powerEnabled;
	CS1_ExitCritical()
	;

	return res;
}

uint32_t getPowerSupplyCurrent(void) {
	uint32_t res;
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	res = currentPowerSupply;
	CS1_ExitCritical()
	;

	return res;
}


uint32_t getLCDTurnOffTime(void) {
	uint32_t res;
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	res = LCD_turn_off_time;
	CS1_ExitCritical()
	;

	return res;
}


uint32_t getScreensaverTime(void){
	uint32_t res;
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	res = Screensaver_time;
	CS1_ExitCritical()
	;

	return res;

}


uint8_t getCurrentPerPixel(void) {
	uint8_t res;
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	res = maxCurrentPerLEDPixel;
	CS1_ExitCritical()
	;

	return res;
}

uint32_t getTrailSpeed(void) {
	return trailSpeed;
}

static uint32_t getColorValue(char* value) {

	uint8_t nDig = strlen(value);
	uint32_t color;
	color = getRealValue(value);

	return color;

}

uint8_t Config_Setup(void) {

	int val;
	int power = -1;
	int lines = -1;
	uint8_t buf[32];
	uint32_t Tbuf[32];

	/*read and Setup Timing*/
	val = MINI1_ini_gets(INI_SECTION_NAME_TIMING, "timeBetweenSeq1_2", "-1",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	Tbuf[0] = getRealValue(buf);

	val = MINI1_ini_gets(INI_SECTION_NAME_TIMING, "timeDisplaySeq2", "-1",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	Tbuf[1] = getRealValue(buf);

	val = MINI1_ini_gets(INI_SECTION_NAME_TIMING, "timeBetweenSeq2_3", "-1",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	Tbuf[2] = getRealValue(buf);

	val = MINI1_ini_gets(INI_SECTION_NAME_TIMING, "timeBetweenSeq3_1", "-1",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	Tbuf[3] = getRealValue(buf);

	setTiming(Tbuf);

	val = MINI1_ini_gets(INI_SECTION_NAME_TIMING, "Trail_Speed", "-1",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	setTrailSpeed(getRealValue(buf));

	/*read and Setup other values*/
	val = MINI1_ini_gets(INI_SECTION_NAME_POWER, "Power_Connected", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	setPowerConnected(buf[0] - (char) '0');

	val = MINI1_ini_gets(INI_SECTION_NAME_POWER, "Max_LED_Current", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	setCurrentPerPixel(getRealValue(buf));

	val = MINI1_ini_gets(INI_SECTION_NAME_POWER, "Power_Supply_Current", "0",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	setPowerSupplyCurrent(getRealValue(buf));

	val = MINI1_ini_gets(INI_SECTION_NAME_SENSOR, "enabled", "0", (char* ) buf,
			sizeof(buf), INI_FILE_NAME);
	if ((buf[0] - '0')) {
		lightSensor = TRUE;
	} else {
		lightSensor = FALSE;
	}


	val = MINI1_ini_gets(INI_SECTION_NAME_NO_POLLEN, "maxNoOfPollen", "5",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	setMaxNoOfPollen(getRealValue(buf));


	val = MINI1_ini_gets(INI_SECTION_NAME_LCD, "LCD_turn_off_time", "1D4C0",		// default wert bei 2 min
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	setLCDturnOffTime(getRealValue(buf));


	val = MINI1_ini_gets(INI_SECTION_NAME_LCD, "Screensaver_time", "1D4C0",		// default wert bei 2 min
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	setScreensaverTime(getRealValue(buf));



	/*COLOR Values for Sequenz 2*/
	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ2_COLOR1", "-1",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	Tbuf[0] = getColorValue(buf);
	setSequenzColor(2, 0, Tbuf[0]);

	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ2_COLOR2", "-1",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	Tbuf[0] = getColorValue(buf);
	setSequenzColor(2, 1, Tbuf[0]);

	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ2_COLOR3", "-1",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	Tbuf[0] = getColorValue(buf);
	setSequenzColor(2, 2, Tbuf[0]);

	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ2_COLOR4", "-1",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	Tbuf[0] = getColorValue(buf);
	setSequenzColor(2, 3, Tbuf[0]);

	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ2_COLOR5", "-1",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	Tbuf[0] = getColorValue(buf);
	setSequenzColor(2, 4, Tbuf[0]);

	/*COLOR Values for Sequenz 3*/
	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ3_COLOR1", "-1",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	Tbuf[0] = getColorValue(buf);
	setSequenzColor(3, 0, Tbuf[0]);

	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ3_COLOR2", "-1",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	Tbuf[0] = getColorValue(buf);
	setSequenzColor(3, 1, Tbuf[0]);

	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ3_COLOR3", "-1",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	Tbuf[0] = getColorValue(buf);
	setSequenzColor(3, 2, Tbuf[0]);

	val = MINI1_ini_gets(INI_SECTION_NAME_COLOR, "SEQ3_COLOR4", "-1",
			(char* ) buf, sizeof(buf), INI_FILE_NAME);
	Tbuf[0] = getColorValue(buf);
	setSequenzColor(3, 3, Tbuf[0]);

	/*Configuration fpr the Letters*/

	val = MINI1_ini_gets(INI_SECTION_NAME_LETTER, "A", "-1", (char* ) buf,
			sizeof(buf), INI_FILE_NAME);
	if ((buf[0] - '0')) {
		letter_A = TRUE;
	} else {
		letter_A = FALSE;
	}

	val = MINI1_ini_gets(INI_SECTION_NAME_LETTER, "T", "-1", (char* ) buf,
			sizeof(buf), INI_FILE_NAME);
	if ((buf[0] - '0')) {
		letter_T = TRUE;
	} else {
		letter_T = FALSE;
	}

	return ERR_OK;
}

uint8_t Config_ReadPollen(void) {

	int val;
	int power = -1;
	int lines = -1;
	int nLEDsPerLine = -1;
	uint8_t buf[100];

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

	CLS1_SendCh(quantity, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\n\r", CLS1_GetStdio()->stdOut);

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

		MINI1_ini_gets("names", key_p, "-", (char* ) buf, sizeof(buf),
				INI_FILE_NAME_POLLEN);

		strcpy(namelist[i - 1], buf);
		CLS1_SendStr((unsigned char*) "-", CLS1_GetStdio()->stdOut);
		CLS1_SendStr((unsigned char*) namelist[i - 1], CLS1_GetStdio()->stdOut);
		CLS1_SendStr((unsigned char*) "\n\r", CLS1_GetStdio()->stdOut);

	}

	return ERR_OK;

}

uint8_t Config_ReadImages(void) {

	int val;
	int power = -1;
	int lines = -1;
	int nLEDsPerLine = -1;
	uint8_t buf[100];

	char key[2];
	char *key_p;
	key_p = key;
	int l = 1;

	CLS1_SendStr(
			(unsigned char*) "Loading Images from " INI_FILE_NAME_IMAGES "\r\n",
			CLS1_GetStdio()->stdOut);
	val = MINI1_ini_gets("quantity", "n", "0", (char* ) buf, sizeof(buf),
			INI_FILE_NAME_IMAGES);
	quantityimages = buf[0] - '0';
	CLS1_SendStr((unsigned char*) "Anzahl Bilder: ", CLS1_GetStdio()->stdOut);

	CLS1_SendCh(quantityimages, CLS1_GetStdio()->stdOut);
	CLS1_SendStr((unsigned char*) "\n\r", CLS1_GetStdio()->stdOut);

	imageslist = (char **) malloc(quantityimages * sizeof(char*));
	if (imageslist == NULL) {
		// ups
		return ERR_FAULT;
	}

	for (int z = 0; z < quantityimages; z++) {
		imageslist[z] = (char*) malloc(MAX_NAME_LENGTH * sizeof(char));
		if (imageslist[z] == NULL) {
			/*ups*/
			return ERR_FAULT;
		}
	}

	for (int i = 1; i <= quantityimages; i++) {

		key[0] = i + '0';
		key[1] = '\0';

		MINI1_ini_gets("names", key_p, "-", (char* ) buf, sizeof(buf),
				INI_FILE_NAME_IMAGES);

		strcpy(imageslist[i - 1], buf);
		CLS1_SendStr((unsigned char*) "-", CLS1_GetStdio()->stdOut);
		CLS1_SendStr((unsigned char*) imageslist[i - 1],
				CLS1_GetStdio()->stdOut);
		CLS1_SendStr((unsigned char*) "\n\r", CLS1_GetStdio()->stdOut);

	}

	return ERR_OK;

}



uint8_t Config_StorePollen(struct DataMessage * ptr) {


	uint8_t res = ERR_OK;



	ptr = FRTOS1_pvPortMalloc(sizeof(DataMessage_t) * quantity);

	if ((ptr) == NULL) {
		res = ERR_FAILED;
		return res;
		/*malloc failed*/
	}

	for (int i = 0; i < 1; i++) {
		(ptr + i)->color_data = FRTOS1_pvPortMalloc(
				sizeof(char) * 2500);
		if (((ptr + i)->color_data) == NULL) {
			res = ERR_FAILED;
			/*malloc failed*/
		}

	}

	return res;

}

