/*
 * Message.c
 *
 *  Created on: 25.11.2018
 *      Author: Pescatore
 */
#include "Message.h"


BaseType_t res;



QUEUE_RESULT AddMessageToQueue(xQueueHandle handle,Message_t * msg){


	QUEUE_RESULT res = QUEUE_OK;
	CS1_CriticalVariable();
	CS1_EnterCritical();
	if(FRTOS1_xQueueSendToBack(handle,(void *)&msg,0)!=pdPASS){
		res = QUEUE_IS_FULL;
	}
	CS1_ExitCritical();
	return res;

}


QUEUE_RESULT TakeMessageFromQueue(xQueueHandle handle,Message_t * msg){

	QUEUE_RESULT res = QUEUE_OK;
	CS1_CriticalVariable();
	CS1_EnterCritical();
	if(FRTOS1_xQueueReceive(handle,(void *)&msg,( TickType_t ) 10 )!=pdPASS){
		res = QUEUE_EMPTY;
	}
	CS1_ExitCritical();
	return res;

}
