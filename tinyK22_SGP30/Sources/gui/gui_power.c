/*
 * gui_power.c
 *
 *  Created on: 18.12.2018
 *      Author: Pescatore
 */

#include "gui_power.h"

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

static lv_obj_t *btn10A;
static lv_obj_t *btn20A;
static lv_obj_t *btn30A;

static lv_res_t win_close_action(lv_obj_t *btn) {
	GUI_GroupPull();
	lv_obj_del(win);
	win = NULL;
	return LV_RES_INV;
}

static lv_res_t Btn_Power_click_action(lv_obj_t *btn) {
	lv_btn_state_t state;
	state = lv_btn_get_state(btn);

	if (btn == btn10A) {
		setPowerConnected(1);
		/*Set global Power connected value to 10A*/
		if (lv_btn_get_state(btn20A) == LV_BTN_STATE_INA) { /*20A was pressed*/
			lv_btn_set_state(btn20A, LV_BTN_STATE_REL);
		} else { /*30A was pressed*/
			lv_btn_set_state(btn30A, LV_BTN_STATE_REL);
		}
		lv_btn_set_state(btn10A, LV_BTN_STATE_INA);

	}

	else if (btn == btn20A) {
		setPowerConnected(2);
		/*Set global Power connected value to 20A*/
		if (lv_btn_get_state(btn10A) == LV_BTN_STATE_INA) { /*10A was pressed*/
			lv_btn_set_state(btn10A, LV_BTN_STATE_REL);
		} else { /*30A was pressed*/
			lv_btn_set_state(btn30A, LV_BTN_STATE_REL);
		}
		lv_btn_set_state(btn20A, LV_BTN_STATE_INA);

	}

	else if (btn == btn30A) {
		setPowerConnected(3);
		/*Set global Power connected value to 30A*/
		if (lv_btn_get_state(btn10A) == LV_BTN_STATE_INA) { /*10A was pressed*/
			lv_btn_set_state(btn10A, LV_BTN_STATE_REL);
		} else { /*20A was pressed*/
			lv_btn_set_state(btn20A, LV_BTN_STATE_REL);
		}
		lv_btn_set_state(btn30A, LV_BTN_STATE_INA);

	}

	//GUI_NEO_Create(namesSet);
	return LV_RES_OK; /* Return OK if the button is not deleted */
}

void update_btn_power_state(void) {

	/*get global settings for Power connected and set state inactive to the button related*/
	uint8_t val = getPowerConnected();

	switch (val) {
	case 1:
		lv_btn_set_state(btn10A, LV_BTN_STATE_INA);
		break;
	case 2:
		lv_btn_set_state(btn20A, LV_BTN_STATE_INA);
		break;
	case 3:
		lv_btn_set_state(btn30A, LV_BTN_STATE_INA);
		break;
	}

}

void GUI_Power_Create(void) {

	win = lv_win_create(lv_scr_act(), NULL);
	lv_win_set_title(win, "Power connected");

	lv_obj_t *closeBtn;
	closeBtn = lv_win_add_btn(win, SYMBOL_CLOSE, win_close_action);
	GUI_GroupPush();

	int x = 0;
	GUI_AddObjToGroup(closeBtn);
	lv_group_focus_obj(closeBtn);

	/* create window */

	/* Make the window content responsive */
	//lv_win_set_layout(gui_win, LV_LAYOUT_PRETTY); /* this will arrange the buttons */
	/* create list of objects */
	lv_obj_t *list1;

	list1 = lv_list_create(win, NULL);
	/*Add list elements*/

#if PL_CONFIG_HAS_NEO_PIXEL
	//obj = lv_list_add(list1, SYMBOL_CLOSE, "Pollen", Btn_NeoPixel_click_action);
	btn10A = lv_list_add(list1, NULL, "10 A", Btn_Power_click_action);
	GUI_AddObjToGroup(btn10A);

#endif

#if PL_CONFIG_HAS_TSL2561
	btn20A = lv_list_add(list1, NULL, "20 A", Btn_Power_click_action);

	GUI_AddObjToGroup(btn20A);
#endif

#if PL_CONFIG_HAS_TSL2561
	btn30A = lv_list_add(list1, NULL, "30 A", Btn_Power_click_action);

	GUI_AddObjToGroup(btn30A);
#endif

	update_btn_power_state();

	lv_obj_set_size(list1, 50, 100); /* fixed size */

	lv_obj_align(list1, win, LV_ALIGN_CENTER, 0, 0);

	/*Align the container to the middle*/
	//lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);
}
