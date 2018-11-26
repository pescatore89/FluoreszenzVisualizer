/*
 * Message.h
 *
 *  Created on: 25.11.2018
 *      Author: Pescatore
 */

#ifndef SOURCES_MESSAGE_H_
#define SOURCES_MESSAGE_H_



#define QUEUE_LENGTH	(100)
#define QUEUE_ITEM_SIZE	(1)

#include "FRTOS1.h"


extern xQueueHandle queue_handler;
extern xSemaphoreHandle mutex;



typedef enum {
	QUEUE_OK = 0,				/* (0) Succeeded */
	QUEUE_ERR,					/* (1)Error  */
	QUEUE_EMPTY,				/* (2)Queue is empty */
	QUEUE_HAS_ITEM,				/* (3)Queue is empty */
	QUEUE_ERROR_IN,				/* (4)Error adding Item to queue*/
	QUEUE_ERROR_OUT,			/* (5)Error taking Item out */
	QUEUE_INVALID_PARAMETER,	/* (6)Given parameter is invalid */
	QUEUE_IS_FULL				/* (7)Given parameter is invalid */
} QUEUE_RESULT;

typedef enum {
	SINGLE = 1,				/* (1) Einzelnes Bild darstellen */
	MODE1,					/* (2) Modus 1 */
	MODE2,					/* (3) Modus 2 */
	MODE3					/* (4) Modus 3*/

} MODE;


typedef enum{
	pause,
	play,
	stop,
	skip_backward,
	skip_forward

}xNavigation;


  struct MESSAGE{
	MODE 	 	 modus;
	uint8_t* 	 data;
	uint32_t 	 color;
	uint32_t 	 fadeoutTime;			/*in ms*/
	uint16_t 	 excitation;			/*Anregungsintensität*/

}xMessage;

typedef struct MESSAGE Message_t;





QUEUE_RESULT AddMessageToQueue(xQueueHandle handle,Message_t * msg);
QUEUE_RESULT TakeMessageFromQueue(xQueueHandle handle,Message_t * msg);
QUEUE_RESULT QueueHasElement(xQueueHandle handle);

#endif /* SOURCES_MESSAGE_H_ */
