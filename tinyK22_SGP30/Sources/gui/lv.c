/*
 * lv.c
 *
 *  Created on: 02.08.2018
 *      Author: Erich Styger
 */
#include "Platform.h"
#if PL_CONFIG_HAS_GUI
#include "lv.h"
#include "lvgl/lvgl.h"
#include "GDisp1.h"
#include "FDisp1.h"
#include "RNG1.h"
#include <string.h> /* for memset() */
#include "CLS1.h"
#include "Application.h"


static lv_indev_t *inputDevicePtr;

lv_indev_t * LV_GetInputDevice(void) {
  return inputDevicePtr;
}

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_flush_ready()' has to be called when finished
 * This function is required only when LV_VDB_SIZE != 0 in lv_conf.h*/
static void ex_disp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p) {
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
    int32_t x;
    int32_t y;
    for(y = y1; y <= y2; y++) {
        for(x = x1; x <= x2; x++) {
            /* Put a pixel to the display. */
          GDisp1_PutPixel(x, y, color_p->full);
          color_p++;
        }
    }
    GDisp1_UpdateRegion(x1, y1, x2-x1+1, y2-y1+1);
    /* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing*/
    lv_flush_ready();
}

/* Write a pixel array (called 'map') to the a specific area on the display
 * This function is required only when LV_VDB_SIZE == 0 in lv_conf.h*/
static void ex_disp_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p) {
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
    int32_t x;
    int32_t y;

    for(y = y1; y <= y2; y++) {
        for(x = x1; x <= x2; x++) {
            /* Put a pixel to the display.*/
            GDisp1_PutPixel(x, y, color_p->full);
            color_p++;
        }
    }
    GDisp1_UpdateRegion(x1, y1, x2-x1+1, y2-y1+1);
}

/* Write a pixel array (called 'map') to the a specific area on the display
 * This function is required only when LV_VDB_SIZE == 0 in lv_conf.h*/
static void ex_disp_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2,  lv_color_t color) {
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
    int32_t x;
    int32_t y;

    for(y = y1; y <= y2; y++) {
        for(x = x1; x <= x2; x++) {
          /* Put a pixel to the display.*/
          GDisp1_PutPixel(x, y, color.full);
        }
    }
    GDisp1_UpdateRegion(x1, y1, x2-x1+1, y2-y1+1);
}

#if USE_LV_GPU

/* If your MCU has hardware accelerator (GPU) then you can use it to blend to memories using opacity
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void ex_mem_blend(lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
{
    /*It's an example code which should be done by your GPU*/

    int32_t i;
    for(i = 0; i < length; i++) {
        dest[i] = lv_color_mix(dest[i], src[i], opa);
    }
}

/* If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void ex_mem_fill(lv_color_t * dest, uint32_t length, lv_color_t color)
{
    /*It's an example code which should be done by your GPU*/

    int32_t i;
    for(i = 0; i < length; i++) {
        dest[i] = color;
    }
}

#endif

#if 0
/* Read the touchpad and store it in 'data'
 * Return false if no more data read; true for ready again */
static bool ex_tp_read(lv_indev_data_t * data) {
    /* Read the touchpad */
	int x=0, y=0, res;
	bool pressed;

	res = TOUCH_Poll(&pressed, &x, &y);
	if (res==1 && pressed) {
		data->state = LV_INDEV_STATE_PR;
	} else {
		data->state = LV_INDEV_STATE_REL;
	}
	data->point.x = x;
	data->point.y = y;
    return false;   /*false: no more data to read because we are no buffering*/
}
#endif

void LV_Task(void) {
  /* Periodically call this function.
   * The timing is not critical but should be between 1..10 ms */
  lv_task_handler();
}

