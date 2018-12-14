/*
 * Player.c
 *
 *  Created on: 14.12.2018
 *      Author: Pescatore
 */

#include "Player.h"
#include "Message.h"
#include "FRTOS1.h"
xQueueHandle queue_handler_Navigation; /*QueueHandler declared in Message.h*/

static void PlayerTask(void *pvParameters) {
	Navigation_t* rxNav;
	rxNav = &xNavigation;

	for (;;) {
		if (TakeNavigationFromQueue(queue_handler_Navigation, rxNav)
				!= QUEUE_EMPTY) {
			switch (rxNav->menu) {
			case pause:
				break;
			case play:
				break;

			}

		}
		else{
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

