/*
 * gui_single.c
 *
 *  Created on: 19.11.2018
 *      Author: Pescatore
 */




/*
 * gui_Modus.c
 *
 *  Created on: 19.11.2018
 *      Author: Pescatore
 */

#include "Platform.h"
#if PL_CONFIG_HAS_CONFIG_MENU
#include "gui_Modus.h"
#include "lvgl/lvgl.h"
#include "gui.h"
#include "..\WS2812B\NeoPixel.h"
#include "gui_mainmenu.h"
#include "gui.h"

static lv_obj_t *win;
static bool TSL1_Flag = TRUE;

/* Called when a new value id set on the slider */
static lv_res_t slider_action(lv_obj_t * slider) {
//    printf("New slider value: %d\n", lv_slider_get_value(slider));
	return LV_RES_OK;
}

/**
 * Called when the window's close button is clicked
 * @param btn pointer to the close button
 * @return LV_ACTION_RES_INV because the window is deleted in the function
 */
static lv_res_t win_close_action(lv_obj_t *btn) {
	// lv_group_focus_freeze(GUI_GetGroup(), false);
	lv_obj_del(win);
	win = NULL;
	return LV_RES_INV;
}





static lv_res_t cb_release_action(lv_obj_t * cb) {
	/*A check box is clicked*/
	uint8_t blub = 13;

	return LV_RES_OK;
}

static lv_res_t btn_click_action(lv_obj_t * btn) {
	uint8_t id = lv_obj_get_free_num(btn);

	//printf("Button %d is released\n", id);

	lv_obj_del(win);
	win = NULL;

	return LV_RES_OK; /*Return OK if the button is not deleted*/
}

void GUI_SINGLE_Create(void) {

	lv_obj_t *closeBtn;
	/* create window */
	win = lv_win_create(lv_scr_act(), NULL);
	lv_win_set_title(win, "Play single Poll");
	closeBtn = lv_win_add_btn(win, SYMBOL_CLOSE, win_close_action);
	GUI_AddObjToGroup(closeBtn);
	lv_group_focus_obj(closeBtn);

	//lv_group_focus_freeze(GUI_GetGroup(), true); /* otherwise the items of the underlying view are still active */
	/* Make the window content responsive */
	lv_win_set_layout(win, LV_LAYOUT_PRETTY);



}
#endif /* PL_CONFIG_HAS_NEO_PIXEL */
