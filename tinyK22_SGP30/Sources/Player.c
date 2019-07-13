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

DataMessage_t *DataPtr = NULL;
DATA_t * charDataPtr = NULL;

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

static char** getNameList() {

	char** pollenNamelist;

	pollenNamelist = getNamelist();

	return pollenNamelist;

	//strcpy(polle, pollenNamelist);

}

static uint8_t getMemory(void) {

	uint8_t res = 0x01;

	//Config_StorePollen(px);


	uint8_t quantity = getQuantity();

	//get Memory for Image Data********************************************************

	DataPtr = FRTOS1_pvPortMalloc(sizeof(DataMessage_t) * quantity * 3);

	if (DataPtr == NULL) {
		/*problem allocating memory*/
		return ERR_FAILED;
	}

	for (int i = 0; i < (quantity * 3); i++) {
		(DataPtr + i)->color_data = FRTOS1_pvPortMalloc(sizeof(char) * 2000);
		if (((DataPtr + i)->color_data) == NULL) {
			res = ERR_FAILED;
			/*malloc failed*/
		}

	}

	// get Memory for characteristic Value Data*****************************************


	charDataPtr = FRTOS1_pvPortMalloc(sizeof(DATA_t) * quantity);


	if (charDataPtr == NULL) {
		/*problem allocating memory*/
		return ERR_FAILED;
	}

#if 0
	for (int i = 0; i < (quantity * 3); i++) {
		(charDataPtr + i)->color_data = FRTOS1_pvPortMalloc(sizeof(char) * 2000);
		if (((DataPtr + i)->color_data) == NULL) {
			res = ERR_FAILED;
			/*malloc failed*/
		}

	}

#endif



//	px = FRTOS1_pvPortMalloc(sizeof(DataMessage_t) * getQuantity());

#if 0

	px = FRTOS1_pvPortMalloc(sizeof(DataMessage_t) * getQuantity());
	for (int i = 0; i < 2; i++) {
		(px + i)->color_data = FRTOS1_pvPortMalloc(sizeof(char) * 2000);
		if (((px + i)->color_data) == NULL) {
			res = ERR_FAILED;
			/*malloc failed*/
		}

	}

#endif

#if 0

	//px = FRTOS1_pvPortMalloc(sizeof(DataMessage_t));
	//if(px == NULL){
	/*problem allocating memory*/
	//	return ERR_FAILED;
//	}
	(px + 0)->color_data = FRTOS1_pvPortMalloc(sizeof(char) * 2500); /*MAGIC NUMBER*/
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

#endif
	return res;
}

static void freeMemory(DataMessage_t* px) {

//	FRTOS1_vPortFree(px->char_data);
	FRTOS1_vPortFree(px->color_data);

}

static uint8_t loadData(void) {

	char ** nameList = getNameList();

	uint8_t excitation = 1;
	uint8_t nameCnt = 0;

	uint8_t res = ERR_OK;

	uint8_t quantity = getQuantity();


	/* load Data for Images*/
	for (int i = 1; i <= (quantity * 3); i++) {
		// now, load the Data
		((DataPtr + i - 1))->name = nameList[nameCnt];
		if (!(i % 3)) {
			res = readDataFromSD(excitation, ((DataPtr + i - 1)),(charDataPtr+ nameCnt));
			excitation = 1;
			nameCnt++;
		} else {
			res = readDataFromSD(excitation, ((DataPtr + i - 1)),(charDataPtr+ nameCnt));
			excitation++;
		}
		if (res != ERR_OK) {
			break;
		}
	}




	return res;
}



static uint8_t getPosition(char* name){


	char** nameList = getNameList();


	uint8_t cnt;
	uint8_t quantity = getQuantity();

	for(cnt = 1; cnt <=quantity; cnt++){
		if(!(strcmp(*(nameList+cnt -1),name))){
			return cnt;
		}
	}

	return cnt;

}

