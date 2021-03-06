/*
 * Pollen.c
 *
 *  Created on: 25.11.2018
 *      Author: Pescatore
 */

#include "Pollen.h"
#include "FAT1.h"
#include "MINI1.h"
#include "readSD.h"
#include "FRTOS1.h"
#include "WS2812B\NeoPixel.h"
#include "WS2812B\NeoApp.h"
xQueueHandle queue_handler;
xQueueHandle queue_handler_Navigation; /*QueueHandler declared in Message.h*/
xSemaphoreHandle mutex;
static bool directory_set;
static uint8_t PrintHelp(const CLS1_StdIOType *io) {
	CLS1_SendHelpStr((unsigned char*) "pollen",
			(unsigned char*) "Group of pollen commands\r\n", io->stdOut);
	CLS1_SendHelpStr((unsigned char*) "Mode <modeNr> <pollenName>",
			(unsigned char*) "w�hle eine Polle und der entsprechende Modus (1 bis 3)\r\n",
			io->stdOut);

	return ERR_OK;
}

uint8_t SetMode(int32_t mode, char* polle, const CLS1_StdIOType *io) {
#if 0
	uint8_t result = ERR_OK;
	QUEUE_RESULT res = QUEUE_OK;
	Message_t *pxMessage;
	pxMessage = &xMessage;


	uint8_t daten[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 4,
			5, 6, 5, 1, 2, 3, 4, 5, 6, 5, 4, 5, 6, 5, 4, 5, 6, 5, 4, 5, 1, 2, 3,
			4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 4, 5, 6, 5, 1, 2, 3, 4,
			5, 6, 5, 4, 5, 6, 5, 4, 5, 6, 5, 4, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1,
			2, 3, 4, 5, 6, 7, 8, 9, 4, 5, 6, 5, 1, 2, 3, 4, 5, 6, 5, 4, 5, 6, 5,
			4, 5, 6, 5, 4, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8,
			9, 4, 5, 6, 5, 1, 2, 3, 4, 5, 6, 5, 4, 5, 6, 5, 4, 5, 6, 5, 4, 5, 1,
			2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 4, 5, 6, 5, 1, 2,
			3, 4, 5, 6, 5, 4, 5, 6, 5, 4, 5, 6, 5, 4, 5 };
	pxMessage->data = daten;

	uint8_t val = 0;
	//res = AddMessageToQueue(queue_handler, pxMessage);
	if (res != QUEUE_OK) {
		return result = ERR_BUSY;
	}

	return result;
#endif

}

uint8_t playPolle(char * polle, const CLS1_StdIOType *io) {

	uint8_t result = ERR_OK;

#if 0

	BMPImage* image;
	char * temp_filename;
	Message_t *pxMessage;
	pxMessage = &xMessage;





	char cd [4] = {"\\.."};
	char * cd_back = cd;


	uint8_t excitation = 1; /*needs to be specified */

#if 0
	if(directory_set){
		result = FAT1_ChangeDirectory(cd_back, io);		// go back in the directory path
	}
	result = FAT1_ChangeDirectory(polle, io);
#endif
	if (result != ERR_OK) {
		CLS1_SendStr(
				(unsigned char*) "Polle nicht gefunden, Namen �berpr�fen \r\n ",
				CLS1_GetStdio()->stdOut);
	} else {

		directory_set = TRUE;
		//image = loadBMPData(polle, io);
	//	NEOA_Display_Image(image);
	//	pxMessage->modus = ALL;		// alle 3 modis werden abgespielt
	//	pxMessage->data = image->data;


	//	readCharacteristicValues(polle, pxMessage, excitation);

	//	result = AddMessageToQueue(queue_handler, pxMessage);
		if (result != QUEUE_OK) {
			result = ERR_BUSY;
		}








	}


#endif
	return result;

}



uint8_t POLLEN_ParseCommand(const unsigned char* cmd, bool *handled,
		const CLS1_StdIOType *io) {
	uint8_t res = ERR_OK;
	int32_t tmp, mode;
	const uint8_t *p;
	char* polle = NULL;

	if (UTIL1_strcmp((char*)cmd, CLS1_CMD_HELP) == 0
			|| UTIL1_strcmp((char*)cmd, "pollen help") == 0) {
		*handled = TRUE;
		return PrintHelp(io);
		/*} else if ((UTIL1_strcmp((char*)cmd, CLS1_CMD_STATUS) == 0)
		 || (UTIL1_strcmp((char*)cmd, "pollen status") == 0)) {
		 *handled = TRUE;
		 res = PrintStatus(io);
		 */
	} else if (UTIL1_strncmp((char*) cmd, "pollen Mode",
			sizeof("pollen Mode") - 1) == 0) {
		if (FRTOS1_xSemaphoreTake(mutex,0) == pdTRUE) {
			p = cmd + sizeof("pollen Mode") - 1;
			res = UTIL1_xatoi(&p, &mode); /* read Mode */
			if (res == ERR_OK && mode >= 1 && mode <= 5) {
				polle = FRTOS1_pvPortMalloc(sizeof(char*));
				if (polle != NULL) {
					strcpy(polle, p);
					if (polle != NULL) {
						*handled = TRUE;
						res = SetMode(mode, polle, io);
						FRTOS1_vPortFree(polle);
					}
				} else {
					/*something went wrong*/
					res = ERR_FAULT;
				}
			}
			if (FRTOS1_xSemaphoreGive(mutex) != pdTRUE) {
				/*Mutex konnte nicht zur�ckgegeben werden*/
			}
		} else {

			CLS1_SendStr((unsigned char*) "Anwendung l�uft bereits \r\n ",
					CLS1_GetStdio()->stdOut);
		}

	} else if (UTIL1_strncmp((char*) cmd, "play ",
			sizeof("play ") - 1) == 0) {

		if (FRTOS1_xSemaphoreTake(mutex,0) == pdTRUE) {
			p = cmd + sizeof("play ") - 1;
			polle = FRTOS1_pvPortMalloc(sizeof(char*));
			if (polle != NULL) {
				strcpy(polle, p);
				if (polle != NULL) {
					*handled = TRUE;
					res = playPolle(polle, io);
					FRTOS1_vPortFree(polle);
				}
			} else {
				/*something went wrong*/
				res = ERR_FAULT;
			}

			if (FRTOS1_xSemaphoreGive(mutex) != pdTRUE) {
				/*Mutex konnte nicht zur�ckgegeben werden*/
			}
		} else {

			CLS1_SendStr((unsigned char*) "Anwendung l�uft bereits \r\n ",
					CLS1_GetStdio()->stdOut);
		}

	} else if (UTIL1_strncmp((char*) cmd, "pollen calibrating",
			sizeof("pollen calibrating") - 1) == 0) {
		res = SetMode(5, NULL, io); /*Mode 5 is Calibration mode*/
	} else if (UTIL1_strncmp((char*) cmd, "pollen pause",
			sizeof("pollen pause") - 1) == 0) {
		//res = SetNav(pause);
	} else if (UTIL1_strncmp((char*) cmd, "pollen play",
			sizeof("pollen play") - 1) == 0) {
		//res = SetNav(play);
	} else if (UTIL1_strncmp((char*) cmd, "pollen stop",
			sizeof("pollen stop") - 1) == 0) {
		//res = SetNav(stop);
	}

	*handled = TRUE;
	return res;
}
