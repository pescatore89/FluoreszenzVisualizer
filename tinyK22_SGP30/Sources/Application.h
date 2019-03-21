/*
 * Application.h
 *
 *  Created on: 30.09.2018
 *      Author: Erich Styger
 */

#ifndef SOURCES_APPLICATION_H_
#define SOURCES_APPLICATION_H_
#include "FRTOS1.h"

void APP_Run(void);
uint8_t createQueues(void);
void resetLCD_Counter(void);

#endif /* SOURCES_APPLICATION_H_ */
