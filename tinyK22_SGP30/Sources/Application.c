/*
 * Application.c
 *
 *  Created on: 30.09.2018
 *      Author: Erich Styger
 */

#include "Platform.h"
#include "CLS1.h"
#include "FRTOS1.h"
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
	//	LED1_Neg();
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

void APP_Run(void) {

	mutex = FRTOS1_xSemaphoreCreateMutex();
	if (mutex == NULL) {
		/*Something went wrong*/
	}
	queue_handler_Navigation = FRTOS1_xQueueCreate(QUEUE_LENGTH,
			sizeof(struct MENU *));
	if (queue_handler_Navigation == NULL) {
		/*Something went wrong*/
	} else {
		vQueueAddToRegistry(queue_handler_Navigation, "Navigation Queue");
	}
	queue_handler = FRTOS1_xQueueCreate(QUEUE_LENGTH, sizeof(struct MESSAGE *));/*Queue erstellen*/
	if (queue_handler == NULL) {
		/*Something went wrong*/
	}

	else {
		vQueueAddToRegistry(queue_handler, "Message Queue");

#if PL_CONFIG_HAS_NEO_PIXEL
		NEOA_Init(queue_handler); /*Dem NeoTask den QueueHandler mitgeben*/
#endif
	}
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