/* called for push button events from Events.c */
void LV_ButtonEvent(uint8_t keys, uint16_t eventMask) {
  uint16_t buttonInfo;






  if (keys&(1<<0)) {
    buttonInfo = LV_BUTTON_CENTER | eventMask;
    RNG1_Put(buttonInfo);
  } else if (keys&(1<<1)) {
    buttonInfo = LV_BUTTON_RIGHT | eventMask;
    RNG1_Put(buttonInfo);
  } else if (keys&(1<<2)) {
    buttonInfo = LV_BUTTON_DOWN | eventMask;
    RNG1_Put(buttonInfo);
  } else if (keys&(1<<3)) {
    buttonInfo = LV_BUTTON_UP | eventMask;
    RNG1_Put(buttonInfo);
  } else if (keys&(1<<4)) {
    buttonInfo = LV_BUTTON_LEFT | eventMask;
    RNG1_Put(buttonInfo);
   }
}

/*
 * To use a keyboard:
    USE_LV_GROUP has to be enabled in lv_conf.h
    An object group has to be created: lv_group_create() and objects have to be added: lv_group_add_obj()
    The created group has to be assigned to an input device: lv_indev_set_group(my_indev, group1);
    Use LV_GROUP_KEY_... to navigate among the objects in the group
 */
static bool keyboard_read(lv_indev_data_t *data)  {
  uint16_t keyData;

  memset(data, 0, sizeof(lv_indev_data_t)); /* initialize all fields */
  data->state = LV_INDEV_STATE_REL; /* by default, not pressed */
  if (RNG1_NofElements()==0) {
    return false; /* no data present */
  }
  if (RNG1_Get(&keyData)!=ERR_OK) {
    return false; /* we had data in the buffer, but now not anymore? something went wrong! */
  }
  if (keyData&(LV_MASK_PRESSED|LV_MASK_PRESSED_LONG)) {
    data->state = LV_INDEV_STATE_PR;
  } else if (keyData&(LV_MASK_RELEASED|LV_MASK_RELEASED_LONG)) {
    data->state = LV_INDEV_STATE_REL;
  }
  switch(keyData&0xff) {
  case LV_BUTTON_LEFT:
    data->key = LV_GROUP_KEY_LEFT;
    break;
  case LV_BUTTON_RIGHT:
    data->key = LV_GROUP_KEY_RIGHT;
    break;
  case LV_BUTTON_UP:
    data->key = LV_GROUP_KEY_UP;
    break;
  case LV_BUTTON_DOWN:
    data->key = LV_GROUP_KEY_DOWN;
    break;
  case LV_BUTTON_CENTER:
    data->key = LV_GROUP_KEY_ENTER;
    break;
 // case LV_BUTTON_NEXT:
 //   data->key = LV_GROUP_KEY_NEXT;
 //   break;
  default:
    return false; /* error case? */
  }
  return RNG1_NofElements()!=0;   /* return true if we have more data */
}

