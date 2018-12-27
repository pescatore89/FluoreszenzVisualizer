/*
 * Images.c
 *
 *  Created on: 27.12.2018
 *      Author: Pescatore
 */

#include "Images.h"

#include "Platform.h"
#include "gui_pollen.h"
#include "gui_mainmenu.h" /* own interface */
#include "gui.h"
#include "lvgl/lvgl.h" /* interface to GUI library */
#include "lv.h"
#include "UTIL1.h"
#include "gui_neopixel.h"
#include "config.h"
#include "Message.h"

static lv_obj_t *win; /* object for window */
static lv_obj_t** cbList;
static uint8_t imagesChecked[100];

static lv_obj_t *label_play;
static lv_obj_t *btn_play;

static uint8_t guiIsActive;

static lv_obj_t *label_clear;
static lv_obj_t *btn_clear;

static bool displaying = FALSE;

static bool playingPollen = FALSE;

void setPlayingPollen(uint8_t val) {

	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	playingPollen = val;
	CS1_ExitCritical()
	;

}

static uint8_t getPlayingPollen(void) {

	uint8_t result;

	CS1_CriticalVariable()
	;
	CS1_EnterCritical()
	;
	result = playingPollen;
	CS1_ExitCritical()
	;
	return result;
}



static void setImageGuiIsActive(uint8_t val){

	CS1_CriticalVariable();


	CS1_EnterCritical();

	guiIsActive = val;

	CS1_ExitCritical();

}

static bool hasElementSelected(void) {

	bool result = FALSE;

	for (int i = 0; i < getQuantityOfImages(); i++) {
		if (lv_cb_is_checked(cbList[i])) {
			result = TRUE;
		}
	}
	return result;

}

static char* getNameSelected(void) {
	char* name;
	char ** names = getImagesList();

	for (int i = 0; i < getQuantityOfImages(); i++) { /*store the state*/
		if (lv_cb_is_checked(cbList[i])) {
			name = names[i];
		}

	}
	return name;
}

static lv_res_t win_close_action(lv_obj_t *btn) {

	for (int i = 0; i < getQuantityOfImages(); i++) { /*store the state*/

		imagesChecked[i] = lv_cb_is_checked(cbList[i]);

	}
	setImageGuiIsActive(false);
	FRTOS1_vPortFree(cbList);
	GUI_GroupPull();
	lv_obj_del(win);
	win = NULL;

	return LV_RES_INV;
}

static lv_res_t cb_release_action(lv_obj_t * cb) {
	/*A check box is clicked*/
	uint8_t length = strlen(lv_cb_get_text(cb) + 1);
	char text[length];
	strcpy(text, lv_cb_get_text(cb));
	bool wasChecked = FALSE;
	lv_style_t style;

	//bool isChecked = lv_cb_is_checked(cb);
	//uint16_t pos = getPosInNamelist(text);

	for (int i = 0; i < getQuantityOfImages(); i++) { /*uncheck the others*/

		if (cbList[i] == cb) {
			lv_cb_set_style(cb, LV_CB_STYLE_BOX_TGL_PR, &style);

		} else {
			lv_cb_set_checked(cbList[i], FALSE);
		}

	}

	if (!hasElementSelected()) {
		if (!displaying) {
			lv_btn_set_state(btn_play, LV_BTN_STATE_INA);
		}
	} else {
		lv_btn_set_state(btn_play, LV_BTN_STATE_REL);
	}

	return LV_RES_OK;
}

static lv_res_t Btn_Images_click_action(lv_obj_t *btn) {

	GUI_Images_Create();
	return LV_RES_OK;

}

