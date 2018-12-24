/*
 * Message.h
 *
 *  Created on: 25.11.2018
 *      Author: Pescatore
 */

#ifndef SOURCES_MESSAGE_H_
#define SOURCES_MESSAGE_H_

#define QUEUE_PLAYLIST_LENGTH			(100)
#define QUEUE_UPDATE_LENGTH				(100)
#define QUEUE_DATA_LENGTH				(100)
#define QUEUE_ITEM_PLAYLIST_SIZE		(100)
#define QUEUE_ITEM_UPDATE_SIZE			(100)
#define QUEUE_ITEM_DATA_SIZE			(100)

#include "FRTOS1.h"

extern xQueueHandle queue_handler;


extern xQueueHandle queue_handler_playlist; /*Queue handler for Playlist Queue*/
extern xQueueHandle queue_handler_data; /*Queue handler for data Queue*/
extern xQueueHandle queue_handler_update; /*Queue handler for update Queue*/




/* Image type - contains height, width, and data */
struct BMPImage {


    long biWidth;  					// [18] specifies width in pixels
    long biHeight;  				// [22] species height in pixels
    									/*      Ist der Wert positiv, so ist die Bitmap eine sogenannte "bottom-up"-Bitmap (die Bilddaten beginnen mit der untersten und enden mit der obersten Bildzeile). Dies ist die gebräuchlichste Variante.
        										Ist der Wert negativ, so ist die Bitmap eine "top-down"-Bitmap (die Bilddaten beginnen mit der obersten und enden mit der untersten Bildzeile).*/
    unsigned int bfOffBits;			// [10] Offset der Bilddaten in Byte vom Beginn der Datei an.
    unsigned short biBitCount; 		// [28] specifies the number of bit per pixel
    unsigned long biSizeImage;  	// [34] size of image in bytes


  //  char *data;						// Farbwerte

}xBMPImage;
typedef struct BMPImage BMPImage;



/* ******************************************************************************************
 *
 * Message for the Playlist Queue
 *
 * ******************************************************************************************/

typedef enum {
	play,
	pause,
	stop,
	skipF,
	skipR
}COMMAND;

typedef enum {
	newData,		/*Update Playlist*/
	newCMD,			/*updated Commando*/
	newImage		/*Display an Image*/
} STATE;


struct PlaylistMessage {
	COMMAND cmd;
	uint8_t* playlist;
	STATE state;
	BMPImage* image;
} xPlaylistMessage;

typedef struct PlaylistMessage PlaylistMessage_t;

typedef enum {
	QUEUE_OK = 0, /* (0) Succeeded */
	QUEUE_ERR, /* (1)Error  */
	QUEUE_EMPTY, /* (2)Queue is empty */
	QUEUE_HAS_ITEM, /* (3)Queue is empty */
	QUEUE_ERROR_IN, /* (4)Error adding Item to queue*/
	QUEUE_ERROR_OUT, /* (5)Error taking Item out */
	QUEUE_INVALID_PARAMETER, /* (6)Given parameter is invalid */
	QUEUE_IS_FULL /* (7)Queue is full */
} QUEUE_RESULT;

QUEUE_RESULT AddMessageToPlaylistQueue(xQueueHandle handle,PlaylistMessage_t *msg);
QUEUE_RESULT TakeMessageFromPlaylistQueue(xQueueHandle handle,PlaylistMessage_t *msg);





/* ******************************************************************************************
 *
 * Message for the Data Queue
 *
 * ******************************************************************************************/

struct DATA {
	/*Data for seq. 1*/

	uint32_t color_266;
	uint32_t color_355;
	uint32_t color_405;
	uint32_t fadeout_266; /*in ms*/
	uint32_t fadeout_355; /*in ms*/
	uint32_t fadeout_405; /*in ms*/

	/*Data for seq. 2*/
	uint8_t amplitude_266_1;
	uint8_t amplitude_266_2;
	uint8_t amplitude_266_3;
	uint8_t amplitude_266_4;
	uint8_t amplitude_266_5;

