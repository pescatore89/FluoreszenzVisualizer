/*
 * Player.h
 *
 *  Created on: 14.12.2018
 *      Author: Pescatore
 */

#ifndef SOURCES_PLAYER_H_
#define SOURCES_PLAYER_H_


void PLAYER_Init(void);
float powerRegulation(char* data);





typedef enum {
	IDLE = 0, /* */
	READ_NEW, /* read new Message from playlistQueue  */
	UPDATE_PLAYLIST, /*  */
	PLAY_LIST, /* */

} PLAYER_STATE;










#endif /* SOURCES_PLAYER_H_ */
