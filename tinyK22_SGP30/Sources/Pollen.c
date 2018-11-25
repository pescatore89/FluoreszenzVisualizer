/*
 * Pollen.c
 *
 *  Created on: 25.11.2018
 *      Author: Pescatore
 */

#include "Pollen.h"
#include "Message.h"

xQueueHandle queue_handler;
char* string;

static uint8_t PrintHelp(const CLS1_StdIOType *io) {
	CLS1_SendHelpStr((unsigned char*) "pollen",
			(unsigned char*) "Group of pollen commands\r\n", io->stdOut);
	CLS1_SendHelpStr((unsigned char*) "Mode <modeNr> <pollenName>",
			(unsigned char*) "w�hle eine Polle und der entsprechende Modus (1 bis 3)\r\n",
			io->stdOut);

	return ERR_OK;
}

uint8_t SetMode(int32_t mode, char* polle, const CLS1_StdIOType *io) {

	uint8_t result = ERR_OK;
	QUEUE_RESULT res = QUEUE_OK;
	Message_t *pxMessage;
	pxMessage = &xMessage;

	pxMessage->modus = mode;

	res = AddMessageToQueue(queue_handler, pxMessage);
	if (res != QUEUE_OK) {
		return result = ERR_BUSY;
	}

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
		p = cmd + sizeof("pollen Mode") - 1;
		res = UTIL1_xatoi(&p, &mode); /* read Mode */
		if (res == ERR_OK && mode >= 1 && mode <= 3) {
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
	}
	return res;
}