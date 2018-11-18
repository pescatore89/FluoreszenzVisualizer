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
#include "..\WS2812B\NeoPixel.h"
//#include "..\PollenHolo\testMatrix.h"

static lv_obj_t *win;



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

static lv_res_t Btn_play(struct _lv_obj_t *obj) {

	lv_area_t co;
	co = obj->coords;


	if (co.y1 == OFFSET_DISTANCE_BUTTON_ZERO) {			// Button Nr1
		NEO_ClearAllPixel();
		NEO_TransferPixels();
	}

#if N_OF_POLLS_HOLO >= 1
	else if (co.y1 == OFFSET_DISTANCE_BUTTON_ZERO + DISTANCE_BTW_BUTTON) {// Button Nr2
		NEO_ClearAllPixel();
		NEO_DislpayHolo(SWITZERLAND);
		NEO_TransferPixels();
	}
#endif
#if N_OF_POLLS_HOLO >= 2
	else if (co.y1 == OFFSET_DISTANCE_BUTTON_ZERO+ (DISTANCE_BTW_BUTTON *2)) {// Button Nr3
		NEO_ClearAllPixel();
		NEO_DislpayHolo(CIRCLE);
		NEO_TransferPixels();
	}
#endif
#if N_OF_POLLS_HOLO >= 3
	else if (co.y1 == OFFSET_DISTANCE_BUTTON_ZERO+ (DISTANCE_BTW_BUTTON *3)) {// Button Nr4
		NEO_ClearAllPixel();
		NEO_DislpayHolo(SMILEY);
		NEO_TransferPixels();
	}
#endif
#if N_OF_POLLS_HOLO >= 4
	else if (co.y1 == OFFSET_DISTANCE_BUTTON_ZERO+ (DISTANCE_BTW_BUTTON *4)) {// Button Nr5
		NEO_ClearAllPixel();
		NEO_TransferPixels();
	}

#endif

	return LV_RES_OK;
}

void GUI_NEO_Create(void) {
	lv_obj_t *closeBtn;
	/* create window */
	win = lv_win_create(lv_scr_act(), NULL);
	lv_win_set_title(win, "Have some fun--NOT");
	closeBtn = lv_win_add_btn(win, SYMBOL_CLOSE, win_close_action);
	GUI_AddObjToGroup(closeBtn);
	lv_group_focus_obj(closeBtn);
	//lv_group_focus_freeze(GUI_GetGroup(), true); /* otherwise the items of the underlying view are still active */
	/* Make the window content responsive */
	lv_win_set_layout(win, LV_LAYOUT_PRETTY);


	/* create list of objects */
	lv_obj_t *list1;
	lv_obj_t *obj;

	list1 = lv_list_create(win, NULL);
	/*Add list elements*/

	obj = lv_list_add(list1, SYMBOL_CLOSE, "clear", Btn_play);
	GUI_AddObjToGroup(obj);
	obj = lv_list_add(list1, SYMBOL_CLOSE, "SWITZERLAND", Btn_play);
	GUI_AddObjToGroup(obj);
	obj = lv_list_add(list1, SYMBOL_CLOSE, "CIRCLE", Btn_play);
	GUI_AddObjToGroup(obj);
	obj = lv_list_add(list1, SYMBOL_CLOSE, "SMILEY", Btn_play);
	GUI_AddObjToGroup(obj);


}

#if 0
void GUI_NEO_Create(void) {
	lv_obj_t *closeBtn, *slider1;

	win = lv_win_create(lv_scr_act(), NULL);
	lv_win_set_title(win, "NeoPixel");
	closeBtn = lv_win_add_btn(win, SYMBOL_CLOSE, win_close_action);
	GUI_AddObjToGroup(closeBtn);
	lv_group_focus_obj(closeBtn);
	// lv_group_focus_freeze(GUI_GetGroup(), true); /* otherwise the items of the underlying view are still active */
	/* Make the window content responsive */
	lv_win_set_layout(win, LV_LAYOUT_PRETTY);

	/* Create a slider*/
	slider1 = lv_slider_create(win, NULL);
	GUI_AddObjToGroup(slider1);
	lv_obj_set_size(slider1, 80, 20);
	lv_obj_align(slider1, NULL, LV_ALIGN_IN_TOP_RIGHT, -30, 30);
	lv_slider_set_action(slider1, slider_action);
	lv_bar_set_value(slider1, 70);

	/* Create a label right to the slider */
	lv_obj_t * slider1_label = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_text(slider1_label, "Default");
	lv_obj_align(slider1_label, slider1, LV_ALIGN_OUT_LEFT_MID, -20, 0);

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
}
#endif
#endif /* PL_CONFIG_HAS_NEO_PIXEL */
