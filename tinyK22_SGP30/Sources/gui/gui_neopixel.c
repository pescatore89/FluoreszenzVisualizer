/*
 * gui_air.c
 *
 *  Created on: 06.08.2018
 *      Author: Erich Styger
 */

#include "Platform.h"
#if PL_CONFIG_HAS_NEO_PIXEL && PL_CONFIG_HAS_GUI
#include "gui_neopixel.h"
#include "lvgl/lvgl.h"
#include "gui.h"
#include "../WS2812B/Neoapp.h"
#include "gui_pollen.h"
#include <stdio.h>
#include <string.h>
#include "config.h"
static lv_obj_t *win; /* object for window */

#define SLIDER_LEVEL_MIN_RANGE    0
#define SLIDER_LEVEL_MAX_RANGE  100
#define SLIDER_LEVEL_LIGHT_MIN    0
#define SLIDER_LEVEL_LIGHT_MAX  255
static lv_obj_t *label_auto_light_level; /* label for auto mode */
static lv_obj_t *label_slider_level; /* label for level value */
#include "Message.h"

xQueueHandle queue_handler_playlist;

static uint8_t guiIsActive;
static bool isPaused = false;
static lv_obj_t *btn_play;
static lv_obj_t *btn_pause;
static lv_obj_t *btn_stop;
static lv_obj_t *btn_next;
static lv_obj_t *btn_prev;
static lv_obj_t *btn_loop;

static lv_btn_state_t play_buttonState = LV_BTN_STATE_REL;
static lv_btn_state_t pause_buttonState = LV_BTN_STATE_INA;
static lv_btn_state_t stop_buttonState = LV_BTN_STATE_INA;

static lv_obj_t *polle_label;
static char* pollenLabel;

static uint8_t namesAvtiv[MAX_N_POLLS_STORED];
static uint8_t* namesActive;
static void SetLabelValue(lv_obj_t *label, int32_t val) {
	uint8_t buf[16];
	if (label != NULL) {
		UTIL1_Num32sToStr(buf, sizeof(buf), val);
		lv_label_set_text(label, buf);
	}
}


void setNavGuiIsActive(uint8_t val){

	CS1_CriticalVariable();


	CS1_EnterCritical();

	guiIsActive = val;

	CS1_ExitCritical();

}

uint8_t getNavGuiIsActive(void){

	uint8_t res;
	CS1_CriticalVariable();


	CS1_EnterCritical();

		res = guiIsActive;

	CS1_ExitCritical();



	return res;
}



/* Called when a new value id set on the slider */
static lv_res_t slider_action(lv_obj_t *slider) {
	int val;

	val = lv_slider_get_value(slider); /* get current value from slider */
	/* map to target range: */
	val = UTIL1_map(val, SLIDER_LEVEL_MIN_RANGE, SLIDER_LEVEL_MAX_RANGE,
	SLIDER_LEVEL_LIGHT_MIN, SLIDER_LEVEL_LIGHT_MAX);
	NEOA_SetLightLevel(val); /* update model */
	SetLabelValue(label_slider_level, val);
	return LV_RES_OK;
}

static lv_res_t auto_light_level_action(lv_obj_t *sw) {
	bool isOn = lv_sw_get_state(sw);

	NEOA_SetAutoLightLevelSetting(isOn); /* update model */
	if (isOn) {
		lv_label_set_text(label_auto_light_level, "on");
	} else {
		lv_label_set_text(label_auto_light_level, "off");
	}
	return LV_RES_OK;
}

/**
 * Called when the window's close button is clicked
 * @param btn pointer to the close button
 * @return LV_ACTION_RES_INV because the window is deleted in the function
 */
static lv_res_t win_close_action(lv_obj_t *btn) {
	GUI_GroupPull();
	lv_obj_del(win);
	win = NULL;
	setNavGuiIsActive(false);
	return LV_RES_INV;
}

static void setNamesActiv(uint8_t * name) {

	int n = getQuantity();
	for (int i = 0; i < n; i++) {
		namesAvtiv[i] = name[i];
	}

}

