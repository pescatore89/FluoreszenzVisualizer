/*
 * Neo.c
 *
 *  Created on: 10.10.2018
 *      Author: Erich Styger
 */

#include "Platform.h"
#include <stdlib.h>     // Header file for malloc/free.
#if PL_CONFIG_HAS_NEO_PIXEL
#include "NeoApp.h"
#include "NeoPixel.h"
#include "NeoLine.h"
#include "FRTOS1.h"
#include "LED1.h"
#include "PixelDMA.h"
#include "config.h"
#if PL_CONFIG_HAS_MMA8451
#include "MMA1.h"
#endif
#if PL_CONFIG_HAS_TSL2561
#include "TSL1.h"
#endif
#if PL_CONFIG_HAS_AMG8833
#include "AMG8833.h"
#include "interpolation.h"
#endif
#if PL_CONFIG_HAS_SSD1351
#include "GDisp1.h"
#include "LCD1.h"
#include "ff.h"
#include "..\Message.h"
#include "readSD.h"
#include <math.h>

#endif

#define DELAY_MS 1
#define nRINGS	12
#define NEO_PROCESSING_TIME			5		/*it takes about 5ms to transmit all the pixelValues in a lane*/
#define STARTING_DEGRADATION		3
#define WIDTH_PIXELS (3*8)  /* 3 8x8 tiles */
#define HEIGHT_PIXELS (8)   /* 1 tile */
#define PIXEL_NOF_X   (24)
#define PIXEL_NOF_Y   (8)

static uint8_t NEOA_LightLevel = 1; /* default 1% */
static bool NEOA_isAutoLightLevel = TRUE;
static bool NEOA_useGammaCorrection = TRUE;

static uint8_t WL_pixels[nDataPoints];

typedef struct {
	uint8_t decrementArray[nDataPoints];
	uint8_t counterArray[nDataPoints];
	uint8_t nPixelsArray[nDataPoints];
	uint32_t colorArray[nDataPoints];
	uint8_t coordinateArray[nDataPoints][2];
} seq3;

typedef enum {
	IDLE_STATE = 0, /* */
	READ_NEW_CMD, /* read new Message from playlistQueue  */
	PLAY_SEQ1, /*  */
	PLAY_SEQ2, /* */
	PLAY_SEQ3, /* */
	ERROR_STATE, STOPPED, DISPLAY_IMAGE, SCREENSAVER,

} NEO_STATUS;

typedef enum {
	notAborted, stopAborted, playImageAborted,
} RETURN_STATUS;

xQueueHandle queue_handler_data; /*Queue handler for data Queue*/
xQueueHandle queue_handler_update; /*Queue handler for update Queue*/

xSemaphoreHandle mutex; /*SemaphoreHandler declared in Message*/
//extern uint8_t ImageDataBuffer[2500];
void NEOA_SetLightLevel(uint8_t level) {
	NEOA_LightLevel = level;
}

uint8_t NEOA_GetLightLevel(void) {
	return NEOA_LightLevel;
}

bool NEOA_GetAutoLightLevelSetting(void) {
	return NEOA_isAutoLightLevel;
}

bool NEOA_SetAutoLightLevelSetting(bool set) {
	NEOA_isAutoLightLevel = set;
}
static void SetPixel(int x, int y, uint32_t color) {
	/* 0, 0 is left upper corner */
	/* single lane, 3x64 modules from left to right */
	int pos;

	pos = ((x / 8) * 64) /* position in tile */
	+ (x % 8) /* position in row */
	+ (y) * 8; /* add y offset */
	NEO_SetPixelColor(0, pos, color);
}

static uint32_t lane12[] = { 4, 12, 20, 28, 36, 44, 52, 60, 68, 76, 84, 92, 100,
		108, 116, 124, 132, 140, 148, 156, 164, 172, 180, 188 };
static uint32_t lane13[] = { 3, 11, 19, 27, 35, 43, 51, 59, 67, 75, 83, 91, 99,
		107, 115, 123, 131, 139, 147, 155, 163, 171, 179, 187 };

#if MATRIX_RES == 24

uint32_t lookUpMatrix[8][24] = { /*Look up Matrix für die Lanes*/
{ 191, 183, 175, 167, 159, 151, 143, 135, 127, 119, 111, 103, 95, 87, 79, 71,
		63, 55, 47, 39, 31, 23, 15, 7 }, /*Reihe 1*/
{ 190, 182, 174, 166, 158, 150, 142, 134, 126, 118, 110, 102, 94, 86, 78, 70,
		62, 54, 46, 38, 30, 22, 14, 6 }, /*Reihe 2*/
{ 189, 181, 173, 165, 157, 149, 141, 133, 125, 117, 109, 101, 93, 85, 77, 69,
		61, 53, 45, 37, 29, 21, 13, 5 }, /*Reihe 3*/
{ 188, 180, 172, 164, 156, 148, 140, 132, 124, 116, 108, 100, 92, 84, 76, 68,
		60, 52, 44, 36, 28, 20, 12, 4 }, /*Reihe 4*/
{ 187, 179, 171, 163, 155, 147, 139, 131, 123, 115, 107, 99, 91, 83, 75, 67, 59,
		51, 43, 35, 27, 19, 11, 3 }, /*Reihe 5*/
{ 186, 178, 170, 162, 154, 146, 138, 130, 122, 114, 106, 98, 90, 82, 74, 66, 58,
		50, 42, 34, 26, 18, 10, 2 }, /*Reihe 6*/
{ 185, 177, 169, 161, 153, 145, 137, 129, 121, 113, 105, 97, 89, 81, 73, 65, 57,
		49, 41, 33, 25, 17, 9, 1 }, /*Reihe 7*/
{ 184, 176, 168, 160, 152, 144, 136, 128, 120, 112, 104, 96, 88, 80, 72, 64, 56,
		48, 40, 32, 24, 16, 8, 0 }, /*Reihe 8*/
};

const static uint8_t ring_1[4] = { 91, 92, 99, 100 };
const static uint8_t ring_2[12] = { 82, 83, 84, 85, 90, 93, 98, 101, 106, 107,
		108, 109 };
const static uint8_t ring_3[20] = { 73, 74, 75, 76, 77, 78, 81, 86, 89, 94, 97,
		102, 105, 110, 113, 114, 115, 116, 117, 118 };
const static uint8_t ring_4[28] = { 64, 65, 66, 67, 68, 69, 70, 71, 72, 79, 80,
		87, 88, 95, 96, 103, 104, 111, 112, 119, 120, 121, 122, 123, 124, 125,
		126, 127 };

const static uint8_t ring_5[3][16] = { { 128, 120, 112, 104, 96, 88, 80, 72, 64,
		56 }, { 128, 129, 130, 131, 132, 133, 134, 135, 56, 57, 58, 59, 60, 61,
		62, 63 }, { 135, 127, 119, 111, 103, 95, 87, 79, 71, 63 } };

const static uint8_t ring_6[3][16] = { { 137, 129, 121, 113, 105, 97, 89, 81,
		73, 65, 57, 49, 48, 136 }, { 136, 137, 138, 139, 140, 141, 142, 143, 48,
		49, 50, 51, 52, 53, 54, 55 }, { 142, 134, 126, 118, 110, 102, 94, 86,
		78, 70, 62, 54, 55, 143 } };

const static uint8_t ring_7[3][18] = { { 146, 138, 130, 122, 114, 106, 98, 90,
		82, 74, 66, 58, 50, 42, 144, 145, 40, 41 }, { 144, 145, 146, 147, 148,
		149, 150, 151, 40, 41, 42, 43, 44, 45, 46, 47 }, { 149, 141, 133, 125,
		117, 109, 101, 93, 85, 77, 69, 61, 53, 45, 150, 151, 46, 47 } };

const static uint8_t ring_8[3][22] = { { 155, 147, 139, 131, 123, 115, 107, 99,
		91, 83, 75, 67, 59, 51, 43, 35, 152, 153, 154, 32, 33, 34

}, { 152, 153, 154, 155, 156, 157, 158, 159, 32, 33, 34, 35, 36, 37, 38, 39

}, { 156, 157, 158, 159, 36, 37, 38, 39, 148, 140, 132, 124, 116, 108, 100, 92,
		84, 76, 68, 60, 52, 44

} };

const static uint8_t ring_9[3][26] = { { 164, 156, 148, 140, 132, 124, 116, 108,
		100, 92, 84, 76, 68, 60, 52, 44, 36, 28, 160, 161, 162, 163, 24, 25, 26,
		27

}, { 160, 161, 162, 163, 164, 165, 166, 167, 24, 25, 26, 27, 28, 29, 30, 31

}, { 163, 164, 165, 166, 167, 27, 28, 29, 30, 31, 155, 147, 139, 131, 123, 115,
		107, 99, 91, 83, 75, 67, 59, 51, 43, 35

} };

const static uint8_t ring_10[3][30] = { { 168, 169, 170, 171, 172, 173, 16, 17,
		18, 19, 20, 21, 165, 157, 149, 141, 133, 125, 117, 109, 101, 93, 85, 77,
		69, 61, 53, 45, 37, 29

}, { 168, 169, 170, 171, 172, 173, 174, 175, 16, 17, 18, 19, 20, 21, 22, 23

}, { 170, 171, 172, 173, 174, 175, 18, 19, 20, 21, 22, 23, 162, 154, 146, 138,
		130, 122, 114, 106, 98, 90, 82, 74, 66, 58, 50, 42, 34, 26

} };

const static uint8_t ring_11[3][34] = { { 176, 177, 178, 179, 180, 181, 182, 8,
		9, 10, 11, 12, 13, 14, 174, 166, 158, 150, 142, 134, 126, 118, 110, 102,
		94, 86, 78, 70, 62, 54, 46, 38, 30, 22

}, { 176, 177, 178, 179, 180, 181, 182, 183, 8, 9, 10, 11, 12, 13, 14, 15

}, { 9, 10, 11, 12, 13, 14, 15, 177, 178, 179, 180, 181, 182, 183, 169, 161,
		153, 145, 137, 129, 121, 113, 105, 97, 89, 81, 73, 65, 57, 49, 41, 33,
		25, 17

} };

