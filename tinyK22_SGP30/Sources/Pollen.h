/*
 * Pollen.h
 *
 *  Created on: 25.11.2018
 *      Author: Pescatore
 */

#ifndef SOURCES_POLLEN_H_
#define SOURCES_POLLEN_H_

#include "PE_Types.h"
#include "CLS1.h"
#include "Message.h"

uint8_t POLLEN_ParseCommand(const unsigned char* cmd, bool *handled,
		const CLS1_StdIOType *io);

uint8_t SetMode(int32_t mode, char* polle, const CLS1_StdIOType *io );
uint8_t SetNav(int32_t nav);

#endif /* SOURCES_POLLEN_H_ */
