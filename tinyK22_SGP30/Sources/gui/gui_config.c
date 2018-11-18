/*
 * gui_config.c
 *
 *  Created on: 18.11.2018
 *      Author: Pescatore
 */

#include "Platform.h"
#if PL_CONFIG_HAS_CONFIG_MENU
#include "gui_config.h"
#include "lvgl/lvgl.h"
#include "gui.h"
#include "..\WS2812B\NeoPixel.h"
#include "gui_mainmenu.h"
#include "gui.h"
#include "TSL1.h"

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

#if PL_CONFIG_HAS_TSL2561
/**
 * Called when the Ambient button is clicked
 * @param btn pointer to the close button
 * @return LV_ACTION_RES_INV because the window is deleted in the function
 */
static lv_res_t Btn_Ambient_click_enable_action(struct _lv_obj_t *obj) {

	if (TSL1_Flag) {
		TSL1_Disable();
		TSL1_Flag = FALSE;
	} else {

		TSL1_Enable();
		TSL1_Flag = TRUE;
	}
	return LV_RES_OK;
}
#endif

static lv_res_t btn_click_action(lv_obj_t * btn) {
	uint8_t id = lv_obj_get_free_num(btn);

	//printf("Button %d is released\n", id);

	lv_obj_del(win);
	win = NULL;
	return LV_RES_INV;

	return LV_RES_OK; /*Return OK if the button is not deleted*/
}

void GUI_MenuCreate(void) {
	lv_obj_t *gui_win;

	/* create window */
	gui_win = lv_win_create(lv_scr_act(), NULL);
	lv_win_set_title(gui_win, "Menu");

	/* Make the window content responsive */
	lv_win_set_layout(gui_win, LV_LAYOUT_PRETTY); /* this will arrange the buttons */

	/* create list of objects */
	lv_obj_t *list1;
	lv_obj_t *obj;

	list1 = lv_list_create(gui_win, NULL);
	/*Add list elements*/
#if PL_CONFIG_HAS_TSL2561
	obj = lv_list_add(list1, SYMBOL_CLOSE, "Ambient ON/OFF",
			Btn_Ambient_click_enable_action);
	GUI_AddObjToGroup(obj);
#endif

}

void GUI_Config_Create(void) {

	GUI_CreateGroup();
	GUI_MenuCreate();
	NEO_ClearAllPixel();
	NEO_SetPixelColor(0, 16, 0x090900);
	NEO_TransferPixels();
	int m = 12;

}
#endif /* PL_CONFIG_HAS_NEO_PIXEL */
