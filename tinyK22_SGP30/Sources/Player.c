/*
 * Player.c
 *
 *  Created on: 14.12.2018
 *      Author: Pescatore
 */

#include "Player.h"
#include "Message.h"
#include "FRTOS1.h"
#include "readSD.h"
#include "config.h"
#include <stdio.h>      // Header file for standard file i/o.
#include <stdlib.h>     // Header file for malloc/free.
#include <ctype.h>
xQueueHandle queue_handler_playlist; /*QueueHandler declared in Message.h*/
xQueueHandle queue_handler_data; /*Queue handler for data Queue*/

static uint8_t listEnabled[100];
static uint8_t nPollenEnabled;
static uint8_t nameCNT;

typedef enum {
	IDLE = 0, /* */
	READ_NEW, /* read new Message from playlistQueue  */
	UPDATE_PLAYLIST, /*  */
	PLAY_LIST, /* */

} PLAYER_STATE;

static uint8_t updateListEnabled(uint8_t* playlist) {
	int counter = 0;
	int nOfPollen = getQuantity();
	int i = 0;
	for (i = 0; i < nOfPollen; i++) {
		if (playlist[i] != 0) {
			listEnabled[counter] = i;
			counter++;
		}
	}
	return counter;

}

static char* getName() {

	char** pollenNamelist;

	if (nameCNT >= nPollenEnabled) {
		nameCNT = 0;
	}

	pollenNamelist = getNamelist();

	return pollenNamelist[listEnabled[nameCNT++]];

	//strcpy(polle, pollenNamelist);

}

static uint8_t getMemory(DataMessage_t* px) {

	uint8_t res = 0x01;
	px->color_data = FRTOS1_pvPortMalloc(sizeof(char) * 2500); /*MAGIC NUMBER*/
	if ((px->color_data) == NULL) {

		return 0x00;
		/*malloc failed*/
	} else {
#if 0
		px->char_data = FRTOS1_pvPortMalloc(sizeof(DATA_t*));
		if ((px->char_data) == NULL) {

			return 0x00;
			/*malloc failed*/
		}
#endif
	}
	return res;
}



static void freeMemory(DataMessage_t* px){


//	FRTOS1_vPortFree(px->char_data);
	FRTOS1_vPortFree(px->color_data);



}

static void PlayerTask(void *pvParameters) {
	PlaylistMessage_t *pxPlaylistMessage;
	pxPlaylistMessage = &xPlaylistMessage;

	DataMessage_t * pxDataMessage;
	pxDataMessage = &xDataMessage;

//int nOfPollen = getQuantity();

	int nOfPollen = getQuantity();

	char** playList = NULL;

	PLAYER_STATE state = IDLE;

	uint8_t res;

	int k = 12;
	for (;;) {

		switch (state) {

		case IDLE:

			if (TakeMessageFromPlaylistQueue(queue_handler_playlist,
					pxPlaylistMessage) != QUEUE_EMPTY) {
				state = READ_NEW;
				break;
			} else {
				vTaskDelay(pdMS_TO_TICKS(100)); /*Queue is Empty*/
			}
			break;

		case READ_NEW:
			if ((pxPlaylistMessage->state) == newCMD) {
				/*Do something*/
			} else if ((pxPlaylistMessage->state) == newData) {
				state = UPDATE_PLAYLIST;
				break;
			} else if ((pxPlaylistMessage->state) == newImage) {
				/*Do something*/
			}

			break;

		case UPDATE_PLAYLIST:

			nPollenEnabled = updateListEnabled(pxPlaylistMessage->playlist);
			/*Update the playlist*/
			nameCNT = 0;
			state = PLAY_LIST;
			break;

		case PLAY_LIST:
			if (TakeMessageFromPlaylistQueue(queue_handler_playlist,
					pxPlaylistMessage) != QUEUE_EMPTY) {
				state = READ_NEW; /*new Element in the Playlist Queue*/
				break;
			}

			else {

				if (PeekDataQueue(queue_handler_data, pxDataMessage)
						!= QUEUE_EMPTY) {
					vTaskDelay(pdMS_TO_TICKS(10)); /*LED Task is busy playing*/
				} else {
					/*send new Data to DataQueue*/
					//	pxDataMessage->name = (getNamesActive(
					//			pxPlaylistMessage->playlist, playList));
					if (!getMemory(pxDataMessage)) {
						/*problem allocating memory*/
					}
					pxDataMessage->name = getName();
					res = readDataFromSD(pxDataMessage);

					if (AddMessageToDataQueue(queue_handler_data, pxDataMessage)
							!= QUEUE_OK) {
						/*Queue is full*/

					}
					freeMemory(pxDataMessage);

				}
			}
			break;
		}
	}
}
void PLAYER_Init(void) {
//	CLS1_SetStdio(ios[0].stdio); /* using the first one as the default channel */
	if (xTaskCreate(PlayerTask, "Player", 6000 / sizeof(StackType_t),
	NULL,
	tskIDLE_PRIORITY + 2, NULL) != pdPASS) {
		for (;;) {
		} /* error */
	}
}

