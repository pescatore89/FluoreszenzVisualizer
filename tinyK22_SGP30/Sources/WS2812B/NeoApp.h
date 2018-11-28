/*
 * Neo.h
 *
 *  Created on: 10.10.2018
 *      Author: Erich Styger
 */

#ifndef SOURCES_WS2812B_NEO_APP_C_
#define SOURCES_WS2812B_NEO_APP_C_

#define NEOA_CONFIG_PARSE_COMMAND_ENABLED  (1)

#if NEOA_CONFIG_PARSE_COMMAND_ENABLED
#include "CLS1.h"
uint8_t NEOA_ParseCommand(const unsigned char* cmd, bool *handled,
		const CLS1_StdIOType *io);
#endif
#include "..\my_types.h"
void NEOA_Init(void* queue_handler);
uint8_t NEOA_Lauflicht(void);
uint8_t NEOA_Display_Image(BMPImage* image);
uint8_t SetCoordinate(int x, int y, uint32_t color);
uint8_t SetTrail(uint32_t color,  uint32_t end, uint32_t nofTail, uint8_t dimmPercent, uint16_t delayMs);
uint8_t ClearCoordinate(int x, int y);
uint8_t DimmPercentPixel(int x, int y, uint8_t percent);

struct CYRCLES {
	uint8_t* C_1;
	uint8_t* C_2;
	uint8_t* C_3;
	uint8_t* C_4;
	uint8_t* C_5;
	uint8_t* C_6;
	uint8_t* C_7;
	uint8_t* C_8;
	uint8_t* C_9;
	uint8_t* C_10;
	uint8_t* C_11;
	uint8_t* C_12;
};
typedef struct CYRCLES Cyrcles_t;



uint8_t DimmPercentRing(uint8_t ring, uint32_t percent);
uint8_t setRingData(uint8_t ring,uint32_t color);
#endif /* SOURCES_WS2812B_NEO_APP_C_ */