static void PlayerTask(void *pvParameters) {

	uint8_t res;
	PlaylistMessage_t *pxPlaylistMessage;
	pxPlaylistMessage = &xPlaylistMessage;

	DataMessage_t * pxDataMessage;
	pxDataMessage = &xDataMessage;

	vTaskDelay(pdMS_TO_TICKS(2000));		// wait until all setup

	getMemory();

	res = loadData();

//int nOfPollen = getQuantity();

	int nOfPollen = getQuantity();

	char** playList = NULL;

	bool wasSkippedForward = FALSE;
	bool wasSkippedBackward = FALSE;
	bool playAgainFlag = FALSE;

	PLAYER_STATE state = IDLE;
	uint8_t excitation = 1;

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
				if ((pxPlaylistMessage->cmd) == stop) {
					pxDataMessage->cmd = stop;
					if (AddMessageToDataQueue(queue_handler_data, pxDataMessage)
							!= QUEUE_OK) {
						/*Queue is full*/
					} else {
						excitation = 1;
						state = IDLE;
					}

				} else if ((pxPlaylistMessage->cmd) == skipR) {
					pxDataMessage->cmd = skipR;
					if (AddMessageToDataQueue(queue_handler_data, pxDataMessage)
							!= QUEUE_OK) {
						/*Queue is full*/
					} else {
						/*
						 * Hier definieren Was zu tun ist wenn ein Skip Reverse ausgelöst wurde TODO
						 * */
						pxDataMessage->cmd = play;
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

			if (!getMemory()) {

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

			//	if (!getMemory(pxDataMessage)) {
			if (0) {
				/*problem allocating memory*/
			} else {

				for (;;) {

					if (TakeMessageFromPlaylistQueue(queue_handler_playlist,
							pxPlaylistMessage) != QUEUE_EMPTY) {

						if (pxPlaylistMessage->cmd == playAgain) {
							pxDataMessage->cmd = playAgain;

							if (AddMessageToDataQueue(queue_handler_data,
									pxDataMessage) != QUEUE_OK) {
								/*Queue is full*/
							}
						}

						else if ((pxPlaylistMessage->cmd) == pause) {
							pxDataMessage->cmd = pause;
							if (AddMessageToDataQueue(queue_handler_data,
									pxDataMessage) != QUEUE_OK) {
								/*Queue is full*/
							}
						}

						else if ((pxPlaylistMessage->cmd) == play) { /*the play CMD after an pause occured*/
							(pxDataMessage)->cmd = play;
							if (AddMessageToDataQueue(queue_handler_data,
									pxDataMessage )!= QUEUE_OK) {
								/*Queue is full*/
							}
						} else if ((pxPlaylistMessage->cmd) == skipF) {
							pxDataMessage->cmd = skipF;
							if (AddMessageToDataQueue(queue_handler_data,
									pxDataMessage) != QUEUE_OK) {
								/*Queue is full*/
							} else {
								/*
								 * Hier definieren Was zu tun ist wenn ein Skip Forward ausgelöst wurde TODO
								 * */
								//pxDataMessage->cmd = play;
								wasSkippedForward = TRUE;
							}

						} else if ((pxPlaylistMessage->cmd) == skipR) {
							pxDataMessage->cmd = skipR;
							if (AddMessageToDataQueue(queue_handler_data,
									pxDataMessage) != QUEUE_OK) {
								/*Queue is full*/
							} else {
								/*
								 * Hier definieren Was zu tun ist wenn ein Skip Forward ausgelöst wurde TODO
								 * */
								wasSkippedBackward = TRUE;
								//excitation = 1;
								//nameCNT--;
							}
						} else {
							state = READ_NEW; /*new Element in the Playlist Queue*/
							break;
						}
					}

					else {

						if ((PeekDataQueue(queue_handler_data, (pxDataMessage))
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

							//		res = readDataFromSD(excitation,
							//				((DataPtr + 1)));
									pxDataMessage->excitation = excitation;
									pxDataMessage->position = getPosition(pxDataMessage->name);

									if (wasSkippedForward || playAgainFlag) {
										(pxDataMessage)->cmd = play; // überschreibt das skipF Kommando und stellt so sicher dass die nächste Excitation starten kann
										wasSkippedForward = FALSE; // Reset Skip Forward Flag
										playAgainFlag = FALSE;
									}

									if (AddMessageToDataQueue(
											queue_handler_data, (pxDataMessage))
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
			break;
		}
	}
}

void PLAYER_Init(void) {
//	CLS1_SetStdio(ios[0].stdio); /* using the first one as the default channel */
	if (xTaskCreate(PlayerTask, "Player", 3000 / sizeof(StackType_t),
	NULL,
	tskIDLE_PRIORITY + 2, NULL) != pdPASS) {
		for (;;) {
		} /* error */
	}
}

