/*
 * gui.c
 *
 *  Created on: 03.08.2018
 *      Author: Erich Styger
 */
#include "Platform.h"
#if PL_CONFIG_HAS_GUI

#include "gui.h"
#include "lv.h"
#include "lvgl/lvgl.h"
#include "FreeRTOS.h"
#include "task.h"
#include "LCD1.h"
#if PL_CONFIG_HAS_KEYS
  #include "KEY1.h"
#endif
#include "gui_mainmenu.h"
#include "gui_neopixel.h"
#include "Message.h"

#include "CS1.h"

#if PL_CONFIG_HAS_GUI_KEY_NAV
#define GUI_GROUP_NOF_IN_STACK   4
typedef struct {
  lv_group_t *stack[GUI_GROUP_NOF_IN_STACK]; /* stack of GUI groups */
  uint8_t sp; /* stack pointer, points to next free element */
} GUI_Group_t;

static GUI_Group_t groups;

xQueueHandle queue_handler_update;

/* style modification callback for the focus of an element */
static void style_mod_cb(lv_style_t *style) {
#if LV_COLOR_DEPTH != 1
    /*Make the style to be a little bit orange*/
    style->body.border.opa = LV_OPA_COVER;
    style->body.border.color = LV_COLOR_ORANGE;

    /*If not empty or has border then emphasis the border*/
    if(style->body.empty == 0 || style->body.border.width != 0) style->body.border.width = LV_DPI / 50;

    style->body.main_color = lv_color_mix(style->body.main_color, LV_COLOR_ORANGE, LV_OPA_70);
    style->body.grad_color = lv_color_mix(style->body.grad_color, LV_COLOR_ORANGE, LV_OPA_70);
    style->body.shadow.color = lv_color_mix(style->body.shadow.color, LV_COLOR_ORANGE, LV_OPA_60);

    style->text.color = lv_color_mix(style->text.color, LV_COLOR_ORANGE, LV_OPA_70);
#else
    style->body.border.opa = LV_OPA_COVER;
    style->body.border.color = LV_COLOR_BLACK;
    style->body.border.width = 3;
#endif
}

void GUI_AddObjToGroup(lv_obj_t *obj) {
  lv_group_add_obj(GUI_GroupPeek(), obj);
}

void GUI_RemoveObjFromGroup_(lv_obj_t *obj) {
  lv_group_remove_obj(obj);
}

lv_group_t *GUI_GroupPeek(void) {
  if (groups.sp == 0) {
    return NULL;
  }
  return groups.stack[groups.sp-1];
}

void GUI_GroupPull(void) {
  if (groups.sp == 0) {
    return;
  }
  lv_group_del(groups.stack[groups.sp-1]);
  groups.sp--;
  lv_indev_set_group(LV_GetInputDevice(), groups.stack[groups.sp-1]); /* assign group to input device */
}

void GUI_GroupPush(void) {
  lv_group_t *gui_group;

  if (groups.sp >= GUI_GROUP_NOF_IN_STACK) {
    return;
  }
  gui_group = lv_group_create();
  lv_indev_set_group(LV_GetInputDevice(), gui_group); /* assign group to input device */
  /* change the default focus style which is an orangish thing */
  lv_group_set_style_mod_cb(gui_group, style_mod_cb);
  groups.stack[groups.sp] = gui_group;
  groups.sp++;
}

#endif /* PL_CONFIG_HAS_GUI_KEY_NAV */

#include "GDisp1.h"







QUEUE_RESULT AddMessageToUpdateQueue(xQueueHandle handle,UpdateMessage_t *msg);
QUEUE_RESULT TakeMessageFromUpdateQueue(xQueueHandle handle,UpdateMessage_t *msg);


static void GuiTask(void *p) {
  vTaskDelay(pdMS_TO_TICKS(1000)); /* give hardware time to power up */
  LCD1_Init();
  uint8_t isActive = false;
  UpdateMessage_t* rxMessage;
  rxMessage = &xUpdateMessage;


//  GDisp1_DrawBox(0, 0, 50, 20, 2, GDisp1_COLOR_RED);
//  GDisp1_UpdateFull();

char text [100] = "Push play to start";


#if PL_CONFIG_HAS_GUI_KEY_NAV
  //GUI_CreateGroup();
#endif
  GUI_MainMenuCreate();
	for(;;) {


		if(TakeMessageFromUpdateQueue(queue_handler_update,rxMessage)!=QUEUE_EMPTY){
			if(rxMessage->cmd == play){
				if(rxMessage->excitation == 1){
					strcpy(text,"Anregung 266 nm\n ");
				}
				else if(rxMessage->excitation == 2){
					strcpy(text,"Anregung 355 nm\n ");
				}
				else if(rxMessage->excitation ==3){
					strcpy(text,"Anregung 405 nm\n ");
				}

				strcat(text,rxMessage->name);
			}
			else if(rxMessage->cmd == pause){
				strcpy(text,"paused: ");
				strcat(text,rxMessage->name);
			}
			else{
				strcpy(text,"Push play to start");
			}
		}

		if(getGuiIsActive()){
			updatePollenLabel(text);
		}

		LV_Task(); /* call this every 1-20 ms */
#if PL_CONFIG_HAS_KEYS
		KEY1_ScanKeys();
#endif
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void GUI_Init(void) {
	LV_Init(); /* initialize GUI library */
	//lv_theme_set_current(lv_theme_night_init(128, NULL));
  //lv_theme_set_current(lv_theme_alien_init(128, NULL));
  //lv_theme_set_current(lv_theme_default_init(128, NULL));
  //lv_theme_set_current(lv_theme_material_init(128, NULL));
  //lv_theme_set_current(lv_theme_mono_init(128, NULL));
  //lv_theme_set_current(lv_theme_zen_init(128, NULL));
  //lv_theme_set_current(lv_theme_nemo_init(128, NULL));

	//lv_theme_t *th = lv_theme_get_current();
	/* change default button style */
 // lv_style_btn_rel.body.radius = LV_DPI / 15;
 // lv_style_btn_rel.body.padding.hor = LV_DPI / 8;
 // lv_style_btn_rel.body.padding.ver = LV_DPI / 12;

  if (xTaskCreate(GuiTask, "Gui", 2000/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
    for(;;){} /* error */
  }
#if PL_CONFIG_HAS_GUI_KEY_NAV
  groups.sp = 0;
#endif
}
#endif /* PL_CONFIG_HAS_GUI */