static uint8_t* getNamesActiv(void) {
	return namesAvtiv;
}

static lv_res_t btn_play_click_action(lv_obj_t *btn) {

	uint8_t res;
	PlaylistMessage_t *pxPlaylistMessage;
	pxPlaylistMessage = &xPlaylistMessage;

	if(isPaused){
		pxPlaylistMessage->cmd = play;
		pxPlaylistMessage->state = newCMD;
	}
	else{
		pxPlaylistMessage->playlist = namesActive;
		pxPlaylistMessage->cmd = play;
		pxPlaylistMessage->state = newData;
	}

	res = AddMessageToPlaylistQueue(queue_handler_playlist, pxPlaylistMessage);
	lv_btn_set_state(btn, LV_BTN_STATE_INA);
	lv_btn_set_state(btn_pause, LV_BTN_STATE_REL);
	lv_btn_set_state(btn_stop, LV_BTN_STATE_REL);
	play_buttonState = LV_BTN_STATE_INA;
	pause_buttonState = LV_BTN_STATE_REL;
	stop_buttonState = LV_BTN_STATE_REL;


	return LV_RES_OK; /* Return OK if the button is not deleted */
}

static lv_res_t btn_pause_click_action(lv_obj_t *btn) {

	uint8_t res;
	PlaylistMessage_t *pxPlaylistMessage;
	pxPlaylistMessage = &xPlaylistMessage;
	pxPlaylistMessage->cmd = pause;
	pxPlaylistMessage->state = newCMD;
	res = AddMessageToPlaylistQueue(queue_handler_playlist, pxPlaylistMessage);


	isPaused = true;
	lv_btn_set_state(btn_play, LV_BTN_STATE_REL);
	lv_btn_set_state(btn, LV_BTN_STATE_INA);
	lv_btn_set_state(btn_stop, LV_BTN_STATE_REL);

	play_buttonState = LV_BTN_STATE_REL;
	pause_buttonState = LV_BTN_STATE_INA;
	stop_buttonState = LV_BTN_STATE_REL;


	return LV_RES_OK; /* Return OK if the button is not deleted */
}

static lv_res_t btn_stop_click_action(lv_obj_t *btn) {

	uint8_t res;
	PlaylistMessage_t *pxPlaylistMessage;
	pxPlaylistMessage = &xPlaylistMessage;

	pxPlaylistMessage->cmd = stop;
	pxPlaylistMessage->state = newCMD;
	res = AddMessageToPlaylistQueue(queue_handler_playlist, pxPlaylistMessage);
	lv_btn_set_state(btn_play, LV_BTN_STATE_REL);
	lv_btn_set_state(btn_pause, LV_BTN_STATE_INA);
	lv_btn_set_state(btn, LV_BTN_STATE_INA);
	isPaused = false;

	play_buttonState = LV_BTN_STATE_REL;
	pause_buttonState = LV_BTN_STATE_INA;
	stop_buttonState = LV_BTN_STATE_INA;


	return LV_RES_OK; /* Return OK if the button is not deleted */
}

static lv_res_t btn_prev_click_action(lv_obj_t *btn) {
	return LV_RES_OK; /* Return OK if the button is not deleted */
}

static lv_res_t btn_next_click_action(lv_obj_t *btn) {
	return LV_RES_OK; /* Return OK if the button is not deleted */
}

static lv_res_t btn_loop_click_action(lv_obj_t *btn) {
	return LV_RES_OK; /* Return OK if the button is not deleted */
}

void updatePollenLabel(char* text){
	pollenLabel = text;
	lv_label_set_text(polle_label, pollenLabel);
}


void updatePlayBtn(uint8_t val){

	if(val == TRUE){
		lv_btn_set_state(btn_play, LV_BTN_STATE_INA);
		lv_btn_set_state(btn_pause, LV_BTN_STATE_REL);
		lv_btn_set_state(btn_stop, LV_BTN_STATE_REL);
	}
	else{
		lv_btn_set_state(btn_play, LV_BTN_STATE_REL);
		lv_btn_set_state(btn_pause, LV_BTN_STATE_INA);
		lv_btn_set_state(btn_stop, LV_BTN_STATE_INA);
	}

}



