/*
 * config.h
 *
 *  Created on: 23.11.2018
 *      Author: Pescatore
 */

#ifndef SOURCES_CONFIG_H_
#define SOURCES_CONFIG_H_
/* Include shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "CLS1.h"

uint8_t Config_ReadIni(const CLS1_StdIOType *io);
uint8_t CONFIG_ParseCommand(const unsigned char *cmd, bool *handled,
		const CLS1_StdIOType *io);
uint8_t Config_ReadPollen(void);
uint8_t Config_Setup(void);
char** getNamelist(void);
int quantity;
int getQuantity(void);
void initConfigData(void);
bool getSensorEnabled(void);
void setSensorEnabled(bool enabled);
char** namelist;	// Platzhalter für die namen der Pollen
uint8_t powerEnabled;
bool lightSensor;
#endif /* SOURCES_CONFIG_H_ */
