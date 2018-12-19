/*
 * Player.c
 *
 *  Created on: 14.12.2018
 *      Author: Pescatore
 */

#include "Player.h"
#include "Message.h"
#include "FRTOS1.h"
xQueueHandle queue_handler_playlist; /*QueueHandler declared in Message.h*/

static void PlayerTask(void *pvParameters) {
	PlaylistMessage_t *pxPlaylistMessage;
	pxPlaylistMessage = &xPlaylistMessage;

	for (;;) {
		if (TakeMessageFromPlaylistQueue(queue_handler_playlist,
				pxPlaylistMessage) != QUEUE_EMPTY) {

			switch (pxPlaylistMessage->state) {
			case newCMD:
				switch (pxPlaylistMessage->cmd) {
				case pause:
					break;
				case play:
					break;

				case stop:
					break;

				case skipF:
					break;

				case skipR:
					break;

				}
				break;

			case newImage:
				break;

			case newData:
				break;
			}

		} else {
			vTaskDelay(pdMS_TO_TICKS(100)); /*Queue is Empty*/
		}
	}
}

void PLAYER_Init(void) {
//	CLS1_SetStdio(ios[0].stdio); /* using the first one as the default channel */
	if (xTaskCreate(PlayerTask, "Player", 6000 / sizeof(StackType_t), NULL,
	tskIDLE_PRIORITY + 2, NULL) != pdPASS) {
		for (;;) {
		} /* error */
	}
}

