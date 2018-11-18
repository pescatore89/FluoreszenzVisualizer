/*
 * gui_air.h
 *
 *  Created on: 06.08.2018
 *      Author: Erich Styger
 */

#ifndef GUI_NEO_H_
#define GUI_NEO_H_



#define N_OF_POLLS_HOLO				(3)
#define DISTANCE_BTW_BUTTON 		(0x16)
#define OFFSET_DISTANCE_BUTTON_ZERO	(0x19)
void GUI_NEO_Create(void);

enum themes{
    SWITZERLAND = 1,
    CIRCLE,
    SMILEY,
    NOF_THEMES,
};


#endif /* GUI_AIR_H_ */
