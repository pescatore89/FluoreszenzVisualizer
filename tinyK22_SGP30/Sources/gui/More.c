/*
 * More.c
 *
 *  Created on: 27.12.2018
 *      Author: Pescatore
 */

#include "More.h"

#include "Platform.h"
#include "gui_pollen.h"
#include "gui_mainmenu.h" /* own interface */
#include "gui.h"
#include "lvgl/lvgl.h" /* interface to GUI library */
#include "lv.h"
#include "UTIL1.h"
#include "gui_neopixel.h"
#include "config.h"
#include "Images.h"

static lv_obj_t *win; /* object for window */

static lv_obj_t *images;
static lv_obj_t *lauflicht;
static bool isPlayingLauflicht = false;



static lv_res_t win_close_action(lv_obj_t *btn) {
	GUI_GroupPull();
	lv_obj_del(win);
	win = NULL;
	return LV_RES_INV;
}



bool getPlayingLauflicht (void){
	return isPlayingLauflicht;
}






static lv_res_t Btn_Images_click_action(lv_obj_t *btn) {

	GUI_Images_Create();
	return LV_RES_OK;


}



static lv_res_t Btn_Lauflicht_click_action(lv_obj_t *btn){


	if(isPlayingLauflicht){

	}

}

void GUI_More_Create() {
	win = lv_win_create(lv_scr_act(), NULL);
	lv_win_set_title(win, "Weiteres");

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


	//obj = lv_list_add(list1, SYMBOL_CLOSE, "Pollen", Btn_NeoPixel_click_action);
	images = lv_list_add(list1, NULL, "Bilder", Btn_Images_click_action);
	GUI_AddObjToGroup(images);

#if 0
	lauflicht = lv_list_add(list1, NULL, "Lauflicht", Btn_Lauflicht_click_action);
	GUI_AddObjToGroup(lauflicht);
#endif
	lv_obj_set_size(list1, 80, 100); /* fixed size */

	lv_obj_align(list1, win, LV_ALIGN_CENTER, 0, 0);

	/*Align the container to the middle*/
	//lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);
}