const static uint8_t ring_12[3][38] = { { 184, 185, 186, 187, 188, 189, 190,
		191, 0, 1, 2, 3, 4, 5, 6, 7, 183, 175, 167, 159, 151, 143, 135, 127,
		119, 111, 103, 95, 87, 79, 71, 63, 55, 47, 39, 31, 23, 15 }, {

184, 185, 186, 187, 188, 189, 190, 191, 0, 1, 2, 3, 4, 5, 6, 7

}, { 184, 185, 186, 187, 188, 189, 190, 191, 0, 1, 2, 3, 4, 5, 6, 7, 176, 168,
		160, 152, 144, 136, 128, 120, 112, 104, 96, 88, 80, 72, 64, 56, 48, 40,
		32, 24, 16, 8

} };
#endif

uint8_t SetCoordinate(int x, int y, uint32_t color) {

	if ((x <= 0) || (x > 24) || (y <= 0) || (y > 25)) {
		return ERR_RANGE;
	} else if (y < 9) {
		NEO_SetPixelColor(0, (191 - (x - 1) * 8 - (y - 1)), color);
	} else if (y < 17) {
		NEO_SetPixelColor(1, (191 - (x - 1) * 8 - (y - 9)), color);
	} else {
		NEO_SetPixelColor(2, (191 - (x - 1) * 8 - (y - 17)), color);
	}

	return ERR_OK;

}

uint8_t ClearCoordinate(int x, int y) {

	if ((x <= 0) || (x > 24) || (y <= 0) || (y > 25)) {
		return ERR_RANGE;
	} else if (y < 9) {
		NEO_SetPixelColor(0, (191 - (x - 1) * 8 - (y - 1)), 0x000000);
	} else if (y < 17) {
		NEO_SetPixelColor(1, (191 - (x - 1) * 8 - (y - 9)), 0x000000);
	} else {
		NEO_SetPixelColor(2, (191 - (x - 1) * 8 - (y - 17)), 0x000000);
	}

	return ERR_OK;

}

static uint8_t* getPixelsWavelength() {

	return WL_pixels;

}

static uint8_t getHighestColorValue(uint32_t color) {

	uint32_t red, green, blue;
	uint8_t result;

	red = NEO_GET_COLOR_RED(color);
	green = NEO_GET_COLOR_GREEN(color);
	blue = NEO_GET_COLOR_BLUE(color);

	if ((red >= green) && (red >= blue)) {
		result = red;
	}

	else if (green >= blue) {
		result = green;
	} else {
		result = blue;
	}

	return result;

}

static void displayLetter(char letter, uint32_t color) {

	if (letter == 'A') {

		SetCoordinate(2, 23, color);
		SetCoordinate(3, 23, color);
		SetCoordinate(4, 23, color);
		SetCoordinate(5, 23, color);

		SetCoordinate(2, 22, color);
		SetCoordinate(2, 21, color);
		SetCoordinate(2, 20, color);
		SetCoordinate(2, 19, color);

		SetCoordinate(5, 22, color);
		SetCoordinate(5, 21, color);
		SetCoordinate(5, 20, color);
		SetCoordinate(5, 19, color);

		SetCoordinate(3, 21, color);
		SetCoordinate(4, 21, color);

	} else if (letter == 'T') {
		SetCoordinate(2, 23, color);
		SetCoordinate(3, 23, color);
		SetCoordinate(4, 23, color);
		SetCoordinate(5, 23, color);
		SetCoordinate(6, 23, color);

		SetCoordinate(4, 22, color);
		SetCoordinate(4, 21, color);
		SetCoordinate(4, 20, color);
		SetCoordinate(4, 19, color);
	}

}

#define COORDINATE_X_PIXEL1	4
#define COORDINATE_X_PIXEL2	8
#define COORDINATE_X_PIXEL3	12
#define COORDINATE_X_PIXEL4	16
#define COORDINATE_X_PIXEL5 20

#if 0
#define COLOR_PIXEL1	0xff00ff
#define COLOR_PIXEL2	0x00ffff
#define COLOR_PIXEL3	0x00ff00
#define COLOR_PIXEL4	0xff0000
#endif

static seq3 calculateDecrementValues(DATA_t * characteristicValues,
		uint8_t excitation) {

	seq3 result;

	uint8_t highest_color_value_array[nDataPoints];

	result.coordinateArray[0][0] = COORDINATE_X_PIXEL1;
	result.coordinateArray[0][1] = COORDINATE_X_PIXEL1 + 1;
	result.coordinateArray[1][0] = COORDINATE_X_PIXEL2;
	result.coordinateArray[1][1] = COORDINATE_X_PIXEL2 + 1;
	result.coordinateArray[2][0] = COORDINATE_X_PIXEL3;
	result.coordinateArray[2][1] = COORDINATE_X_PIXEL3 + 1;
	result.coordinateArray[3][0] = COORDINATE_X_PIXEL4;
	result.coordinateArray[3][1] = COORDINATE_X_PIXEL4 + 1;
	result.coordinateArray[4][0] = COORDINATE_X_PIXEL5;
	result.coordinateArray[4][1] = COORDINATE_X_PIXEL5 + 1;

	/*
	 * Load Color value from config.c
	 */

	for (int r = 0; r < nDataPoints; r++) {
		result.colorArray[r] = getSequenzColor(2, r + 1);
		/*
		 * Calculate the highest color value (rgb)
		 */
		highest_color_value_array[r] = getHighestColorValue(
				result.colorArray[r]);
	}

	uint8_t* nPixelArray = getPixelsWavelength();

	for (int z = 0; z < nDataPoints; z++) {
		result.nPixelsArray[z] = nPixelArray[z];
	}

	/*
	 * calculating dectrementstep 1
	 */

	for (int u = 0; u < nDataPoints; u++) {

		if (result.nPixelsArray[u] != 0) {
			if (((highest_color_value_array[u] * NEO_PROCESSING_TIME)
					* result.nPixelsArray[u])
					> (characteristicValues->lifetime[excitation - 1][u])) {
				result.decrementArray[u] =
						round(
								(((float) highest_color_value_array[u]
										* NEO_PROCESSING_TIME)
										* result.nPixelsArray[u])
										/ (float) (characteristicValues->lifetime[excitation
												- 1][u]));
				result.counterArray[u] = 1;
			} else {

				result.counterArray[u] =
						round(
								(float) (characteristicValues->lifetime[excitation
										- 1][u])
										/ ((float) (highest_color_value_array[u]
												* NEO_PROCESSING_TIME)
												* result.nPixelsArray[u]));
				result.decrementArray[u] = 1;

			}
		}
	}

#if 0
	/*
	 * calculating dectrementstep 2
	 */

	if (result.nPixelsArray[u] != 0) {
		if (((highest_color_value_array[u] * NEO_PROCESSING_TIME)
						* result.nPixelsArray[u])
				> (characteristicValues->lifetime[excitation - 1][u])) {
			result.decrementArray[u] =
			round(
					((float) (highest_color_value_array[u]
									* NEO_PROCESSING_TIME)
							* result.nPixelsArray[u])
					/ (float) (characteristicValues->lifetime[excitation
							- 1][u]));
			result.counterArray[u] = 1;
		} else {

			result.counterArray[u] = round(
					(float) (characteristicValues->lifetime[excitation - 1][u])
					/ ((float) (highest_color_value_array[u]
									* NEO_PROCESSING_TIME)
							* result.nPixelsArray[u]));
			result.decrementArray[u] = 1;
		}
	}

	u++;

	/*
	 * calculating dectrementstep 3
	 */

	if (result.nPixelsArray[u] != 0) {
		if (((highest_color_value_array[u] * NEO_PROCESSING_TIME)
						* result.nPixelsArray[u])
				> (characteristicValues->lifetime[excitation - 1][u])) {
			result.decrementArray[u] =
			round(
					((float) (highest_color_value_array[u]
									* NEO_PROCESSING_TIME)
							* result.nPixelsArray[u])
					/ (float) (characteristicValues->lifetime[excitation
							- 1][u]));
			result.counterArray[u] = 1;
		} else {

			result.counterArray[u] = round(
					(float) (characteristicValues->lifetime[excitation - 1][u])
					/ ((float) (highest_color_value_array[u]
									* NEO_PROCESSING_TIME)
							* result.nPixelsArray[u]));
			result.decrementArray[u] = 1;
		}
	}

	/*
	 * calculating dectrementstep 4
	 */

	u++;

	if (result.nPixelsArray[u] != 0) {
		if (((highest_color_value_array[u] * NEO_PROCESSING_TIME)
						* result.nPixelsArray[u])
				> (characteristicValues->lifetime[excitation - 1][u])) {
			result.decrementArray[u] =
			round(
					(float) ((highest_color_value_array[u]
									* NEO_PROCESSING_TIME)
							* result.nPixelsArray[u])
					/ (float) (characteristicValues->lifetime[excitation
							- 1][u]));
			result.counterArray[u] = 1;
		} else {

			result.counterArray[u] = round(
					(float) (characteristicValues->lifetime[excitation - 1][u])
					/ (float) ((highest_color_value_array[u]
									* NEO_PROCESSING_TIME)
							* result.nPixelsArray[u]));
			result.decrementArray[u] = 1;

		}
	}

	/*
	 * calculating dectrementstep 5
	 */

	u++;
	if (result.nPixelsArray[u] != 0) {
		if (((highest_color_value_array[u] * NEO_PROCESSING_TIME)
						* result.nPixelsArray[u])
				> (characteristicValues->lifetime[excitation - 1][u])) {
			result.decrementArray[u] =
			round(
					(float) ((highest_color_value_array[u]
									* NEO_PROCESSING_TIME)
							* result.nPixelsArray[u])
					/ (float) (characteristicValues->lifetime[excitation
							- 1][u]));
			result.counterArray[u] = 1;
		} else {

			result.counterArray[u] = round(
					(float) (characteristicValues->lifetime[excitation - 1][u])
					/ ((float) (highest_color_value_array[u]
									* NEO_PROCESSING_TIME)
							* result.nPixelsArray[u]));
			result.decrementArray[u] = 1;

		}
	}
#endif
	return result;
}