static void update(void) {

	lv_style_t style;
	for (int i = 0; i < getQuantityOfImages(); i++) { /*store the state*/
		if (imagesChecked[i]) {
			lv_cb_set_checked(cbList[i], TRUE);
		}
	}

	if (displaying) {
		lv_label_set_text(label_play, SYMBOL_CLOSE);
	} else {
		lv_label_set_text(label_play, SYMBOL_OK);
	}

	if (!hasElementSelected()) {
		lv_btn_set_state(btn_play, LV_BTN_STATE_INA);
	}

}

static lv_res_t btn_play_click_action(lv_obj_t *btn) {

	uint8_t res;
	PlaylistMessage_t *pxPlaylistMessage;
	pxPlaylistMessage = &xPlaylistMessage;
	char *name;

	if (displaying) {
		lv_label_set_text(label_play, SYMBOL_OK);
		pxPlaylistMessage->state = clrImage;
		res = AddMessageToPlaylistQueue(queue_handler_playlist,
				pxPlaylistMessage);
		if (!hasElementSelected()) {
			lv_btn_set_state(btn_play, LV_BTN_STATE_INA);
		}
		displaying = FALSE;
	} else {
		name = getNameSelected();
		pxPlaylistMessage->state = newImage;
		pxPlaylistMessage->imageName = name;

		res = AddMessageToPlaylistQueue(queue_handler_playlist,
				pxPlaylistMessage);
		lv_label_set_text(label_play, SYMBOL_CLOSE);
		displaying = TRUE;
	}

	return LV_RES_OK; /* Return OK if the button is not deleted */
}




uint8_t getImageGuiIsActive(void){

	uint8_t res;
	CS1_CriticalVariable();


	CS1_EnterCritical();

		res = guiIsActive;

	CS1_ExitCritical();



	return res;
}





void GUI_Images_Create(void) {

	setImageGuiIsActive(true);
	win = lv_win_create(lv_scr_act(), NULL);
	lv_win_set_title(win, "Bilder");

	lv_obj_t *closeBtn;
	closeBtn = lv_win_add_btn(win, SYMBOL_CLOSE, win_close_action);
	GUI_GroupPush();

	char ** names = getImagesList();
	int nOFnames = getQuantityOfImages();

	int x = 0;
	GUI_AddObjToGroup(closeBtn);
	lv_group_focus_obj(closeBtn);

	/* create window */

	/* Make the window content responsive */
	/*Create check boxes*/

	cbList = (lv_obj_t**) FRTOS1_pvPortMalloc(sizeof(lv_obj_t*) * nOFnames);
	if (cbList != NULL) {

		uint16_t posX = 0;
		uint16_t posY = 0;
		lv_obj_t * cb;
		uint8_t counter = 0;

		while (x < nOFnames) {
			if (x > 4) {
				cbList[x] = lv_cb_create(win, NULL);
				lv_obj_set_pos(cbList[x], posX, posY);
				GUI_AddObjToGroup(cbList[x]);
				lv_cb_set_text(cbList[x], (names[x]));
				lv_cb_set_action(cbList[x], cb_release_action);
				//updateCheckbox(cb,counter);
				x++;
				posX = 50;
				posY = posY + 20;
			} else {
				cbList[x] = lv_cb_create(win, NULL);
				lv_obj_set_pos(cbList[x], posX, posY);
				GUI_AddObjToGroup(cbList[x]);
				lv_cb_set_text(cbList[x], (names[x]));
				lv_cb_set_action(cbList[x], cb_release_action);
				//updateCheckbox(cb,counter);
				counter++;
				if (x == 4) {
					posY = 0;
				}

				x++;
				posY = posY + 20;
			}
		}

		btn_play = lv_btn_create(win, NULL);
		lv_obj_set_pos(btn_play, 90, 80);
		lv_btn_set_action(btn_play, LV_BTN_ACTION_CLICK, btn_play_click_action);
		label_play = lv_label_create(btn_play, NULL);
		lv_label_set_text(label_play, SYMBOL_OK);
		GUI_AddObjToGroup(btn_play);

		update();

	}

	else {
		/*error Malloc*/
	}

}