	uint8_t amplitude_355_1;
	uint8_t amplitude_355_2;
	uint8_t amplitude_355_3;
	uint8_t amplitude_355_4;
	uint8_t amplitude_355_5;

	uint8_t amplitude_405_1;
	uint8_t amplitude_405_2;
	uint8_t amplitude_405_3;
	uint8_t amplitude_405_4;
	uint8_t amplitude_405_5;

	/*Data for seq. 3*/
	uint32_t lifetime_266_1;
	uint32_t lifetime_266_2;
	uint32_t lifetime_266_3;
	uint32_t lifetime_266_4;

	uint32_t lifetime_355_1;
	uint32_t lifetime_355_2;
	uint32_t lifetime_355_3;
	uint32_t lifetime_355_4;

	uint32_t lifetime_405_1;
	uint32_t lifetime_405_2;
	uint32_t lifetime_405_3;
	uint32_t lifetime_405_4;
}xDATA;



typedef struct DATA DATA_t;


struct DataMessage {
	DATA_t* char_data;
	char* color_data;
	char* name;
	uint8_t farbtiefe;
	COMMAND cmd;
	BMPImage* image;
	uint8_t excitation;
}xDataMessage;

typedef struct DataMessage DataMessage_t;



QUEUE_RESULT AddMessageToDataQueue(xQueueHandle handle,DataMessage_t *msg);
QUEUE_RESULT TakeMessageFromDataQueue(xQueueHandle handle,DataMessage_t *msg);
QUEUE_RESULT PeekDataQueue(xQueueHandle handle,DataMessage_t *msg);




/* ******************************************************************************************
 *
 * MEssage for the Update Queue
 *
 * ******************************************************************************************/




struct UpdateMessage {
	COMMAND cmd;
	char* name;
	uint8_t excitation;
}xUpdateMessage;


typedef struct UpdateMessage UpdateMessage_t;


QUEUE_RESULT AddMessageToUpdateQueue(xQueueHandle handle,UpdateMessage_t *msg);
QUEUE_RESULT TakeMessageFromUpdateQueue(xQueueHandle handle,UpdateMessage_t *msg);











extern xSemaphoreHandle mutex;
extern xQueueHandle queue_handler_Navigation;


struct MESSAGE {
	uint8_t* data;
	uint32_t color_266;
	uint32_t color_355;
	uint32_t color_405;

	uint32_t fadeout_266; /*in ms*/
	uint32_t fadeout_355; /*in ms*/
	uint32_t fadeout_405; /*in ms*/

	uint8_t amplitude_266_400;
	uint8_t amplitude_266_500;
	uint8_t amplitude_266_600;
	uint8_t amplitude_266_700;

	uint8_t amplitude_355_400;
	uint8_t amplitude_355_500;
	uint8_t amplitude_355_600;
	uint8_t amplitude_355_700;

	uint8_t amplitude_405_400;
	uint8_t amplitude_405_500;
	uint8_t amplitude_405_600;
	uint8_t amplitude_405_700;

	uint8_t lifetime_266_400;
	uint8_t lifetime_266_500;
	uint8_t lifetime_266_600;
	uint8_t lifetime_266_700;

	uint8_t lifetime_355_400;
	uint8_t lifetime_355_500;
	uint8_t lifetime_355_600;
	uint8_t lifetime_355_700;

	uint8_t lifetime_405_400;
	uint8_t lifetime_405_500;
	uint8_t lifetime_405_600;
	uint8_t lifetime_405_700;

	//uint16_t excitation; /*Anregungsintensität*/

} xMessage;

typedef struct Navigation Navigation_t;
typedef struct MESSAGE Message_t;

QUEUE_RESULT AddNavigationToQueue(xQueueHandle handle, Navigation_t *msg);
QUEUE_RESULT TakeNavigationFromQueue(xQueueHandle handle, Navigation_t *msg);

QUEUE_RESULT AddMessageToQueue(xQueueHandle handle, Message_t * msg);
QUEUE_RESULT TakeMessageFromQueue(xQueueHandle handle, Message_t * msg);
QUEUE_RESULT QueueHasElement(xQueueHandle handle);

#endif /* SOURCES_MESSAGE_H_ */
