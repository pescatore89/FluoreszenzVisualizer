/*
 * NeoConfig.h
 *
 *  Created on: 07.07.2014
 *      Author: Erich Styger
 */

#ifndef NEOCONFIG_H_
#define NEOCONFIG_H_

#include "PE_Types.h"

#define NEOC_NOF_LANES         (3)   /* number of data lanes. For a matrix it is assumed that the number of pixels are divided to the available lanes! */
#define NEOC_NOF_LEDS_IN_LANE  (192) /* number of LEDs in each lane */
#define NEOC_USE_NOF_COLOR     (3)   /* number of colors, either 3 (RGB) or 4 (RGBW) Neopixels */

#define NEOC_NOF_PIXEL   ((NEOC_NOF_LANES)*(NEOC_NOF_LEDS_IN_LANE)) /* number of pixels */
#define NEO_IS_MATRIX		(1)
#define RESOLUTION 		((NEOC_NOF_LANES)*(NEOC_NOF_LEDS_IN_LANE))		/* Auflösung des NeoPixel Bildes */
#define MATRIX_RES		(24)										/* 24 * 24 Auflösung */
#define SINGLE_MATRIX_SIDE_LENGTH 	(8)


#endif /* NEOCONFIG_H_ */
