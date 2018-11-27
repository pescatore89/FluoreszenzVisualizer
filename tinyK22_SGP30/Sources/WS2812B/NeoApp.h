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
#endif /* SOURCES_WS2812B_NEO_APP_C_ */
