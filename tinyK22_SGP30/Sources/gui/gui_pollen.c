/*
 * gui_pollen.c
 *
 *  Created on: 13.12.2018
 *      Author: Pescatore
 */
#include "Platform.h"
#include "gui_pollen.h"
#include "gui_mainmenu.h" /* own interface */
#include "gui.h"
#include "lvgl/lvgl.h" /* interface to GUI library */
#include "lv.h"
#include "UTIL1.h"
#include "gui_neopixel.h"
#include "config.h"

static uint8_t namesSet[MAX_N_POLLS_STORED];
static lv_obj_t *win; /* object for page */
static lv_group_t * group;
static lv_group_t * g;

static void group_focus_cb(lv_group_t * group) {
	lv_obj_t * f = lv_group_get_focused(group);

	/*Get the wondows content object */
	lv_obj_t * par = lv_obj_get_parent(f); /*The content object is page so first get scrollable object*/
	if (par)
		par = lv_obj_get_parent(par); /*Then get the page itself*/

	/*If the focused object is on the window then scrol lteh window to make it visible*/
	if (par == lv_win_get_content(win)) {
		lv_win_focus(win, f, 200);
	} else {
	//	lv_group_focus_next(group);
	}
}

static lv_res_t exit_action(struct _lv_obj_t *obj) {

	//lv_label_ext_t * ext = lv_obj_get_ext_attr(obj);

	//GUI_NEO_Create();
	return LV_RES_OK;
}
static lv_res_t my_click_action(struct _lv_obj_t *obj) {
	return LV_RES_OK;
}

static lv_res_t btn_play_click_action(lv_obj_t *btn) {
	GUI_NEO_Create(namesSet);
	return LV_RES_OK; /* Return OK if the button is not deleted */
}

static uint16_t getPosInNamelist(char* name) {

	int i = 0;
	int nOFnames = getQuantity();
	uint16_t pos = 0;
	for (i; i < nOFnames; i++) {
		if (strcmp(name, getNamelist()[i]) == 0) {
			return pos;
		} else {
			pos++;
		}
	}

}
static lv_res_t cb_release_action(lv_obj_t * cb) {
	/*A check box is clicked*/
	uint8_t length = strlen(lv_cb_get_text(cb) + 1);
	char text[length];
	strcpy(text, lv_cb_get_text(cb));
	bool isChecked = lv_cb_is_checked(cb);
	uint16_t pos = getPosInNamelist(text);
	namesSet[pos] = isChecked;

	return LV_RES_OK;
}

static void updateCheckbox(lv_obj_t * cb, uint8_t pos) {

	bool checked = false;
	checked = namesSet[pos];
	lv_cb_set_checked(cb, checked);

}

static lv_res_t win_close_action(lv_obj_t *btn) {
	GUI_GroupPull();
	lv_obj_del(win);
	win = NULL;
	return LV_RES_INV;
}

void GUI_POLLEN_Create(void) {

	GUI_GroupPush();

	static lv_style_t win_style;
	lv_style_copy(&win_style, &lv_style_transp);
	win_style.body.padding.hor = LV_DPI / 4;
	win_style.body.padding.ver = LV_DPI / 4;
	win_style.body.padding.inner = LV_DPI / 4;

	win = lv_win_create(lv_scr_act(), NULL);
	lv_win_set_title(win, "Playlist");
	lv_page_set_scrl_layout(lv_win_get_content(win), LV_LAYOUT_COL_L);
	lv_win_set_style(win, LV_WIN_STYLE_CONTENT_SCRL, &win_style);

	/* Create close Window Button*/
	lv_obj_t *closeBtn;
	closeBtn = lv_win_add_btn(win, SYMBOL_CLOSE, win_close_action);
	lv_group_add_obj(GUI_GroupPeek(), closeBtn);
	lv_group_focus_obj(closeBtn);

	/*Create check boxes*/
	int x = 0;
	int nOFnames = getQuantity();	//  get number of pollen to initialize
	char ** name = getNamelist();	//  get names of the pollen
	lv_obj_t * cb;
	uint8_t counter = 0;

	while (x < nOFnames) {			// Initializes the desired pollen

		cb = lv_cb_create(win, NULL);
		lv_group_add_obj(GUI_GroupPeek(), cb);
		lv_cb_set_text(cb, (name[x]));
		lv_cb_set_action(cb, cb_release_action);//define what happen when checkbox pushed/released
		updateCheckbox(cb, counter);
		counter++;
		x++;

	}

	lv_obj_t *label;
	lv_obj_t *btn_play;

	btn_play = lv_btn_create(win, NULL);
	lv_btn_set_action(btn_play, LV_BTN_ACTION_CLICK, btn_play_click_action);
	label = lv_label_create(btn_play, NULL);
	lv_label_set_text(label, SYMBOL_PLAY);
	lv_group_add_obj(GUI_GroupPeek(), btn_play);

	lv_group_set_focus_cb(GUI_GroupPeek(), group_focus_cb);


}
