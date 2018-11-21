/*
 * NeoConfig.h
 *
 *  Created on: 07.07.2014
 *      Author: Erich Styger
 */

#ifndef NEOCONFIG_H_
#define NEOCONFIG_H_

#include "PE_Types.h"
#include "Platform.h"

#define NEOC_NOF_LANES         			(1)   /* number of data lanes. For a matrix it is assumed that the number of pixels are divided to the available lanes! */
#define NEOC_NOF_LEDS_IN_LANE  			(64) /* number of LEDs in each lane */
#define NEOC_USE_NOF_COLOR     			(3)   /* number of colors, either 3 (RGB) or 4 (RGBW) Neopixels */

#define NEOC_NOF_PIXEL   				((NEOC_NOF_LANES)*(NEOC_NOF_LEDS_IN_LANE)) /* number of pixels */
#define NEOC_MAX_COLOR_VALUE			((NEOC_NOF_PIXEL)*(255)*(3))		/* total possible color value*/
#define NEOC_MAX_CONS_PER_LED			(0x32)	/* 50mA per LED*/
#define NEOC_MAX_CURRENT_CONS			((NEOC_NOF_PIXEL) * (NEOC_MAX_CONS_PER_LED))	/* total amount of consumption */
#define NEOC_MAX_COLOR_VALUE_ALLOWED 	((PL_CONFIG_MAX_CURRENT_SUPPLY)/(NEOC_MAX_CURRENT_CONS)*(NEOC_MAX_COLOR_VALUE))	/*max. color value that can be diplayed at once -- use for dimm-calculation*/

#endif /* NEOCONFIG_H_ */