void GUI_NEO_Create(uint8_t * name) {


	setNavGuiIsActive(true);

	namesActive = name;	// schreibt die Namen der aktiven pollen auf den globalen pointer




	lv_obj_t *closeBtn, *slider1;

	/* Create the window */
	win = lv_win_create(lv_scr_act(), NULL);
	lv_win_set_title(win, "Nav");
	closeBtn = lv_win_add_btn(win, SYMBOL_CLOSE, win_close_action);
	GUI_GroupPush();
	GUI_AddObjToGroup(closeBtn);
	lv_group_focus_obj(closeBtn);
	/* Make the window content responsive */
//   lv_win_set_layout(win, LV_LAYOUT_PRETTY);
	/*-------------------------------------------------------*/
	/* Switch to turn on/off the auto light level */
	/*-------------------------------------------------------*/
	/* Create a label left to the light 'auto' switch */
	lv_obj_t *switch1_label = lv_label_create(win, NULL);
	lv_label_set_text(switch1_label, "Auto");
	lv_obj_align(switch1_label, win, LV_ALIGN_IN_BOTTOM_LEFT, 5, 10);

	/* create a switch for the auto light level setting */
	lv_obj_t *sw1 = lv_sw_create(win, NULL);
	GUI_AddObjToGroup(sw1);
	if (NEOA_GetAutoLightLevelSetting()) {
		lv_sw_on(sw1); /* turn on */
	} else {
		lv_sw_off(sw1); /* turn on */
	}
	lv_sw_set_action(sw1, auto_light_level_action); /* register callback */
	lv_obj_align(sw1, switch1_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

	/* label for the switch status, on the right side of the switch  */
	label_auto_light_level = lv_label_create(win, NULL);
	if (lv_sw_get_state(sw1)) {
		lv_label_set_text(label_auto_light_level, "on");
	} else {
		lv_label_set_text(label_auto_light_level, "off");
	}
	lv_obj_align(label_auto_light_level, sw1, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

	/*-------------------------------------------------------*/
	/* Slider to adjust the light level */
	/*-------------------------------------------------------*/
	/* Create a label left to the light level slider */
	lv_obj_t *slider1_label = lv_label_create(win, NULL);
	lv_label_set_text(slider1_label, "Level");
	lv_obj_align(slider1_label, switch1_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

	/* Create a slider for the light level */
	slider1 = lv_slider_create(win, NULL);
	GUI_AddObjToGroup(slider1);
	lv_obj_set_size(slider1, 50, 15);
	lv_slider_set_range(slider1, SLIDER_LEVEL_MIN_RANGE,
	SLIDER_LEVEL_MAX_RANGE);
	lv_obj_align(slider1, slider1_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
	lv_slider_set_action(slider1, slider_action);
	lv_bar_set_value(slider1,
			UTIL1_map(NEOA_GetLightLevel(), SLIDER_LEVEL_LIGHT_MIN,
			SLIDER_LEVEL_LIGHT_MAX, SLIDER_LEVEL_MIN_RANGE,
			SLIDER_LEVEL_MAX_RANGE));

	/* Create a label with the slider value right to the slider */
	label_slider_level = lv_label_create(win, NULL);
	SetLabelValue(label_slider_level, NEOA_GetLightLevel());
	lv_obj_align(label_slider_level, slider1, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

	/*Create a bar, an indicator and a knob style*/
	static lv_style_t style_bg;
	static lv_style_t style_indic;
	static lv_style_t style_knob;

	lv_style_copy(&style_bg, &lv_style_pretty);
	style_bg.body.main_color = LV_COLOR_BLACK;
	style_bg.body.grad_color = LV_COLOR_GRAY;
	style_bg.body.radius = LV_RADIUS_CIRCLE;
	style_bg.body.border.color = LV_COLOR_WHITE;

	lv_style_copy(&style_indic, &lv_style_pretty);
	style_indic.body.grad_color = LV_COLOR_GREEN;
	style_indic.body.main_color = LV_COLOR_LIME;
	style_indic.body.radius = LV_RADIUS_CIRCLE;
	style_indic.body.shadow.width = 10;
	style_indic.body.shadow.color = LV_COLOR_LIME;
	style_indic.body.padding.hor = 3;
	style_indic.body.padding.ver = 3;

	lv_style_copy(&style_knob, &lv_style_pretty);
	style_knob.body.radius = LV_RADIUS_CIRCLE;
	style_knob.body.opa = LV_OPA_70;
	style_knob.body.padding.ver = 10;


	/* previous button */


#if 0
	btn_prev = lv_btn_create(win, NULL);
	lv_obj_align(btn_prev, btn_play, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
	lv_btn_set_action(btn_prev, LV_BTN_ACTION_CLICK, btn_prev_click_action);
	label = lv_label_create(btn_prev, NULL);
	lv_label_set_text(label, SYMBOL_PREV);
	GUI_AddObjToGroup(btn_prev);
	/* next button */
	btn_next = lv_btn_create(win, NULL);
	lv_obj_align(btn_next, btn_prev, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
	lv_btn_set_action(btn_next, LV_BTN_ACTION_CLICK, btn_next_click_action);
	label = lv_label_create(btn_next, NULL);
	lv_label_set_text(label, SYMBOL_NEXT);
	GUI_AddObjToGroup(btn_next);
	/* loop button */
	btn_loop = lv_btn_create(win, NULL);
	lv_obj_align(btn_loop, btn_next, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
	lv_btn_set_action(btn_loop, LV_BTN_ACTION_CLICK, btn_loop_click_action);
	label = lv_label_create(btn_loop, NULL);
	lv_label_set_text(label, SYMBOL_LOOP);
	GUI_AddObjToGroup(btn_loop);

#endif

	/* Create a label with the name of the playing polle */
	polle_label = lv_label_create(win, NULL);
	lv_label_set_text(polle_label, "");
	lv_obj_align(polle_label, slider1_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
	GUI_AddObjToGroup(polle_label);




	/*-------------------------------------------------------*/
	/* buttons for play, pause, stop, ... */
	/*-------------------------------------------------------*/
	lv_obj_t *label;


	/* play buttons */
	btn_play = lv_btn_create(win, NULL);
	lv_obj_align(btn_play, slider1_label, LV_ALIGN_OUT_BOTTOM_LEFT, 7, 40);
	lv_btn_set_action(btn_play, LV_BTN_ACTION_CLICK, btn_play_click_action);
	label = lv_label_create(btn_play, NULL);
	lv_btn_set_state(btn_play,play_buttonState);
	lv_label_set_text(label, SYMBOL_PLAY);
	GUI_AddObjToGroup(btn_play);
	/* pause button */
	btn_pause = lv_btn_create(win, NULL);
	lv_obj_align(btn_pause, btn_play, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	lv_btn_set_action(btn_pause, LV_BTN_ACTION_CLICK, btn_pause_click_action);
	label = lv_label_create(btn_pause, NULL);
	lv_btn_set_state(btn_pause,pause_buttonState);
	lv_label_set_text(label, SYMBOL_PAUSE);
	GUI_AddObjToGroup(btn_pause);
	/* stop button */

	btn_stop = lv_btn_create(win, NULL);
	lv_obj_align(btn_stop, btn_pause, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	lv_btn_set_action(btn_stop, LV_BTN_ACTION_CLICK, btn_stop_click_action);
	label = lv_label_create(btn_stop, NULL);
	lv_btn_set_state(btn_stop,stop_buttonState);
	lv_label_set_text(label, SYMBOL_STOP);
	GUI_AddObjToGroup(btn_stop);











}
#endif /* PL_CONFIG_HAS_NEO_PIXEL */
