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
static lv_obj_t *win; /* object for window */
static lv_res_t win_close_action(lv_obj_t *btn) {
	GUI_GroupPull();
	lv_obj_del(win);
	win = NULL;
	return LV_RES_INV;
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
	namesSet[pos] =isChecked;


	return LV_RES_OK;
}

static void updateCheckbox(lv_obj_t * cb, uint8_t pos){


	bool checked = false;

	checked = namesSet[pos];


	lv_cb_set_checked(cb,checked);


}

void GUI_POLLEN_Create(void) {



	lv_obj_t *closeBtn;
	win = lv_win_create(lv_scr_act(), NULL);
	closeBtn = lv_win_add_btn(win, SYMBOL_CLOSE, win_close_action);
	GUI_GroupPush();
	char ** name = getNamelist();
	int nOFnames = getQuantity();

	int x = 0;
	GUI_AddObjToGroup(closeBtn);
	lv_group_focus_obj(closeBtn);

	lv_win_set_title(win, "Pollenarten");


	/********************************************
	 * Create list ob objects
	 ********************************************/

	/*
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

	/********************************************
	 * Create a container for the check boxes
	 ********************************************/

	/*Create a container*/
	/*
	 lv_obj_t * cont;
	 cont = lv_cont_create(win, NULL);
	 lv_obj_set_click(cont,TRUE);
	 GUI_AddObjToGroup(cont);
	 lv_cont_set_layout(cont, LV_LAYOUT_COL_L);      /*Arrange the children in a column*/
	/*
	 lv_cont_set_fit(cont, true, true);              /*Fit the size to the content*/
	/*	lv_obj_set_style(cont, &style_border);



	 /*Create check boxes*/

	uint16_t posX = 0;
	uint16_t posY = 0;
	lv_obj_t * cb;
	uint8_t counter = 0;

	while (x < nOFnames) {
		if(x>4){
			cb = lv_cb_create(win, NULL);
			lv_obj_set_pos(cb, posX, posY);
			GUI_AddObjToGroup(cb);
			lv_cb_set_text(cb, (name[x]));
			lv_cb_set_action(cb, cb_release_action);
			updateCheckbox(cb,counter);
			x++;
			posX = 50;
			posY = posY + 20;
		}
		else{
		cb = lv_cb_create(win, NULL);
		lv_obj_set_pos(cb, posX, posY);
		GUI_AddObjToGroup(cb);
		lv_cb_set_text(cb, (name[x]));
		lv_cb_set_action(cb, cb_release_action);
		updateCheckbox(cb,counter);
		counter++;
		if(x == 4){
			posY = 0;
		}

		x++;
		posY = posY + 20;
		}
	}

	lv_obj_t *label;
	lv_obj_t *btn_play;

	btn_play = lv_btn_create(win, NULL);
	lv_obj_set_pos(btn_play, 90, 80);
	lv_btn_set_action(btn_play, LV_BTN_ACTION_CLICK, btn_play_click_action);
	label = lv_label_create(btn_play, NULL);
	lv_label_set_text(label, SYMBOL_PLAY);
	GUI_AddObjToGroup(btn_play);




	/*Align the container to the middle*/
	//lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);

}
