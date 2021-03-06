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
#include "Message.h"

uint8_t Config_ReadIni(const CLS1_StdIOType *io);
uint8_t CONFIG_ParseCommand(const unsigned char *cmd, bool *handled,
		const CLS1_StdIOType *io);
uint8_t Config_ReadPollen(void);
uint8_t Config_ReadImages(void);
uint8_t Config_Setup(void);
char** getNamelist(void);

char** getImagesList(void);
int getQuantityOfImages(void);

void setPowerSupplyCurrent(uint32_t val);
uint8_t getCurrentPerPixel(void);
uint32_t getPowerSupplyCurrent(void);
uint32_t getLCDTurnOffTime(void);
uint32_t getScreensaverTime(void);
int getQuantity(void);
void initConfigData(void);
bool getSensorEnabled(void);
void setSensorEnabled(bool enabled);

uint8_t getMaxNoOfPollen(void);

uint32_t getSequenzColor(uint8_t sequenz, uint8_t pos);

uint8_t getLetterEnabled(char letter);
uint32_t getTrailSpeed(void);
void setPowerConnected(uint8_t);
uint8_t getPowerConnected(void);
bool lightSensor;

uint8_t Config_ReadIni_Local(const CLS1_StdIOType *io);




#define nDataPoints 5
#define nOfExcitation 3


/*
 * Function to get the Timing Setup
 *
 * Return :
 * pos 0 --> timingDelayBetweenSeq1_2;
 * pos 1 --> timingDisplaySeq2
 * pos 2 --> timingDelayBetweenSeq2_3
 * pos 3 --> timingDelayBetweenSeq3_1
 *
 * */
uint32_t getTiming(uint8_t pos);

uint8_t Config_StorePollen(struct DataMessage * ptr);


#endif /* SOURCES_CONFIG_H_ */
