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
#include "LCD1.h"
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
#include "FDisp1.h"
#include "GDisp1.h"
#include "GFont1.h"
#include "FRTOS1.h"
#include "gui/Images.h"

#endif

#define APP_PERIODIC_TIMER_PERIOD_MS   10
#define LCD_PERIODIC_TIMER_MS	100
#if TmDt1_TICK_TIME_MS!=APP_PERIODIC_TIMER_PERIOD_MS
#error "Software RTC tick time has to match timer time"
#endif
#if TRG1_CONFIG_TICK_PERIOD_MS!=APP_PERIODIC_TIMER_PERIOD_MS
#error "Trigger tick time has to match timer time"
#endif
#if TMOUT1_TICK_PERIOD_MS!=APP_PERIODIC_TIMER_PERIOD_MS
#error "Timeout tick time has to match timer time"
#endif
static xTimerHandle timerHndl_LCD;
static xTimerHandle timerHndl;
static int counter_LCD;
static int counter_ScreensaverPlaylist;
static bool display_state = TRUE;
static bool LCD_Init = TRUE;

static uint8_t ScreensaverExpiredState = FALSE;
xQueueHandle queue_handler; /*QueueHandler declared in Message.h*/
xQueueHandle queue_handler_Navigation; /*QueueHandler declared in Message.h*/

xQueueHandle queue_handler_playlist;
xQueueHandle queue_handler_data;
xQueueHandle queue_handler_update;

xSemaphoreHandle mutex; /*SemaphoreHandler declared in Message.h*/

typedef enum {
	INIT = 0, ON, OFF, RESET,
} LCD_state;
static LCD_state state = INIT;

static void vTimerCallbackExpired(xTimerHandle pxTimer) {
#if PL_CONFIG_HAS_GUI
	lv_tick_inc(APP_PERIODIC_TIMER_PERIOD_MS);
#endif
	TRG1_AddTick();
	TMOUT1_AddTick();
	TmDt1_AddTick();
}

uint8_t getDisplayState(void) {

	uint8_t temp = 1;
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	temp = display_state;
	CS1_ExitCritical()
	;
	return temp;
}



uint8_t isScreensaverTimeExpired (void){

	uint8_t temp = ERR_BUSY;
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	temp = ScreensaverExpiredState;
	CS1_ExitCritical()
	;

	return temp;
}



void setScreensaverTimeExpired (uint8_t state){

	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	ScreensaverExpiredState = state;
	CS1_ExitCritical()
	;
}

static void setDisplayState(uint8_t state) {

	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	display_state = state;
	CS1_ExitCritical()
	;

}

static bool LCDisInit() {

	bool temp = false;
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	temp = LCD_Init;
	CS1_ExitCritical()
	;
	return temp;
}

static void setLCDinitState(bool isInit) {
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	LCD_Init = isInit;
	CS1_ExitCritical()
	;

}

static bool decrementLCD_CNT() {
	bool res = TRUE;	//not reached 0 yet

	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	counter_LCD--;
	res = counter_LCD;
	CS1_ExitCritical()
	;
	return res;

}

static bool decrementScreensaver_CNT() {
	bool res = TRUE;	//not reached 0 yet

	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	counter_ScreensaverPlaylist--;
	res = counter_ScreensaverPlaylist;
	CS1_ExitCritical()
	;
	return res;

}

static void setTimerLCD(void) {

	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	counter_LCD = getLCDTurnOffTime() / LCD_PERIODIC_TIMER_MS;
	//counter_LCD = 12000 / LCD_PERIODIC_TIMER_MS;
	CS1_ExitCritical()
	;

}

static void setTimerScreensaverPlaylist(void) {
	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	counter_ScreensaverPlaylist = getScreensaverTime() / LCD_PERIODIC_TIMER_MS;
	//counter_ScreensaverPlaylist = 12000 / LCD_PERIODIC_TIMER_MS;
	CS1_ExitCritical()
	;

}

static void vTimerCallbackExpired_LCD(xTimerHandle pxTimer) {

	int tempCnt = 0;

	switch (state) {
	case INIT:
		if (!LCDisInit()) {
			setTimerLCD();
			setTimerScreensaverPlaylist();
			state = ON;
		}
		break;
	case ON:


		/*PLAYING MODE*/
		if (!isStateIdle() & getDisplayState()) {//man befindet sich im Playmodus und Display ist noch nicht ausgeschalten
			if (!decrementScreensaver_CNT()) {// True falls timer 0 erreicht hat
				// GUI Task wird benachrichtigt, dass er den Bildschirm abstellen soll
				TaskHandle_t xTaskToNotify = NULL;
				xTaskToNotify = xTaskGetHandle("Gui");
				//Notify GUI Task to turn off Display
				FRTOS1_xTaskNotifyGive(xTaskToNotify);
				setDisplayState(FALSE);
				setScreensaverTimeExpired(TRUE);
			}
		}


		/*IDLE MODE*/
		else if (getDisplayState()) {		//true if LCD is turned on
			if (!decrementLCD_CNT()) {// decrement counter, true if has reached 0
				if (isStateIdle() & !getIsDisplayingImage()) {//überprüfen ob im Idle Mode oder ob die Playlist läuft und deshalb kein Button mehr gedrückt wurde
						// GUI Task wird benachrichtigt, dass er in den Screensaver mode gehen soll
					TaskHandle_t xTaskToNotify = NULL;
					xTaskToNotify = xTaskGetHandle("Gui");
					//Notify GUI Task to turn off Display
					FRTOS1_xTaskNotifyGive(xTaskToNotify);
					//Notify NeoPixel task to run screensaver mode
					xTaskToNotify = xTaskGetHandle("Neo");
					FRTOS1_xTaskNotifyGive(xTaskToNotify);
					setDisplayState(FALSE);
				} else {
					resetLCD_Counter();
				}
			}
		} else {
			// do nothing
		}

		break;

	}
}

void resetScreensaver_Counter(void) {
	LCD1_DisplayOnOff(TRUE);	// switch on Display
	setTimerScreensaverPlaylist();
	setScreensaverTimeExpired(FALSE);
	setDisplayState(TRUE);

}

void resetLCD_Counter(void) {

	LCD1_DisplayOnOff(TRUE);	// switch on Display
	setTimerLCD();

	setDisplayState(TRUE);

}

static void AppTask(void *pv) {
	(void) pv;
	vTaskDelay(pdMS_TO_TICKS(500));
	initConfigData();
#if PL_CONFIG_HAS_GUI
	GUI_Init();
#endif
	setLCDinitState(FALSE);		//LCD ready;
	bool val = TRUE;

	for (;;) {
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
	if (res != ERR_OK) {
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

	/*Timer 1*/
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

	/*Timer 2, Turning off LCD*/

	timerHndl_LCD = xTimerCreate("timer_LCD", /* name */
	pdMS_TO_TICKS(LCD_PERIODIC_TIMER_MS), /* period/time */
	pdTRUE, /* auto reload */
	(void*) 0, /* timer ID */
	vTimerCallbackExpired_LCD); /* callback */
	if (timerHndl == NULL) {
		for (;;)
			; /* failure! */
	}
	if (xTimerStart(timerHndl_LCD, 0) != pdPASS) { /* start the timer */
		for (;;)
			; /* failure!?! */
	}

	vTaskStartScheduler();
	for (;;) {
		__asm("nop");
		/* should not get here! */
	}
}