static void updateLetterColor(uint8_t nPixels1, uint8_t nPixels2,
		uint8_t nPixels3, uint8_t nPixels4) {

	uint32_t color1, color2, color3, color4, sum1, sum2, sum3, sum4, sumRed,
			sumGreen, sumBlue = 0;
	uint8_t red1, green1, blue1, red2, green2, blue2, red3, green3, blue3, red4,
			green4, blue4, red, green, blue = 0;
	uint32_t color = 0;

	float percent1, percent2, percent3, percent4;

	sum1 = 10 * nPixels1;
	sum2 = 10 * nPixels2;
	sum3 = 10 * nPixels3;
	sum4 = 10 * nPixels4;

	percent1 = (((float) sum1) / ((float) 0xff));
	percent2 = (((float) sum2) / ((float) 0xff));
	percent3 = (((float) sum3) / ((float) 0xff));
	percent4 = (((float) sum4) / ((float) 0xff));

	red1 = NEO_GET_COLOR_RED(getSequenzColor(3, 1));
	green1 = NEO_GET_COLOR_GREEN(getSequenzColor(3, 1));
	blue1 = NEO_GET_COLOR_BLUE(getSequenzColor(3, 1));

	red1 = rint((float) percent1 * (float) red1);
	green1 = rint((float) percent1 * (float) green1);
	blue1 = rint((float) percent1 * (float) blue1);

	red2 = NEO_GET_COLOR_RED(getSequenzColor(3, 2));
	green2 = NEO_GET_COLOR_GREEN(getSequenzColor(3, 2));
	blue2 = NEO_GET_COLOR_BLUE(getSequenzColor(3, 2));

	red2 = rint((float) percent2 * (float) red2);
	green2 = rint((float) percent2 * (float) green2);
	blue2 = rint((float) percent2 * (float) blue2);

	red3 = NEO_GET_COLOR_RED(getSequenzColor(3, 3));
	green3 = NEO_GET_COLOR_GREEN(getSequenzColor(3, 3));
	blue3 = NEO_GET_COLOR_BLUE(getSequenzColor(3, 3));

	red3 = rint((float) percent3 * (float) red3);
	green3 = rint((float) percent3 * (float) green3);
	blue3 = rint((float) percent3 * (float) blue3);

	red4 = NEO_GET_COLOR_RED(getSequenzColor(3, 4));
	green4 = NEO_GET_COLOR_GREEN(getSequenzColor(3, 4));
	blue4 = NEO_GET_COLOR_BLUE(getSequenzColor(3, 4));

	red4 = rint((float) percent4 * (float) red4);
	green4 = rint((float) percent4 * (float) green4);
	blue4 = rint((float) percent4 * (float) blue4);

	sumRed = red1 + red2 + red3 + red4;
	sumGreen = green1 + green2 + green3 + green4;
	sumBlue = blue1 + blue2 + blue3 + blue4;

	if (sumRed > 0xff) {
		red = 0xff;
	} else {
		red = sumRed;
	}

	if (sumGreen > 0xff) {
		green = 0xff;
	} else {
		green = sumGreen;
	}

	if (sumBlue > 0xff) {
		blue = 0xff;
	} else {
		blue = sumBlue;
	}

	color = NEO_MAKE_COLOR_RGB(red, green, blue);

	displayLetter('T', color);

}

static void setPixelsWavelength(uint8_t* pixels) {

	for (int i = 0; i < nDataPoints; i++) {
		WL_pixels[i] = pixels[i];
	}

}

#if 0

static void displayText(char prefix, uint32_t color) {

}
#endif

static uint8_t getHighestColorValueFromMatrix() {

	NEO_Color color;
	uint8_t temp = 0;
	uint8_t highscore = 0;
	uint32_t color_val = 0;

	int i = 0;

	for (int i = 0; i < 3; i++) {
		for (int l = 0; l < 192; l++) {
			NEO_GetPixelColor(i, l, &color);

			temp = getHighestColorValue(color);
			if (temp >= highscore) {
				highscore = temp;
				color_val = color;
			}

		}

	}

	return highscore;

}

static uint32_t getMatrixColorValue() {

	uint32_t value = 0;
	NEO_Color color;
	uint32_t red, green, blue = 0;

	int i = 0;

	for (int i = 0; i < 3; i++) {
		for (int l = 0; l < 192; l++) {
			NEO_GetPixelColor(i, l, &color);

			red = NEO_GET_COLOR_RED(color);
			green = NEO_GET_COLOR_GREEN(color);
			blue = NEO_GET_COLOR_BLUE(color);

			value = value + red + green + blue;
		}
	}
	return value;
}

static uint8_t getHighestColorValueFromLane() {

	NEO_Color color;
	uint8_t temp = 0;
	uint8_t highscore = 0;
	uint32_t color_val = 0;

	int i = 0;

	for (i = 0; i < 24; i++) {
		NEO_GetPixelColor(1, lane12[i], &color);
		temp = getHighestColorValue(color);
		if (temp >= highscore) {
			highscore = temp;
			color_val = color;
		}
	}

	for (i = 0; i < 24; i++) {
		NEO_GetPixelColor(1, lane13[i], &color);
		temp = getHighestColorValue(color);
		if (temp >= highscore) {
			highscore = temp;
			color_val = color;
		}
	}

	return highscore;

}

static uint32_t getHighestRingData(uint8_t ring) {

	NEO_Color color;

	int i = 0;
	int k = 0;
	int m = 0;
	uint8_t highscore = 0;
	uint8_t temp = 0;
	uint32_t color_val = 0;

	switch (ring) {

	case 1:

		for (i = 0; i < 4; i++) {
			NEO_GetPixelColor(1, ring_1[i], &color);
			temp = getHighestColorValue(color);
			if (temp >= highscore) {
				highscore = temp;
				color_val = color;
			}
		}
		break;
	case 2:

		for (i = 0; i < 12; i++) {
			NEO_GetPixelColor(1, ring_2[i], &color);
			temp = getHighestColorValue(color);
			if (temp >= highscore) {
				highscore = temp;
				color_val = color;
			}
		}
		break;
	case 3:
		NEO_GetPixelColor(1, ring_3[0], &color);
		break;
	case 4:
		NEO_GetPixelColor(1, ring_4[0], &color);
		break;
	case 5:
		break;
	case 6:
		break;
	case 7:
		break;
	case 8:
		break;
	case 9:
		break;
	case 10:
		break;
	case 11:
		break;
	case 12:
		break;

	}

	return color_val;
}