static bool encoder_read(lv_indev_data_t *data){
  uint16_t keyData;

  memset(data, 0, sizeof(lv_indev_data_t)); /* initialize all fields */
  data->state = LV_INDEV_STATE_REL; /* by default, not pressed */
  if (RNG1_NofElements()==0) {
    return false; /* no data present */
  }
  if (RNG1_Get(&keyData)!=ERR_OK) {
    return false; /* we had data in the buffer, but now not anymore? something went wrong! */
  }
  data->state = LV_INDEV_STATE_REL;
  switch(keyData&0xff) {
  case LV_BUTTON_LEFT:
    if (keyData&(LV_MASK_PRESSED)) {
      data->enc_diff = -1;
      CLS1_SendStr("left pressed\r\n", CLS1_GetStdio()->stdOut);
    } else {
      data->enc_diff = 0;
      CLS1_SendStr("left released\r\n", CLS1_GetStdio()->stdOut);
    }
    break;
  case LV_BUTTON_RIGHT:
    if (keyData&(LV_MASK_PRESSED)) {
      data->enc_diff = 1;
      CLS1_SendStr("right pressed\r\n", CLS1_GetStdio()->stdOut);
    } else {
      data->enc_diff = 0;
      CLS1_SendStr("right released\r\n", CLS1_GetStdio()->stdOut);
    }
    break;
  case LV_BUTTON_UP:
    if (keyData&(LV_MASK_PRESSED)) {
      data->enc_diff = -1;
      CLS1_SendStr("up pressed\r\n", CLS1_GetStdio()->stdOut);
    } else {
      data->enc_diff = 0;
      CLS1_SendStr("up released\r\n", CLS1_GetStdio()->stdOut);
    }
    break;
  case LV_BUTTON_DOWN:
    if (keyData&(LV_MASK_PRESSED)) {
      data->enc_diff = 1;
      CLS1_SendStr("down pressed\r\n", CLS1_GetStdio()->stdOut);
    } else {
      data->enc_diff = 0;
      CLS1_SendStr("down released\r\n", CLS1_GetStdio()->stdOut);
    }
    break;
  case LV_BUTTON_CENTER:
    if (keyData&(LV_MASK_PRESSED)) {
      data->state = LV_INDEV_STATE_REL;
      data->enc_diff = 0;
      CLS1_SendStr("center pressed\r\n", CLS1_GetStdio()->stdOut);
    } else if (keyData&(LV_MASK_PRESSED_LONG)) {
      data->state = LV_INDEV_STATE_PR;
      data->enc_diff = 0;
      CLS1_SendStr("center long pressed\r\n", CLS1_GetStdio()->stdOut);
    } else {
      data->state = LV_INDEV_STATE_REL;
      data->enc_diff = 0;
      CLS1_SendStr("center released\r\n", CLS1_GetStdio()->stdOut);
    }
    break;
#if 0
  case LV_BUTTON_NEXT:
    if (keyData&(LV_MASK_PRESSED)) {
      data->state = LV_INDEV_STATE_REL;
      data->enc_diff = 1;
    } else if (keyData&(LV_MASK_PRESSED_LONG)) {
      data->state = LV_INDEV_STATE_PR;
      data->enc_diff = 0;
    } else {
      data->state = LV_INDEV_STATE_REL;
      data->enc_diff = 0;
    }
    break;
  case LV_BUTTON_PREV:
    if (keyData&(LV_MASK_PRESSED_LONG)) {
      data->state = LV_INDEV_STATE_PR;
      data->enc_diff = 0;
    } else {
      data->state = LV_INDEV_STATE_REL;
      data->enc_diff = -1;
    }
    break;
#endif
  default:
    return false; /* error case? */
  } /* switch */
  return RNG1_NofElements()!=0;   /* return true if we have more data */
}

void LV_Init(void) {
  lv_disp_drv_t disp_drv;

  lv_init();
  lv_disp_drv_init(&disp_drv);
  /*Set up the functions to access to your display*/
  disp_drv.disp_flush = ex_disp_flush;            /*Used in buffered mode (LV_VDB_SIZE != 0  in lv_conf.h)*/
  disp_drv.disp_fill = ex_disp_fill;              /*Used in unbuffered mode (LV_VDB_SIZE == 0  in lv_conf.h)*/
  disp_drv.disp_map = ex_disp_map;                /*Used in unbuffered mode (LV_VDB_SIZE == 0  in lv_conf.h)*/

#if USE_LV_GPU
  /*Optionally add functions to access the GPU. (Only in buffered mode, LV_VDB_SIZE != 0)*/
  disp_drv.mem_blend = ex_mem_blend;              /*Blend two color array using opacity*/
  disp_drv.mem_fill = ex_mem_fill;                /*Fill a memory array with a color*/
#endif

  /*Finally register the driver*/
  lv_disp_drv_register(&disp_drv);

  /*************************
   * Input device interface
   *************************/
  lv_indev_drv_t indev_drv;                       /*Descriptor of an input device driver*/
  lv_indev_drv_init(&indev_drv);                  /*Basic initialization*/
#if 0 /* touch pad */
  indev_drv.type = LV_INDEV_TYPE_POINTER;         /*The touchpad is pointer type device*/
  indev_drv.read = ex_tp_read;                 /*Library ready your touchpad via this function*/
#elif 1
  indev_drv.type = LV_INDEV_TYPE_ENCODER;
  indev_drv.read = encoder_read;
#else /* keyboard input */
  indev_drv.type = LV_INDEV_TYPE_KEYPAD;
  indev_drv.read = keyboard_read;
#endif
  inputDevicePtr = lv_indev_drv_register(&indev_drv);              /*Finally register the driver*/
}

#endif /* PL_CONFIG_HAS_GUI */
