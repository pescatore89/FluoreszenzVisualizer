/*
 * Message.c
 *
 *  Created on: 25.11.2018
 *      Author: Pescatore
 */
#include "Message.h"


BaseType_t res;


QUEUE_RESULT AddMessageToPlaylistQueue(xQueueHandle handle,PlaylistMessage_t *msg){

	QUEUE_RESULT res = QUEUE_OK;
	if(FRTOS1_xQueueSendToBack(handle,(void *)&msg,0)!=pdPASS){
		res = QUEUE_IS_FULL;
	}
	return res;
}


QUEUE_RESULT TakeMessageFromPlaylistQueue(xQueueHandle handle,PlaylistMessage_t *msg){

	QUEUE_RESULT res = QUEUE_OK;
	if(FRTOS1_xQueueReceive(handle,(void *)&msg,0 )!=pdPASS){
		res = QUEUE_EMPTY;
	}
	else{
		res = QUEUE_HAS_ITEM;
	}
	return res;
}




QUEUE_RESULT AddMessageToDataQueue(xQueueHandle handle,DataMessage_t *msg){

	QUEUE_RESULT res = QUEUE_OK;

	if(FRTOS1_xQueueSendToBack(handle,(void *)&msg,0)!=pdPASS){
		res = QUEUE_IS_FULL;
	}
	return res;
}
QUEUE_RESULT TakeMessageFromDataQueue(xQueueHandle handle,DataMessage_t *msg){
	QUEUE_RESULT res = QUEUE_OK;

	if(FRTOS1_xQueueReceive(handle,(void *)&msg,0 )!=pdPASS){
		res = QUEUE_EMPTY;
	}
	else{
		res = QUEUE_HAS_ITEM;
	}

	return res;

}

QUEUE_RESULT PeekDataQueue(xQueueHandle handle,DataMessage_t *msg){
	QUEUE_RESULT res = QUEUE_OK;

	if( xQueuePeek( handle, (void *)&msg, ( TickType_t ) 10 ) != pdPASS ){
		res = QUEUE_EMPTY;
	}
	else {
		res = QUEUE_HAS_ITEM;
	}
	return res;



}



QUEUE_RESULT AddMessageToUpdateQueue(xQueueHandle handle,UpdateMessage_t *msg){
	QUEUE_RESULT res = QUEUE_OK;

	if(FRTOS1_xQueueSendToBack(handle,(void *)&msg,0)!=pdPASS){
		res = QUEUE_IS_FULL;
	}

	return res;

}
QUEUE_RESULT TakeMessageFromUpdateQueue(xQueueHandle handle,UpdateMessage_t *msg){
	QUEUE_RESULT res = QUEUE_OK;

	if(FRTOS1_xQueueReceive(handle,(void *)&msg,0 )!=pdPASS){
		res = QUEUE_EMPTY;
	}
	else{
		res = QUEUE_HAS_ITEM;
	}

	return res;

}










#if 0

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
	if(FRTOS1_xQueueReceive(handle,(void *)&msg,0 )!=pdPASS){
		res = QUEUE_EMPTY;
	}
	CS1_ExitCritical();
	return res;

}



QUEUE_RESULT AddNavigationToQueue(xQueueHandle handle,Navigation_t *msg){


	QUEUE_RESULT res = QUEUE_OK;
	CS1_CriticalVariable();
	CS1_EnterCritical();
	if(FRTOS1_xQueueSendToBack(handle,(void *)&msg,0)!=pdPASS){
		res = QUEUE_IS_FULL;
	}
	CS1_ExitCritical();
	return res;

}


QUEUE_RESULT TakeNavigationFromQueue(xQueueHandle handle,Navigation_t *msg){

	QUEUE_RESULT res = QUEUE_OK;
	CS1_CriticalVariable();
	CS1_EnterCritical();
	if(FRTOS1_xQueueReceive(handle,(void *)&msg,0 )!=pdPASS){
		res = QUEUE_EMPTY;
	}
	else{
		int z = 12;
	}
	CS1_ExitCritical();
	return res;

}

#endif
