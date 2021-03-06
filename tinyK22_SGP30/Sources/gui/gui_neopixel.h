/*
 * gui_air.h
 *
 *  Created on: 06.08.2018
 *      Author: Erich Styger
 */

#ifndef GUI_NEO_H_
#define GUI_NEO_H_

#include <stdint.h>

void NEO_GUI_SetLightLevel(uint8_t val);

void GUI_NEO_Create(uint8_t* name);
void updatePollenLabel(char* text);

void updatePlayBtn(uint8_t val);
void setNavGuiIsActive(uint8_t);
uint8_t getNavGuiIsActive(void);
#endif /* GUI_AIR_H_ */
