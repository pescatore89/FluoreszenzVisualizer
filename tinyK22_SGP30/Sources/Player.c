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
#include "LCD1.h"
#include <stdio.h>      // Header file for standard file i/o.
#include <stdlib.h>     // Header file for malloc/free.
#include <ctype.h>
xQueueHandle queue_handler_playlist; /*QueueHandler declared in Message.h*/
xQueueHandle queue_handler_data; /*Queue handler for data Queue*/
xSemaphoreHandle mutex; /*SemaphoreHandler declared in Message.h*/
static uint8_t listEnabled[100];
static uint8_t nPollenEnabled;
static uint8_t nameCNT;

typedef enum {
	IDLE = 0, /* */
	READ_NEW, /* read new Message from playlistQueue  */
	UPDATE_PLAYLIST, /*  */
	PLAY_LIST, /* */
	DISPLAY_IMAGE, CLEAR_IMAGE,

} PLAYER_STATE;

static uint8_t updateListEnabled(uint8_t* playlist) {
	int counter = 0;
	int nOfPollen = getQuantity();
	int i = 0;

	/*delete current list*/
	for (i = 0; i < nOfPollen; i++) {
		listEnabled[i] = 0;
	}

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

static void freeMemory(DataMessage_t* px) {

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
	uint8_t excitation = 1;
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
				if ((pxPlaylistMessage->cmd) == pause) {
					pxDataMessage->cmd = pause;
					if (AddMessageToDataQueue(queue_handler_data, pxDataMessage)
							!= QUEUE_OK) {
						/*Queue is full*/
					} else {
						state = PLAY_LIST;
					}
				} else if ((pxPlaylistMessage->cmd) == stop) {
					pxDataMessage->cmd = stop;
					if (AddMessageToDataQueue(queue_handler_data, pxDataMessage)
							!= QUEUE_OK) {
						/*Queue is full*/
					} else {
						excitation = 1;
						state = IDLE;
					}

				} else if ((pxPlaylistMessage->cmd) == play) { /*the play CMD after an pause occured*/
					pxDataMessage->cmd = play;
					if (AddMessageToDataQueue(queue_handler_data, pxDataMessage)
							!= QUEUE_OK) {
						/*Queue is full*/
					} else {
						state = PLAY_LIST;
					}
				} else if ((pxPlaylistMessage->cmd) == skipF) {
					pxDataMessage->cmd = skipF;
					if (AddMessageToDataQueue(queue_handler_data, pxDataMessage)
							!= QUEUE_OK) {
						/*Queue is full*/
					} else {
						/*
						 * Hier definieren Was zu tun ist wenn ein Skip Forward ausgelöst wurde TODO
						 * */
						state = PLAY_LIST;
					}

				} else if ((pxPlaylistMessage->cmd) == skipR) {
					pxDataMessage->cmd = skipR;
					if (AddMessageToDataQueue(queue_handler_data, pxDataMessage)
							!= QUEUE_OK) {
						/*Queue is full*/
					} else {
						/*
						 *
						 * Hier definieren Was zu tun ist wenn ein Skip Reverse ausgelöst wurde TODO
						 * */
						state = PLAY_LIST;
					}

				}

				/*Do something*/
			} else if ((pxPlaylistMessage->state) == newData) {
				pxDataMessage->cmd = play;
				state = UPDATE_PLAYLIST;
			} else if ((pxPlaylistMessage->state) == newImage) {
				state = DISPLAY_IMAGE;
			} else if ((pxPlaylistMessage->state) == clrImage) {
				state = CLEAR_IMAGE;
			}

			break;

		case DISPLAY_IMAGE:

			excitation = 1;

			if (!getMemory(pxDataMessage)) {
				/*problem allocating memory*/
			} else {

				pxDataMessage->name = pxPlaylistMessage->imageName;
				res = readImageFromSD(pxDataMessage);

				pxDataMessage->cmd = playImage;

				if (AddMessageToDataQueue(queue_handler_data, pxDataMessage)
						!= QUEUE_OK) {
					/*Queue is full*/
				}

				freeMemory(pxDataMessage);

			}
			state = IDLE;

			break;

		case CLEAR_IMAGE:

			excitation = 1;

			pxDataMessage->cmd = clearImage;

			if (AddMessageToDataQueue(queue_handler_data, pxDataMessage)
					!= QUEUE_OK) {
				/*Queue is full*/
			}

			state = IDLE;

			break;

		case UPDATE_PLAYLIST:

			nPollenEnabled = updateListEnabled(pxPlaylistMessage->playlist);
			if (nPollenEnabled == 0) {
				state = IDLE;
			} else {
				/*Update the playlist*/

				state = PLAY_LIST;

			}
			nameCNT = 0;
			excitation = 1;
			break;

		case PLAY_LIST:

			if (!getMemory(pxDataMessage)) {
				/*problem allocating memory*/
			} else {

				for (;;) {

					if (TakeMessageFromPlaylistQueue(queue_handler_playlist,
							pxPlaylistMessage) != QUEUE_EMPTY) {
						state = READ_NEW; /*new Element in the Playlist Queue*/
						break;
					}

					else {

						if ((PeekDataQueue(queue_handler_data, pxDataMessage)
								!= QUEUE_EMPTY)) { /*Has Element in the Data Queue, Neo Task hasnt taken it out yet*/
							vTaskDelay(pdMS_TO_TICKS(10));

						} else {
							/*make sure the NEO Task is waiting for a new Element */
							if ((FRTOS1_xSemaphoreTake(mutex,0) != pdTRUE)) {
								vTaskDelay(pdMS_TO_TICKS(10)); /*NEO Task is busy playing*/
								continue;
								//break;
							} else {

								if (FRTOS1_xSemaphoreGive(mutex) != pdTRUE) {
									/*error giving back the semaphore*/
								} else {

									if (excitation == 1) {
										pxDataMessage->name = getName();
									}

									res = readDataFromSD(excitation,
											pxDataMessage);
									pxDataMessage->excitation = excitation;

									if (AddMessageToDataQueue(
											queue_handler_data, pxDataMessage)
											!= QUEUE_OK) {
										/*Queue is full*/
									}

									excitation++;
									if (excitation == 4) {
										excitation = 1;
									}

								}

							}
						}
					}

				}
			}
			freeMemory(pxDataMessage);
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

