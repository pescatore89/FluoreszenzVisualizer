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

static lv_obj_t *win; /* object for window */
static lv_res_t win_close_action(lv_obj_t *btn) {
	GUI_GroupPull();
	lv_obj_del(win);
	win = NULL;
	return LV_RES_INV;
}

static lv_res_t Btn_Ambient2_click_action(struct _lv_obj_t *obj) {

	lv_label_ext_t * ext = lv_obj_get_ext_attr(obj);

	GUI_NEO_Create();
	return LV_RES_OK;
}
static lv_res_t Btn_Pollen2_click_action(struct _lv_obj_t *obj) {
	return LV_RES_OK;
}
void GUI_POLLEN_Create(void) {
	lv_obj_t *closeBtn;
	win = lv_win_create(lv_scr_act(), NULL);
	closeBtn = lv_win_add_btn(win, SYMBOL_CLOSE, win_close_action);
	GUI_GroupPush();
	GUI_AddObjToGroup(closeBtn);
	lv_group_focus_obj(closeBtn);
	char ** name = getNamelist();
	int nOFnames = getQuantity();
	int x = 0;
	lv_win_set_title(win, "Gespeicherte Pollendaten");

	/* create list of objects */

	lv_obj_t *obj;
	lv_obj_t *list1;
	list1 = lv_list_create(win, NULL);

	while (x < nOFnames) {
		obj = lv_list_add(list1, SYMBOL_CLOSE, (name[x]),
				Btn_Ambient2_click_action);
		GUI_AddObjToGroup(obj);
		x++;

	}

	lv_obj_set_size(list1, 64, 100); /* fixed size */
}
