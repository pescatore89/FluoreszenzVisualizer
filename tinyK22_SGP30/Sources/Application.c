/*
 * Application.c
 *
 *  Created on: 30.09.2018
 *      Author: Erich Styger
 */

#include "Platform.h"
#include "CLS1.h"
//#include "FRTOS1.h"
#include "LED1.h"
#include "TRG1.h"
#include "TMOUT1.h"
#include "Shell.h"
#include "Application.h"
#include "Sensor.h"
#include "TmDt1.h"
#include  "Message.h"
#include "config.h"
#include "Player.h"
#if PL_CONFIG_HAS_NEO_PIXEL
#include "WS2812B/NeoApp.h"
#endif
#if PL_CONFIG_HAS_SPI
#include "SPI.h"
#endif
#if PL_CONFIG_HAS_GUI
#include "gui/lvgl/lvgl.h"
#include "gui/gui.h"
#endif

#define APP_PERIODIC_TIMER_PERIOD_MS   10
#if TmDt1_TICK_TIME_MS!=APP_PERIODIC_TIMER_PERIOD_MS
#error "Software RTC tick time has to match timer time"
#endif
#if TRG1_CONFIG_TICK_PERIOD_MS!=APP_PERIODIC_TIMER_PERIOD_MS
#error "Trigger tick time has to match timer time"
#endif
#if TMOUT1_TICK_PERIOD_MS!=APP_PERIODIC_TIMER_PERIOD_MS
#error "Timeout tick time has to match timer time"
#endif
static xTimerHandle timerHndl;
xQueueHandle queue_handler; /*QueueHandler declared in Message.h*/
xQueueHandle queue_handler_Navigation; /*QueueHandler declared in Message.h*/

xQueueHandle queue_handler_playlist;
xQueueHandle queue_handler_data;
xQueueHandle queue_handler_update;

xSemaphoreHandle mutex; /*SemaphoreHandler declared in Message.h*/
static void vTimerCallbackExpired(xTimerHandle pxTimer) {
#if PL_CONFIG_HAS_GUI
	lv_tick_inc(APP_PERIODIC_TIMER_PERIOD_MS);
#endif
	TRG1_AddTick();
	TMOUT1_AddTick();
	TmDt1_AddTick();
}

static void AppTask(void *pv) {
	(void) pv;

	vTaskDelay(pdMS_TO_TICKS(1000));
	initConfigData();
#if PL_CONFIG_HAS_GUI
	GUI_Init();
#endif

	for (;;) {
	//		LED1_Neg();
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

uint8_t createQueues(void) {

	uint8_t res = ERR_OK;

	/*Initialize Playlist Queue*/
	queue_handler_playlist = FRTOS1_xQueueCreate(QUEUE_PLAYLIST_LENGTH,
			sizeof(struct PlaylistMessage*));

	if (queue_handler_playlist == NULL) {
		return ERR_FAILED;
	} else {
		vQueueAddToRegistry(queue_handler_playlist, "Playlist Queue");
	}


	/*Initialize Data Queue*/
	queue_handler_data = FRTOS1_xQueueCreate(QUEUE_DATA_LENGTH,
			sizeof(struct DataMessage_t*));

	if (queue_handler_data == NULL) {
		return ERR_FAILED;
	} else {
		vQueueAddToRegistry(queue_handler_data, "Data Queue");
	}


	/*Initialize Update Queue*/
	queue_handler_update = FRTOS1_xQueueCreate(QUEUE_UPDATE_LENGTH,
			sizeof(struct UpdateMessage_t*));

	if (queue_handler_update == NULL) {
		return ERR_FAILED;
	} else {
		vQueueAddToRegistry(queue_handler_update, "Update Queue");
	}



	return res;
}

void APP_Run(void) {



	uint8_t res = createQueues();
	if(res != ERR_OK){
		/*error initializing queues*/
	}
	mutex = FRTOS1_xSemaphoreCreateMutex();
	if (mutex == NULL) {
		/*Something went wrong*/
	}

#if PL_CONFIG_HAS_NEO_PIXEL
	NEOA_Init(queue_handler); /*Dem NeoTask den QueueHandler mitgeben*/
#endif

	SHELL_Init();

#if PL_CONFIG_HAS_SPI
	SPI_Init();
#endif

	SENSOR_Init();

	PLAYER_Init();

	if (xTaskCreate(AppTask, /* pointer to the task */
	"App", /* task name for kernel awareness debugging */
	2500 / sizeof(StackType_t), /* task stack size */
	(void*) NULL, /* optional task startup argument */
	tskIDLE_PRIORITY + 1, /* initial priority */
	(xTaskHandle*) NULL /* optional task handle to create */
	) != pdPASS) {
		/*lint -e527 */
		for (;;) {
		}; /* error! probably out of memory */
		/*lint +e527 */
	}
	timerHndl = xTimerCreate("timer", /* name */
	pdMS_TO_TICKS(APP_PERIODIC_TIMER_PERIOD_MS), /* period/time */
	pdTRUE, /* auto reload */
	(void*) 0, /* timer ID */
	vTimerCallbackExpired); /* callback */
	if (timerHndl == NULL) {
		for (;;)
			; /* failure! */
	}
	if (xTimerStart(timerHndl, 0) != pdPASS) { /* start the timer */
		for (;;)
			; /* failure!?! */
	}

	vTaskStartScheduler();
	for (;;) {
		__asm("nop");
		/* should not get here! */
	}
}