uint8_t clearRingData(uint8_t ring) {

	uint8_t lenght_lane_0 = 0;
	uint8_t lenght_lane_1 = 0;
	uint8_t lenght_lane_2 = 0;
	uint8_t res = ERR_OK;
	uint8_t temp;
	int i = 0;

	if (ring < 1 || ring > 12) {
		return ERR_FAILED; /*out of Range*/
	}

	switch (ring) {
	case 1:
		lenght_lane_1 = sizeof(ring_1) / sizeof(ring_1[0]);
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_ClearPixel(1, ring_1[i]);
		}
		break;

	case 2:
		lenght_lane_1 = sizeof(ring_2) / sizeof(ring_2[0]);
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_ClearPixel(1, ring_2[i]);
		}
		break;
	case 3:
		lenght_lane_1 = sizeof(ring_3) / sizeof(ring_3[0]);
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_ClearPixel(1, ring_3[i]);
		}
		break;
	case 4:
		lenght_lane_1 = sizeof(ring_4) / sizeof(ring_4[0]);
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_ClearPixel(1, ring_4[i]);
		}
		break;
	case 5:
		lenght_lane_0 = 10;
		lenght_lane_1 = 16;
		lenght_lane_2 = 10;

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_ClearPixel(0, ring_5[0][i]);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_ClearPixel(1, ring_5[1][i]);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_ClearPixel(2, ring_5[2][i]);
		}
		break;

	case 6:

		lenght_lane_0 = 14; //sizeof(ring_6[0]) / sizeof(ring_6[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = 14; //sizeof(ring_6[2]) / sizeof(ring_6[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			temp = ring_6[0][i];
			NEO_ClearPixel(0, ring_6[0][i]);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			temp = ring_6[1][i];
			NEO_ClearPixel(1, ring_6[1][i]);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			temp = ring_6[2][i];
			NEO_ClearPixel(2, ring_6[2][i]);
		}
		break;

	case 7:
		lenght_lane_0 = sizeof(ring_7[0]) / sizeof(ring_7[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_7[2]) / sizeof(ring_7[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_ClearPixel(0, ring_7[0][i]);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_ClearPixel(1, ring_7[1][i]);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_ClearPixel(2, ring_7[2][i]);
		}
		break;

	case 8:
		lenght_lane_0 = sizeof(ring_8[0]) / sizeof(ring_8[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_8[2]) / sizeof(ring_8[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_ClearPixel(0, ring_8[0][i]);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_ClearPixel(1, ring_8[1][i]);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_ClearPixel(2, ring_8[2][i]);
		}
		break;

	case 9:
		lenght_lane_0 = sizeof(ring_9[0]) / sizeof(ring_9[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_9[2]) / sizeof(ring_9[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_ClearPixel(0, ring_9[0][i]);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_ClearPixel(1, ring_9[1][i]);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_ClearPixel(2, ring_9[2][i]);
		}
		break;

	case 10:
		lenght_lane_0 = sizeof(ring_10[0]) / sizeof(ring_10[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_10[2]) / sizeof(ring_10[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_ClearPixel(0, ring_10[0][i]);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_ClearPixel(1, ring_10[1][i]);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_ClearPixel(2, ring_10[2][i]);
		}
		break;

	case 11:
		lenght_lane_0 = sizeof(ring_11[0]) / sizeof(ring_11[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_11[2]) / sizeof(ring_11[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_ClearPixel(0, ring_11[0][i]);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_ClearPixel(1, ring_11[1][i]);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_ClearPixel(2, ring_11[2][i]);
		}
		break;

	case 12:
		lenght_lane_0 = sizeof(ring_12[0]) / sizeof(ring_12[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_12[2]) / sizeof(ring_12[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_ClearPixel(0, ring_12[0][i]);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_ClearPixel(1, ring_12[1][i]);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_ClearPixel(2, ring_12[2][i]);
		}
		break;
	}

	return res;

}

uint8_t setRingData(uint8_t ring, uint32_t color) {

	uint8_t lenght_lane_0 = 0;
	uint8_t lenght_lane_1 = 0;
	uint8_t lenght_lane_2 = 0;
	uint8_t res = ERR_OK;
	uint8_t temp;
	int i = 0;

	if (ring < 1 || ring > 12) {
		return ERR_FAILED; /*out of Range*/
	}

	switch (ring) {
	case 1:
		lenght_lane_1 = sizeof(ring_1) / sizeof(ring_1[0]);
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_SetPixelColor(1, ring_1[i], color);
		}
		break;

	case 2:
		lenght_lane_1 = sizeof(ring_2) / sizeof(ring_2[0]);
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_SetPixelColor(1, ring_2[i], color);
		}
		break;
	case 3:
		lenght_lane_1 = sizeof(ring_3) / sizeof(ring_3[0]);
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_SetPixelColor(1, ring_3[i], color);
		}
		break;
	case 4:
		lenght_lane_1 = sizeof(ring_4) / sizeof(ring_4[0]);
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_SetPixelColor(1, ring_4[i], color);
		}
		break;
	case 5:
		lenght_lane_0 = 10;
		lenght_lane_1 = 16;
		lenght_lane_2 = 10;

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_SetPixelColor(0, ring_5[0][i], color);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_SetPixelColor(1, ring_5[1][i], color);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_SetPixelColor(2, ring_5[2][i], color);
		}
		break;

	case 6:

		lenght_lane_0 = 14; //sizeof(ring_6[0]) / sizeof(ring_6[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = 14; //sizeof(ring_6[2]) / sizeof(ring_6[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			temp = ring_6[0][i];
			NEO_SetPixelColor(0, ring_6[0][i], color);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			temp = ring_6[1][i];
			NEO_SetPixelColor(1, ring_6[1][i], color);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			temp = ring_6[2][i];
			NEO_SetPixelColor(2, ring_6[2][i], color);
		}
		break;

	case 7:
		lenght_lane_0 = sizeof(ring_7[0]) / sizeof(ring_7[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_7[2]) / sizeof(ring_7[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_SetPixelColor(0, ring_7[0][i], color);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_SetPixelColor(1, ring_7[1][i], color);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_SetPixelColor(2, ring_7[2][i], color);
		}
		break;

	case 8:
		lenght_lane_0 = sizeof(ring_8[0]) / sizeof(ring_8[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_8[2]) / sizeof(ring_8[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_SetPixelColor(0, ring_8[0][i], color);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_SetPixelColor(1, ring_8[1][i], color);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_SetPixelColor(2, ring_8[2][i], color);
		}
		break;

	case 9:
		lenght_lane_0 = sizeof(ring_9[0]) / sizeof(ring_9[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_9[2]) / sizeof(ring_9[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_SetPixelColor(0, ring_9[0][i], color);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_SetPixelColor(1, ring_9[1][i], color);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_SetPixelColor(2, ring_9[2][i], color);
		}
		break;

	case 10:
		lenght_lane_0 = sizeof(ring_10[0]) / sizeof(ring_10[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_10[2]) / sizeof(ring_10[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_SetPixelColor(0, ring_10[0][i], color);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_SetPixelColor(1, ring_10[1][i], color);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_SetPixelColor(2, ring_10[2][i], color);
		}
		break;

	case 11:
		lenght_lane_0 = sizeof(ring_11[0]) / sizeof(ring_11[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_11[2]) / sizeof(ring_11[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_SetPixelColor(0, ring_11[0][i], color);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_SetPixelColor(1, ring_11[1][i], color);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_SetPixelColor(2, ring_11[2][i], color);
		}
		break;

	case 12:
		lenght_lane_0 = sizeof(ring_12[0]) / sizeof(ring_12[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_12[2]) / sizeof(ring_12[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_SetPixelColor(0, ring_12[0][i], color);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_SetPixelColor(1, ring_12[1][i], color);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_SetPixelColor(2, ring_12[2][i], color);
		}
		break;
	}

	return res;

}

static uint32_t decrementValue(uint32_t color, uint8_t decrementVal) {
	uint32_t red, green, blue;

	red = NEO_GET_COLOR_RED(color);
	green = NEO_GET_COLOR_GREEN(color);
	blue = NEO_GET_COLOR_BLUE(color);

	if (red > decrementVal) {
		red = red - decrementVal;

	} else {
		red = 0;
	}
	if (green > decrementVal) {
		green = green - decrementVal;

	} else {
		green = 0;
	}
	if (blue > decrementVal) {
		blue = blue - decrementVal;

	} else {
		blue = 0;
	}

	return NEO_MAKE_COLOR_RGB(red, green, blue);
}

static void decrementRing(int ring, uint32_t step) {

	uint32_t color;
	uint8_t red, green, blue;

	color = getHighestRingData(ring);

	red = NEO_GET_COLOR_RED(color);
	green = NEO_GET_COLOR_GREEN(color);
	blue = NEO_GET_COLOR_BLUE(color);

	if (red >= step) {
		red = red - step;
	} else {
		red = 0;
	}

	if (green >= step) {
		green = green - step;
	} else {
		green = 0;
	}

	if (blue >= step) {
		blue = blue - step;
	} else {
		blue = 0;
	}

	setRingData(ring, NEO_MAKE_COLOR_RGB(red, green, blue));

}

static uint8_t decrementRingData(uint8_t ring, uint8_t step) {

	uint8_t lenght_lane_0 = 0;
	uint8_t lenght_lane_1 = 0;
	uint8_t lenght_lane_2 = 0;
	uint8_t res = ERR_OK;
	uint8_t temp;

	uint8_t red, green, blue;

	uint32_t color_value = 0;
	NEO_Color color;
	int i = 0;

	if (ring < 1 || ring > 12) {
		return ERR_FAILED; /*out of Range*/
	}

	switch (ring) {
	case 1:
		lenght_lane_1 = sizeof(ring_1) / sizeof(ring_1[0]);
		for (i = 0; i < lenght_lane_1; i++) {

			NEO_GetPixelColor(1, ring_1[i], &color);
			color_value = decrementValue(color, step);

			NEO_SetPixelColor(1, ring_1[i], color_value);
		}
		break;

	case 2:
		lenght_lane_1 = sizeof(ring_2) / sizeof(ring_2[0]);
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_GetPixelColor(1, ring_2[i], &color);
			color_value = decrementValue(color, step);

			NEO_SetPixelColor(1, ring_2[i], color_value);
		}
		break;
	case 3:
		lenght_lane_1 = sizeof(ring_3) / sizeof(ring_3[0]);
		for (i = 0; i < lenght_lane_1; i++) {

			NEO_GetPixelColor(1, ring_3[i], &color);
			color_value = decrementValue(color, step);

			NEO_SetPixelColor(1, ring_3[i], color_value);
		}
		break;
	case 4:
		lenght_lane_1 = sizeof(ring_4) / sizeof(ring_4[0]);
		for (i = 0; i < lenght_lane_1; i++) {

			NEO_GetPixelColor(1, ring_4[i], &color);
			color_value = decrementValue(color, step);

			NEO_SetPixelColor(1, ring_4[i], color_value);
		}
		break;

	case 5:
		lenght_lane_0 = 10;
		lenght_lane_1 = 16;
		lenght_lane_2 = 10;

		for (i = 0; i < lenght_lane_0; i++) {

			NEO_GetPixelColor(0, ring_5[0][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(0, ring_5[0][i], color_value);
		}
		for (i = 0; i < lenght_lane_1; i++) {

			NEO_GetPixelColor(1, ring_5[1][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(1, ring_5[1][i], color_value);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_GetPixelColor(2, ring_5[2][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(2, ring_5[2][i], color_value);
		}
		break;

	case 6:

		lenght_lane_0 = 14; //sizeof(ring_6[0]) / sizeof(ring_6[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = 14; //sizeof(ring_6[2]) / sizeof(ring_6[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_GetPixelColor(0, ring_6[0][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(0, ring_6[0][i], color_value);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_GetPixelColor(1, ring_6[1][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(1, ring_6[1][i], color_value);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_GetPixelColor(2, ring_6[2][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(2, ring_6[2][i], color_value);
		}
		break;

	case 7:
		lenght_lane_0 = sizeof(ring_7[0]) / sizeof(ring_7[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_7[2]) / sizeof(ring_7[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_GetPixelColor(0, ring_7[0][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(0, ring_7[0][i], color_value);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_GetPixelColor(1, ring_7[1][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(1, ring_7[1][i], color_value);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_GetPixelColor(2, ring_7[2][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(2, ring_7[2][i], color_value);
		}
		break;

	case 8:
		lenght_lane_0 = sizeof(ring_8[0]) / sizeof(ring_8[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_8[2]) / sizeof(ring_8[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_GetPixelColor(0, ring_8[0][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(0, ring_8[0][i], color_value);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_GetPixelColor(1, ring_8[1][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(1, ring_8[1][i], color_value);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_GetPixelColor(2, ring_8[2][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(2, ring_8[2][i], color_value);
		}
		break;

	case 9:
		lenght_lane_0 = sizeof(ring_9[0]) / sizeof(ring_9[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_9[2]) / sizeof(ring_9[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_GetPixelColor(0, ring_9[0][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(0, ring_9[0][i], color_value);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_GetPixelColor(1, ring_9[1][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(1, ring_9[1][i], color_value);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_GetPixelColor(2, ring_9[2][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(2, ring_9[2][i], color_value);
		}
		break;

	case 10:
		lenght_lane_0 = sizeof(ring_10[0]) / sizeof(ring_10[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_10[2]) / sizeof(ring_10[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_GetPixelColor(0, ring_10[0][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(0, ring_10[0][i], color_value);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_GetPixelColor(1, ring_10[1][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(1, ring_10[1][i], color_value);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_GetPixelColor(2, ring_10[2][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(2, ring_10[2][i], color_value);
		}
		break;

	case 11:
		lenght_lane_0 = sizeof(ring_11[0]) / sizeof(ring_11[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_11[2]) / sizeof(ring_11[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_GetPixelColor(0, ring_11[0][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(0, ring_11[0][i], color_value);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_GetPixelColor(1, ring_11[1][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(1, ring_11[1][i], color_value);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_GetPixelColor(2, ring_11[2][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(2, ring_11[2][i], color_value);
		}
		break;

	case 12:
		lenght_lane_0 = sizeof(ring_12[0]) / sizeof(ring_12[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_12[2]) / sizeof(ring_12[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_GetPixelColor(0, ring_12[0][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(0, ring_12[0][i], color_value);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_GetPixelColor(1, ring_12[1][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(1, ring_12[1][i], color_value);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_GetPixelColor(2, ring_12[2][i], &color);
			color_value = decrementValue(color, step);
			NEO_SetPixelColor(2, ring_12[2][i], color_value);
		}
		break;
	}

	return res;

}

uint8_t DimmPercentRing(uint8_t ring, uint32_t percent) {

	uint8_t lenght_lane_0 = 0;
	uint8_t lenght_lane_1 = 0;
	uint8_t lenght_lane_2 = 0;
	uint8_t res = ERR_OK;
	int i = 0;

	if (ring < 1 || ring > 12) {
		return ERR_FAILED; /*out of Range*/
	}

	switch (ring) {
	case 1:
		lenght_lane_1 = sizeof(ring_1) / sizeof(ring_1[0]);
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_DimmPercentPixel(1, ring_1[i], percent);
		}
		break;
	case 2:
		lenght_lane_1 = sizeof(ring_2) / sizeof(ring_2[0]);
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_DimmPercentPixel(1, ring_2[i], percent);
		}
		break;
	case 3:
		lenght_lane_1 = sizeof(ring_3) / sizeof(ring_3[0]);
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_DimmPercentPixel(1, ring_3[i], percent);
		}
		break;
	case 4:
		lenght_lane_1 = sizeof(ring_4) / sizeof(ring_4[0]);
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_DimmPercentPixel(1, ring_4[i], percent);
		}
		break;
	case 5:
		lenght_lane_0 = 10;
		lenght_lane_1 = 16;
		lenght_lane_2 = 10;

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_DimmPercentPixel(0, ring_5[0][i], percent);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_DimmPercentPixel(1, ring_5[1][i], percent);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_DimmPercentPixel(2, ring_5[2][i], percent);
		}
		break;

	case 6:
		lenght_lane_0 = 14;
		lenght_lane_1 = 16;
		lenght_lane_2 = 14;

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_DimmPercentPixel(0, ring_6[0][i], percent);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_DimmPercentPixel(1, ring_6[1][i], percent);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_DimmPercentPixel(2, ring_6[2][i], percent);
		}
		break;

	case 7:
		lenght_lane_0 = sizeof(ring_7[0]) / sizeof(ring_7[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_7[2]) / sizeof(ring_7[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_DimmPercentPixel(0, ring_7[0][i], percent);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_DimmPercentPixel(1, ring_7[1][i], percent);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_DimmPercentPixel(2, ring_7[2][i], percent);
		}
		break;

	case 8:
		lenght_lane_0 = sizeof(ring_8[0]) / sizeof(ring_8[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_8[2]) / sizeof(ring_8[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_DimmPercentPixel(0, ring_8[0][i], percent);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_DimmPercentPixel(1, ring_8[1][i], percent);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_DimmPercentPixel(2, ring_8[2][i], percent);
		}
		break;

	case 9:
		lenght_lane_0 = sizeof(ring_9[0]) / sizeof(ring_9[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_9[2]) / sizeof(ring_9[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_DimmPercentPixel(0, ring_9[0][i], percent);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_DimmPercentPixel(1, ring_9[1][i], percent);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_DimmPercentPixel(2, ring_9[2][i], percent);
		}
		break;

	case 10:
		lenght_lane_0 = sizeof(ring_10[0]) / sizeof(ring_10[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_10[2]) / sizeof(ring_10[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_DimmPercentPixel(0, ring_10[0][i], percent);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_DimmPercentPixel(1, ring_10[1][i], percent);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_DimmPercentPixel(2, ring_10[2][i], percent);
		}
		break;

	case 11:
		lenght_lane_0 = sizeof(ring_11[0]) / sizeof(ring_11[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_11[2]) / sizeof(ring_11[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_DimmPercentPixel(0, ring_11[0][i], percent);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_DimmPercentPixel(1, ring_11[1][i], percent);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_DimmPercentPixel(2, ring_11[2][i], percent);
		}
		break;

	case 12:
		lenght_lane_0 = sizeof(ring_12[0]) / sizeof(ring_12[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_12[2]) / sizeof(ring_12[0][0]);

		for (i = 0; i < lenght_lane_0; i++) {
			NEO_DimmPercentPixel(0, ring_12[0][i], percent);
		}
		for (i = 0; i < lenght_lane_1; i++) {
			NEO_DimmPercentPixel(1, ring_12[1][i], percent);
		}
		for (i = 0; i < lenght_lane_2; i++) {
			NEO_DimmPercentPixel(2, ring_12[2][i], percent);
		}
		break;
	}

	return res;

}

uint8_t DimmPercentPixel(int x, int y, uint8_t percent) {
	uint8_t red, green, blue;
	uint32_t dRed, dGreen, dBlue;

	uint8_t res;
	NEO_Color color;

	if ((x <= 0) || (x > 24) || (y <= 0) || (y > 25)) {
		return ERR_RANGE;
	} else if (y < 9) {
		res = NEO_GetPixelColor(0, (191 - (x - 1) * 8 - (y - 1)), &color);
	} else if (y < 17) {
		res = NEO_GetPixelColor(1, (191 - (x - 1) * 8 - (y - 9)), &color);
	} else {
		res = NEO_GetPixelColor(2, (191 - (x - 1) * 8 - (y - 17)), &color);
	}

	if (res != ERR_OK) {
		return res;
	}
	red = NEO_GET_COLOR_RED(color);
	green = NEO_GET_COLOR_GREEN(color);
	blue = NEO_GET_COLOR_BLUE(color);
	dRed = ((uint32_t) red * (100 - percent)) / 100;
	dGreen = ((uint32_t) green * (100 - percent)) / 100;
	dBlue = ((uint32_t) blue * (100 - percent)) / 100;

	color = NEO_MAKE_COLOR_RGB(dRed, dGreen, dBlue);

	if ((x <= 0) || (x > 24) || (y <= 0) || (y > 25)) {
		return ERR_RANGE;
	} else if ((y > 0) && (y < 9)) {
		return NEO_SetPixelColor(0, (191 - (x - 1) * 8 - (y - 1)), color);
	} else if ((y > 8) && (y < 17)) {
		return NEO_SetPixelColor(1, (191 - (x - 1) * 8 - (y - 9)), color);
	} else if (y > 16) {
		return NEO_SetPixelColor(2, (191 - (x - 1) * 8 - (y - 17)), color);
	}

	return res;
}

#if NEOA_CONFIG_PARSE_COMMAND_ENABLED
static uint8_t PrintStatus(const CLS1_StdIOType *io) {
	uint8_t buf[32];
	uint8_t res;

	CLS1_SendStatusStr((unsigned char*) "neoa", (unsigned char*) "\r\n",
			io->stdOut);
	UTIL1_Num8uToStr(buf, sizeof(buf), NEOA_LightLevel);
	UTIL1_strcat(buf, sizeof(buf),
			NEOA_isAutoLightLevel ? " (auto)\r\n" : " (fix)\r\n");
	CLS1_SendStatusStr("  lightlevel", buf, io->stdOut);
	UTIL1_strcpy(buf, sizeof(buf),
			NEOA_useGammaCorrection ? "on\r\n" : "off\r\n");
	CLS1_SendStatusStr("  gamma", buf, io->stdOut);
	return ERR_OK;
}

static uint8_t PrintHelp(const CLS1_StdIOType *io) {
	CLS1_SendHelpStr((unsigned char*) "neoa",
			(unsigned char*) "Group of neoa commands\r\n", io->stdOut);
	CLS1_SendHelpStr((unsigned char*) "  lightlevel <val>|auto",
			(unsigned char*) "Set light level (0..255) or use auto level\r\n",
			io->stdOut);
	CLS1_SendHelpStr((unsigned char*) "  gamma on|off",
			(unsigned char*) "Usage of gamma correction\r\n", io->stdOut);
	CLS1_SendHelpStr((unsigned char*) "  help|status",
			(unsigned char*) "Print help or status information\r\n",
			io->stdOut);
	return ERR_OK;
}
#endif /* NEOA_CONFIG_PARSE_COMMAND_ENABLED */

uint8_t NEOA_Lauflicht(void) {

}

static bool SoftwareCurrentLimit(char* data, uint8_t farbtiefe) {

	bool isLimited = FALSE;
	uint32_t Farbwert = 0;
	uint32_t maxAllowedColorValue = 0;
	uint8_t val = 0;
	uint8_t cnt;
	uint8_t percent = 0;
	float currentLimitation;
	uint8_t powerConnected = getPowerConnected();
	uint32_t current_per_powerSupply = getPowerSupplyCurrent();
	uint8_t current_per_pixel = getCurrentPerPixel();
	uint32_t nVal = ((farbtiefe) / 8) * 24 * 24;

	maxAllowedColorValue = rint(
			(float) ((current_per_powerSupply) * (powerConnected)) * (255) * (3)
					/ current_per_pixel);

	/*Der Farbwert entspricht dem gesamten Farbwert des Bildes*/
	for (int z = 0; z < nVal; z++) {
		val = (data[z]);
		Farbwert = Farbwert + val;
	}

	/*hier würde man zum Farbwert noch den Helligkeitsfaktor dazurechnen, resp zuerst alle werte damit multiplizieren*/

	if (Farbwert > maxAllowedColorValue) {
		currentLimitation = (float) (((float) (maxAllowedColorValue))
				/ ((float) (Farbwert)));
		percent = currentLimitation * 100;
		isLimited = TRUE;
		for (int z = 0; z < nVal; z++) {
			data[z] = ((uint8_t) data[z] * (percent)) / 100;
		}

	}
	return isLimited;

}

uint8_t NEOA_Display_Image(char* image, unsigned short farbtiefe) {

	uint32_t size;
	uint32_t position = 0;
	uint8_t red = 0;
	uint8_t green = 0;
	uint8_t blue = 0;
	uint32_t colorValue = 0;
	uint8_t lane = 0;
	uint32_t color;
	FRESULT res = FR_OK;
	uint32_t cnt = 0;
	uint8_t val = 0;
	int j = 0;
	int k = 0;
	int i = 0;
	char* data;

	NEO_ClearAllPixel();
	NEO_TransferPixels();

	/*Softwarestrombegrenzung*/
#if 1
	bool isLimited = SoftwareCurrentLimit(image, farbtiefe);

//	size = ((image->biWidth) * (image->biHeight));
#endif
	for (j = 0; j < NEOC_NOF_LANES; j++) {
		for (k = 0; k < SINGLE_MATRIX_SIDE_LENGTH; k++) {
			for (i = 0; i < MATRIX_RES; i++) {
				position = lookUpMatrix[k][i];
				red = (image[cnt]);
				green = (image[cnt + 1]);
				blue = (image[cnt + 2]);
				colorValue = (red << 16) + (green << 8) + (blue);
				NEO_SetPixelColor(j, position, colorValue);
				cnt = cnt + ((farbtiefe) / 8);
			}
		}

	}
	NEO_TransferPixels();

	return res;
}

#if NEOA_CONFIG_PARSE_COMMAND_ENABLED

uint8_t NEOA_ParseCommand(const unsigned char* cmd, bool *handled,
		const CLS1_StdIOType *io) {
	uint8_t res = ERR_OK;
	const unsigned char *p;

	if (UTIL1_strcmp((char*)cmd, CLS1_CMD_HELP) == 0
			|| UTIL1_strcmp((char*)cmd, "neoa help") == 0) {
		*handled = TRUE;
		return PrintHelp(io);
	} else if ((UTIL1_strcmp((char*)cmd, CLS1_CMD_STATUS) == 0)
			|| (UTIL1_strcmp((char*)cmd, "neoa status") == 0)) {
		*handled = TRUE;
		res = PrintStatus(io);
	} else if ((UTIL1_strcmp((char*)cmd, CLS1_CMD_STATUS) == 0)
			|| (UTIL1_strcmp((char*)cmd, "neoa lauflicht") == 0)) {
		*handled = TRUE;
		res = NEOA_Lauflicht();
	} else if (UTIL1_strcmp((char*)cmd, "neoa lightlevel auto") == 0) {
		NEOA_isAutoLightLevel = TRUE;
		*handled = TRUE;
		res = ERR_OK;
	} else if (UTIL1_strncmp((char* )cmd, "neoa lightlevel ",
			sizeof("neoa lightlevel ") - 1) == 0) {
		int32_t level;

		p = cmd + sizeof("neoa lightlevel ") - 1;
		res = UTIL1_xatoi(&p, &level);
		if (res == ERR_OK) {
			if (level < 0) {
				level = 0;
			} else if (level > 0xff) {
				level = 0xff;
			}
			NEOA_isAutoLightLevel = FALSE;
			NEOA_LightLevel = level;
		}
		*handled = TRUE;
	}
	return res;
}
#endif /* NEOA_CONFIG_PARSE_COMMAND_ENABLED */

static uint32_t getColorDimmedWithGamma(uint32_t color, uint8_t percent) {
	uint32_t red, green, blue;
	uint32_t redCorr, greenCorr, blueCorr, dRed, dBlue, dGreen;

	red = NEO_GET_COLOR_RED(color);
	green = NEO_GET_COLOR_GREEN(color);
	blue = NEO_GET_COLOR_BLUE(color);
	dRed = ((uint32_t) red * (100 - percent)) / 100;
	dGreen = ((uint32_t) green * (100 - percent)) / 100;
	dBlue = ((uint32_t) blue * (100 - percent)) / 100;

	redCorr = NEO_GammaCorrect8(dRed);
	greenCorr = NEO_GammaCorrect8(dGreen);
	blueCorr = NEO_GammaCorrect8(dBlue);
	return NEO_MAKE_COLOR_RGB(redCorr, greenCorr, blueCorr);
}

static uint32_t getColorPercentage(uint32_t color, uint8_t percent) {
	uint32_t red, green, blue;
	uint32_t redCorr, greenCorr, blueCorr, dRed, dBlue, dGreen;

	red = NEO_GET_COLOR_RED(color);
	green = NEO_GET_COLOR_GREEN(color);
	blue = NEO_GET_COLOR_BLUE(color);
	dRed = ((uint32_t) red * (100 - percent)) / 100;
	dGreen = ((uint32_t) green * (100 - percent)) / 100;
	dBlue = ((uint32_t) blue * (100 - percent)) / 100;
	return NEO_MAKE_COLOR_RGB(dRed, dGreen, dBlue);

}

static uint32_t getResolution(uint32_t lifetime1, uint32_t lifetime2,
		uint32_t lifetime3, uint32_t lifetime4) {

	float temp;
	uint32_t resolution = 0;
	if ((lifetime1 >= lifetime2) && (lifetime1 >= lifetime3)
			&& (lifetime1 >= lifetime4)) {
		resolution = ceil((float) ((float) (lifetime1) / (float) (24)));
	} else if ((lifetime2 >= lifetime3) && (lifetime2 >= lifetime4)) {
		resolution = ceil((float) ((float) (lifetime2) / (float) (24)));

	} else if ((lifetime3 >= lifetime4)) {
		resolution = ceil((float) ((float) (lifetime3) / (float) (24)));
	}

	else {
		resolution = ceil((float) ((float) (lifetime4) / (float) (24)));
	}

	return resolution;

}

static void setupMatrix(uint8_t nPixels1, uint8_t nPixels2, uint8_t nPixels3,
		uint8_t nPixels4) {

	int i;
	for (i = 1; i <= nPixels1; i++) {
		SetCoordinate(COORDINATE_X_PIXEL1, i, getSequenzColor(3, 1));
		SetCoordinate(COORDINATE_X_PIXEL1 + 1, i, getSequenzColor(3, 1));
	}

	for (i = 1; i <= nPixels2; i++) {
		SetCoordinate(COORDINATE_X_PIXEL2, i, getSequenzColor(3, 2));
		SetCoordinate(COORDINATE_X_PIXEL2 + 1, i, getSequenzColor(3, 2));
	}

	for (i = 1; i <= nPixels3; i++) {
		SetCoordinate(COORDINATE_X_PIXEL3, i, getSequenzColor(3, 3));
		SetCoordinate(COORDINATE_X_PIXEL3 + 1, i, getSequenzColor(3, 3));
	}

	for (i = 1; i <= nPixels4; i++) {
		SetCoordinate(COORDINATE_X_PIXEL4, i, getSequenzColor(3, 4));
		SetCoordinate(COORDINATE_X_PIXEL4 + 1, i, getSequenzColor(3, 4));
	}

	NEO_TransferPixels();

}

static RETURN_STATUS playSeq1(DATA_t * characteristicValues, char* colorData,
		unsigned short farbtiefe, uint8_t excitation) {

	RETURN_STATUS aborted = notAborted;
	DataMessage_t * pxRxDataMessage;
	pxRxDataMessage = &xDataMessage;
	uint8_t delay = (DELAY_MS) + (NEO_PROCESSING_TIME);
	uint8_t percente = 0;
	uint32_t fadeout = 0;
	uint32_t matrixColorValue = 0;

	if (excitation == 1) {
		SetTrail(characteristicValues->color_266, 13, 5, 50, getTrailSpeed());
		fadeout = characteristicValues->fadeout_266;
	} else if (excitation == 2) {
		SetTrail(characteristicValues->color_355, 13, 5, 50, getTrailSpeed());
		fadeout = characteristicValues->fadeout_355;
	} else if (excitation == 3) {
		SetTrail(characteristicValues->color_405, 13, 5, 50, getTrailSpeed());
		fadeout = characteristicValues->fadeout_405;
	}

	NEOA_Display_Image(colorData, farbtiefe);
	uint8_t highestColVal = 0xff;

	for (int i = 1; i < 13; i++) {
		DimmPercentRing(i, (percente));
		percente = percente + STARTING_DEGRADATION;
	}

	NEO_TransferPixels();

//matrixColorValue = getMatrixColorValue();

	highestColVal = getHighestColorValueFromLane();

	uint32_t nTicks = rint((float) (fadeout) / (delay));

	uint32_t nCycles = ceil((float) ((nTicks) / (nRINGS)));

	uint32_t decrementStep = ceil((float) (highestColVal) / (float) nCycles);

	for (int i = 0; i < nCycles; i++) {
		for (int z = 1; z <= nRINGS; z++) {
			decrementRingData(z, decrementStep);
			NEO_TransferPixels();
			vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

			if (TakeMessageFromDataQueue(queue_handler_data, pxRxDataMessage)
					!= QUEUE_EMPTY) {
				if (pxRxDataMessage->cmd == stop) {
					return stopAborted;
				} else if (pxRxDataMessage->cmd == playImage) {
					return playImageAborted; /*Session was aborted*/
				}

				else if (pxRxDataMessage->cmd == pause) {
					for (;;) {
						if (TakeMessageFromDataQueue(queue_handler_data,
								pxRxDataMessage) != QUEUE_EMPTY) {
							if (pxRxDataMessage->cmd == play) {
								break;
							} else if (pxRxDataMessage->cmd == stop) {
								return stopAborted;
							} else if (pxRxDataMessage->cmd == playImage) {
								return playImageAborted;
							}
						} else {
							vTaskDelay(pdMS_TO_TICKS(20));
						}
					}
				}

			}

		}

		highestColVal = getHighestColorValueFromMatrix();
		if (highestColVal == 0) {
			NEO_ClearAllPixel();
			NEO_TransferPixels();
			break;
		}

	}

	return aborted;

}

#define COLOR_LETTER	0x7E7E7E

static bool playSeq2(DATA_t * characteristicValues, uint8_t excitation) {

	uint8_t value1, value2, value3, value4, value5, nPixels1, nPixels2,
			nPixels3, nPixels4, nPixels5;

	uint8_t nPixelArray[nDataPoints];

#if 0		//Letter disabled
	if (getLetterEnabled('A')) { /*check if enabled in Config.txt*/
		displayLetter('A', COLOR_LETTER);
	}
#endif

	if (excitation == 1) {
		value1 = characteristicValues->amplitude_266_1;
		value2 = characteristicValues->amplitude_266_2;
		value3 = characteristicValues->amplitude_266_3;
		value4 = characteristicValues->amplitude_266_4;
		value5 = characteristicValues->amplitude_266_5;
	} else if (excitation == 2) {
		value1 = characteristicValues->amplitude_355_1;
		value2 = characteristicValues->amplitude_355_2;
		value3 = characteristicValues->amplitude_355_3;
		value4 = characteristicValues->amplitude_355_4;
		value5 = characteristicValues->amplitude_355_5;
	} else if (excitation == 3) {

		value1 = characteristicValues->amplitude_405_1;
		value2 = characteristicValues->amplitude_405_2;
		value3 = characteristicValues->amplitude_405_3;
		value4 = characteristicValues->amplitude_405_4;
		value5 = characteristicValues->amplitude_405_5;

	}

	float n1, n2, n3, n4, n5;

	float divisor = 100 / 24;

	if ((value1) < 4) {
		nPixels1 = 0;
	} else {
		n1 = (value1 / divisor);
		nPixels1 = floor(n1);
		if (nPixels1 >= 0x19) {
			nPixels1 = 0x18;
		}
	}
	nPixelArray[0] = nPixels1;

	for (int i = 1; i <= nPixels1; i++) {
		SetCoordinate(4, i, getSequenzColor(2, 1));
		SetCoordinate(5, i, getSequenzColor(2, 1));

	}

	if (value2 < 4) {
		nPixels2 = 0;
	} else {
		n2 = (value2 / divisor);
		nPixels2 = floor(n2);
		if (nPixels2 >= 0x19) {
			nPixels2 = 0x18;
		}
	}

	nPixelArray[1] = nPixels2;

	for (int i = 1; i <= nPixels2; i++) {
		SetCoordinate(8, i, getSequenzColor(2, 2));
		SetCoordinate(9, i, getSequenzColor(2, 2));

	}
	if (value3 < 4) {
		nPixels3 = 0;
	} else {
		n3 = (value3 / divisor);
		nPixels3 = floor(n3);
		if (nPixels3 >= 0x19) {
			nPixels3 = 0x18;
		}
	}

	nPixelArray[2] = nPixels3;

	for (int i = 1; i <= nPixels3; i++) {
		SetCoordinate(12, i, getSequenzColor(2, 3));
		SetCoordinate(13, i, getSequenzColor(2, 3));

	}
	if (value4 < 4) {
		nPixels4 = 0;
	} else {
		n4 = (value4 / divisor);
		nPixels4 = floor(n4);
		if (nPixels4 >= 0x19) {
			nPixels4 = 0x18;
		}
	}

	nPixelArray[3] = nPixels4;
	for (int i = 1; i <= nPixels4; i++) {
		SetCoordinate(16, i, getSequenzColor(2, 4));
		SetCoordinate(17, i, getSequenzColor(2, 4));

	}
	if (value5 < 4) {
		nPixels5 = 0;
	} else {
		n5 = (value5 / divisor);
		nPixels5 = floor(n5);
		if (nPixels5 >= 0x19) {
			nPixels5 = 0x18;
		}
	}
	nPixelArray[4] = nPixels5;

	for (int i = 1; i <= nPixels5; i++) {
		SetCoordinate(20, i, getSequenzColor(2, 5));
		SetCoordinate(21, i, getSequenzColor(2, 5));

	}

	NEO_TransferPixels();

	setPixelsWavelength(nPixelArray);

}

#define DELAY_TIME 					0		/*0ms */
#define DECR_DELAY_AT_DELAY_TIME 	((255*5)*((NEO_PROCESSING_TIME)+(DELAY_TIME))/5)

static RETURN_STATUS playSeq3(DATA_t * characteristicValues, uint8_t excitation) {

	RETURN_STATUS aborted = notAborted;
	DataMessage_t * pxRxDataMessage;
	pxRxDataMessage = &xDataMessage;

	uint8_t cntArray[nDataPoints];

	for (int l = 0; l < nDataPoints; l++) {
		cntArray[l] = 0;
	}

	/*
	 * Calculate the characteristic values assigned to the excitation
	 */
	seq3 char_values_seq3 = calculateDecrementValues(characteristicValues,
			excitation);

	int index = 0;

	while (!((char_values_seq3.nPixelsArray[0] == 0)
			&& (char_values_seq3.nPixelsArray[1] == 0)
			&& (char_values_seq3.nPixelsArray[2] == 0)
			&& (char_values_seq3.nPixelsArray[3] == 0)
			&& (char_values_seq3.nPixelsArray[4] == 0))) {

		// check for Message from Player
		if (TakeMessageFromDataQueue(queue_handler_data, pxRxDataMessage)
				!= QUEUE_EMPTY) {
			if (pxRxDataMessage->cmd == stop) {

				return stopAborted; /*Session was aborted*/
			}

			else if (pxRxDataMessage->cmd == playImage) {
				return playImageAborted; /*Session was aborted*/
			}

			else if (pxRxDataMessage->cmd == pause) {
				for (;;) {
					if (TakeMessageFromDataQueue(queue_handler_data,
							pxRxDataMessage) != QUEUE_EMPTY) {
						if (pxRxDataMessage->cmd == play) {
							break;
						} else if (pxRxDataMessage->cmd == stop) {

							return stopAborted; /*Session was aborted*/
						}

						else if (pxRxDataMessage->cmd == playImage) {
							return playImageAborted;
						}

					} else {
						vTaskDelay(pdMS_TO_TICKS(20));
					}
				}
			}
		}

		// Dekrementierung der Pixelwerte

		for (index = 0; index < nDataPoints; index++) {

			if (char_values_seq3.nPixelsArray[index] != 0) {

				if (cntArray[index] == char_values_seq3.counterArray[index]) {
					char_values_seq3.colorArray[index] = decrementValue(
							char_values_seq3.colorArray[index],
							char_values_seq3.decrementArray[index]);
					cntArray[index] = 0;

					if (char_values_seq3.colorArray[index] == 0) {

						ClearCoordinate(
								char_values_seq3.coordinateArray[index][0],
								char_values_seq3.nPixelsArray[index]);
						ClearCoordinate(
								char_values_seq3.coordinateArray[index][1],
								char_values_seq3.nPixelsArray[index]);
						char_values_seq3.nPixelsArray[index]--;
						char_values_seq3.colorArray[index] = getSequenzColor(2,
								index + 1);
					} else {
						SetCoordinate(
								char_values_seq3.coordinateArray[index][0],
								char_values_seq3.nPixelsArray[index],
								char_values_seq3.colorArray[index]);
						SetCoordinate(
								char_values_seq3.coordinateArray[index][1],
								char_values_seq3.nPixelsArray[index],
								char_values_seq3.colorArray[index]);
					}
				}
				cntArray[index]++;
			}
		}

#if 0
		if (getLetterEnabled('T')) { /*only needs to be updatet when enabled*/
			updateLetterColor(nPixels1, nPixels2, nPixels3, nPixels4);
		}
#endif

		NEO_TransferPixels();

		//	vTaskDelay(pdMS_TO_TICKS(DELAY_TIME));

	}

	return aborted;
}

static void playScreensaver(void) {

	NEO_ClearAllPixel();
	NEO_TransferPixels();
	uint8_t counterRed = 0xff;
	uint8_t counterGreen = 0x0;
	uint8_t counterBlue = 0x0;
	uint32_t color = 0x0;
	uint16_t cutoff = 128;
	int z;

	for (;;) {
		if (FRTOS1_ulTaskNotifyTake(pdTRUE, 1)) {
			NEO_ClearAllPixel();
			NEO_TransferPixels();
			return;
		} else {
			//set from inner to outer
			for (int i = 1; i < 13; i++) {
				if (FRTOS1_ulTaskNotifyTake(pdTRUE, 1)) {
					NEO_ClearAllPixel();
					NEO_TransferPixels();
					return;
				} else {
					counterRed = 0xff;
					counterGreen = 0x0;
					counterBlue = 0x0;
					for (z = 0; z < cutoff; z++) {
						if (FRTOS1_ulTaskNotifyTake(pdTRUE, 1)) {
							NEO_ClearAllPixel();
							NEO_TransferPixels();
							return;
						} else {
							if (i <= 2) {
								counterRed = 0xff;
								counterGreen = z;
								counterBlue = 0x00;
							} else if (i <= 4) {
								counterRed = 0xff - z;
								counterGreen = 0xff;
								counterBlue = 0x00;
							}
							else if (i <= 6) {
								counterRed = 0x00;
								counterGreen = 0xff;
								counterBlue = z;
							} else if (i <= 8) {
								counterRed = 0x00;
								counterGreen = 0xff - z;
								counterBlue = 0xff;
							} else if (i <= 10) {
								counterRed = z;
								counterGreen = 0;
								counterBlue = 0xff;
							} else if (i <= 12) {
								counterRed = 0xff;
								counterGreen = 0;
								counterBlue = 0xff - z;
							}

							color = (counterRed << 16) | (counterGreen << 8)
									| counterBlue;
							setRingData(i, color);
							NEO_TransferPixels();
							//vTaskDelay(pdMS_TO_TICKS(20));
						}
					}

					if(z == 128){
						cutoff = 256;
					}
					else{
						cutoff = 128;
						z = 0;
					}

				}

			}
			// clear from inner to outer
			for (int k = 1; k < 13; k++) {
				if (FRTOS1_ulTaskNotifyTake(pdTRUE, 1)) {
					NEO_ClearAllPixel();
					NEO_TransferPixels();
					return;
				} else {
					clearRingData(k);
					NEO_TransferPixels();
					vTaskDelay(pdMS_TO_TICKS(200));
				}
			}

		}
	}

}

#define DELAY_PER_TICK_SEQ_2		10	/*10ms*/
static void NeoTask(void* pvParameters) {

//queue_handler = pvParameters;
	int value = -1;
	QUEUE_RESULT res = QUEUE_OK;
	uint8_t excitation = 0;
	bool sequenzAborted = 0; /*return value from sequenz, 0--> if not aborted(not stopped), 1--> if aborted(stoped)*/
	DataMessage_t * pxRxDataMessage;
	pxRxDataMessage = &xDataMessage;
	uint8_t highestColorValue;
	UpdateMessage_t * pxMessage;
	pxMessage = &xUpdateMessage;
	bool wasPaused = FALSE;

	uint32_t delayCNT = 0;

	RETURN_STATUS ret_value;

	NEO_STATUS state = IDLE_STATE;
	for (;;) {
		switch (state) {

		case IDLE_STATE:

			if (TakeMessageFromDataQueue(queue_handler_data, pxRxDataMessage)
					== QUEUE_EMPTY) {
				vTaskDelay(pdMS_TO_TICKS(100)); /*Queue is Empty*/

				if (FRTOS1_ulTaskNotifyTake(pdTRUE, 1)) {
					pxMessage->cmd = screensaver;
					state = SCREENSAVER;
				} else {
					pxMessage->cmd = stop;
				}
				if (AddMessageToUpdateQueue(queue_handler_update, pxMessage)
						!= QUEUE_OK) {
					/*Queue is somehow full*/
				}

			} else {
				if (FRTOS1_xSemaphoreTake(mutex,100) != pdTRUE) {
					/*error taking Mutex*/
					state = ERROR_STATE;
				} else {
					state = READ_NEW_CMD;
				}

			}

			break;

		case READ_NEW_CMD:
			if (pxRxDataMessage->cmd == play) {

				pxMessage->cmd = play;
				pxMessage->excitation = pxRxDataMessage->excitation;
				pxMessage->name = pxRxDataMessage->name;
				excitation = pxRxDataMessage->excitation;

				if (AddMessageToUpdateQueue(queue_handler_update, pxMessage)
						!= QUEUE_OK) {
					/*Queue is somehow full*/
				}

				state = PLAY_SEQ1;

			} else if (pxRxDataMessage->cmd == pause) {
				/*sollte eigendlich kein pause kommando hier bekommen*/
				state = IDLE_STATE;
			}

			else if (pxRxDataMessage->cmd == stop) {
				pxMessage->cmd = stop;
				if (AddMessageToUpdateQueue(queue_handler_update, pxMessage)
						!= QUEUE_OK) {
					/*Queue is somehow full*/
				} else {
					state = STOPPED;
				}

			}
			/*Commandos for Images*/
			else if (pxRxDataMessage->cmd == playImage) {

				state = DISPLAY_IMAGE;
			}

			else if (pxRxDataMessage->cmd == clearImage) {
				NEO_ClearAllPixel();
				NEO_TransferPixels();
				state = IDLE_STATE;
				if (FRTOS1_xSemaphoreGive(mutex) != pdTRUE) {
					/*could not give back the semaphore, maybe because its already given back*/
				}
				pxMessage->cmd = stop;
				if (AddMessageToUpdateQueue(queue_handler_update, pxMessage)
						!= QUEUE_OK) {
					/*Queue is somehow full*/
				}

			}

			break;

		case DISPLAY_IMAGE:

			NEO_ClearAllPixel();
			NEO_TransferPixels();
			excitation = 0;
			if (FRTOS1_xSemaphoreGive(mutex) != pdTRUE) {
				/*could not give back the semaphore, maybe because its already given back*/
			}

			pxMessage->cmd = readyForImage;
			if (AddMessageToUpdateQueue(queue_handler_update, pxMessage)
					!= QUEUE_OK) {
				/*Queue is somehow full*/
			}

			NEOA_Display_Image(pxRxDataMessage->color_data,
					pxRxDataMessage->image->biBitCount);

			state = IDLE_STATE;
			break;

		case PLAY_SEQ1:
			NEO_ClearAllPixel();
			NEO_TransferPixels();

			ret_value = playSeq1(pxRxDataMessage->char_data,
					pxRxDataMessage->color_data,
					pxRxDataMessage->image->biBitCount, excitation);

			if (ret_value == notAborted) {
				NEO_ClearAllPixel();
				NEO_TransferPixels();
				vTaskDelay(pdMS_TO_TICKS(getTiming(0)));
				state = PLAY_SEQ2;
			} else if (ret_value == stopAborted) {
				NEO_ClearAllPixel();
				NEO_TransferPixels();
				state = STOPPED;
			} else if (ret_value == playImageAborted) {
				state = DISPLAY_IMAGE;
			}

			break;

		case PLAY_SEQ2:

			playSeq2(pxRxDataMessage->char_data, excitation);

			delayCNT = rint((float) getTiming(1) / (DELAY_PER_TICK_SEQ_2));

			state = PLAY_SEQ3;
			for (int i = 0; i < delayCNT; i++) {

				if (TakeMessageFromDataQueue(queue_handler_data,
						pxRxDataMessage) != QUEUE_EMPTY) {
					if (pxRxDataMessage->cmd == stop) {
						state = STOPPED;
						break;
					}

					else if (pxRxDataMessage->cmd == playImage) {
						state = DISPLAY_IMAGE;
						break;
					}

					else if (pxRxDataMessage->cmd == pause) {
						for (;;) {
							if (TakeMessageFromDataQueue(queue_handler_data,
									pxRxDataMessage) != QUEUE_EMPTY) {
								if (pxRxDataMessage->cmd == play) {
									break;
								} else if (pxRxDataMessage->cmd == stop) {
									state = STOPPED;
									wasPaused = TRUE;
									break;
								}

								else if (pxRxDataMessage->cmd == playImage) {
									state = DISPLAY_IMAGE;
									wasPaused = TRUE;
								}
							} else {
								vTaskDelay(pdMS_TO_TICKS(20));
							}
						}
					}
				}
				if (wasPaused) {
					break;
				}
				vTaskDelay(pdMS_TO_TICKS(DELAY_PER_TICK_SEQ_2));
			}

			break;

		case PLAY_SEQ3:

			vTaskDelay(pdMS_TO_TICKS(getTiming(2)));

			ret_value = playSeq3(pxRxDataMessage->char_data, excitation);

			if (ret_value == notAborted) {
				if (FRTOS1_xSemaphoreGive(mutex) != pdTRUE) {
					state = ERROR_STATE;
				} else {
					pxMessage->cmd = stop;
					if (AddMessageToUpdateQueue(queue_handler_update, pxMessage)
							!= QUEUE_OK) {
						/*Queue is somehow full*/
					}
					vTaskDelay(pdMS_TO_TICKS(getTiming(3)));
					state = IDLE_STATE;
				}
			} else if (ret_value == stopAborted) {
				NEO_ClearAllPixel();
				NEO_TransferPixels();
				state = STOPPED;
			}

			else if (ret_value == playImageAborted) {
				state = DISPLAY_IMAGE;
			}

			break;

		case STOPPED:
			NEO_ClearAllPixel();
			NEO_TransferPixels();
			excitation = 0;
			if (FRTOS1_xSemaphoreGive(mutex) != pdTRUE) {
				/*could not give back the semaphore, maybe because its already given back*/
			} else {
				pxMessage->cmd = stop;
				if (AddMessageToUpdateQueue(queue_handler_update, pxMessage)
						!= QUEUE_OK) {
					/*Queue is somehow full*/
				}
				//	vTaskDelay(pdMS_TO_TICKS(getTiming(3)));
				state = IDLE_STATE;
			}
			state = IDLE_STATE;
			break;

		case ERROR_STATE:
			for (;;) {
				vTaskDelay(pdMS_TO_TICKS(50));
			}
			break;

		case SCREENSAVER:

			playScreensaver();
			state = IDLE_STATE;

		}

#if 0
		CLS1_SendStr((unsigned char*) "\r\n ", CLS1_GetStdio()->stdOut);
		CLS1_SendStr((unsigned char*) "taken out Data from :", CLS1_GetStdio()->stdOut);
		CLS1_SendStr((unsigned char*) pxRxDataMessage->name, CLS1_GetStdio()->stdOut);
		vTaskDelay(pdMS_TO_TICKS(1000)); /*Queue is Empty*/
		// FRTOS1_vPortFree(pxRxDataMessage->char_data);
		// FRTOS1_vPortFree(pxRxDataMessage->color_data);
		// FRTOS1_vPortFree(polle);
#endif
	}
}

uint8_t SetTrail(uint32_t color, uint32_t end, uint32_t nofTail,
		uint8_t dimmPercent, uint16_t delayMs) {
	NEO_PixelIdxT pixel;
	unsigned int i;

	for (pixel = 1; pixel <= end + nofTail + 1; pixel++) {
		/* move head */
		if (pixel <= end) {
			SetCoordinate(pixel, 13, color);
			SetCoordinate(pixel, 12, color);
		}
		/* clear tail pixel */
		if ((pixel > (nofTail + 1)) && pixel - (nofTail + 1) <= end) {
			ClearCoordinate(pixel - (nofTail + 1), 13);
			ClearCoordinate(pixel - (nofTail + 1), 12);
		}
		/* dim remaining tail pixel */
		for (i = 0; i < nofTail; i++) {
			if (pixel > i && pixel - (i + 1) <= end) {
				DimmPercentPixel(pixel - (i + 1), 13, dimmPercent);
				DimmPercentPixel(pixel - (i + 1), 12, dimmPercent);
				//NEO_DimmPercentPixel(1,lookUpMatrix[12][pixel-(i+1)+1], dimmPercent);
			}
		}
		NEO_TransferPixels();
		vTaskDelay(pdMS_TO_TICKS(delayMs));
	}
	return ERR_OK;
}

void NEOA_Init(void* queue_handler) {
	NEO_Init();
	PIXDMA_Init();
	if (xTaskCreate(NeoTask, /* pointer to the task */
	"Neo", /* task name for kernel awareness debugging */
	2000 / sizeof(StackType_t), /* task stack size */
	(void*) queue_handler, /* optional task startup argument */
	tskIDLE_PRIORITY + 3, /* initial priority */
	(xTaskHandle*) NULL /* optional task handle to create */
	) != pdPASS) {
		/*lint -e527 */
		for (;;) {
		}; /* error! probably out of memory */
		/*lint +e527 */
	}
}

#endif /* PL_CONFIG_HAS_NEO_PIXEL */

