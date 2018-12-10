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

#endif

#define WIDTH_PIXELS (3*8)  /* 3 8x8 tiles */
#define HEIGHT_PIXELS (8)   /* 1 tile */
#define PIXEL_NOF_X   (24)
#define PIXEL_NOF_Y   (8)

static uint8_t NEOA_LightLevel = 1; /* default 1% */
static bool NEOA_isAutoLightLevel = TRUE;
static bool NEOA_useGammaCorrection = TRUE;
xQueueHandle queue_handler; /*QueueHandler declared in Message.h*/
xQueueHandle queue_handler_Navigation; /*QueueHandler declared in Message.h*/
xSemaphoreHandle mutex; /*SemaphoreHandler declared in Message*/
extern uint8_t ImageDataBuffer[2500];



static void SetPixel(int x, int y, uint32_t color) {
	/* 0, 0 is left upper corner */
	/* single lane, 3x64 modules from left to right */
	int pos;

	pos = ((x / 8) * 64) /* position in tile */
	+ (x % 8) /* position in row */
	+ (y) * 8; /* add y offset */
	NEO_SetPixelColor(0, pos, color);
}

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
		lenght_lane_0 = sizeof(ring_6[0]) / sizeof(ring_6[0][0]);
		lenght_lane_1 = 16;
		lenght_lane_2 = sizeof(ring_6[2]) / sizeof(ring_6[0][0]);

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

#if PL_CONFIG_HAS_NEO_SHADOW_BOX
static void Layer(int layer, uint32_t color) {
	int y, x;

	y = layer;
	for(x=0;x<WIDTH_PIXELS;x++) {
		SetPixel(x, y, color);
	}
}

static int32_t Rainbow(int32_t numOfSteps, int32_t step) {
	float r = 0.0;
	float g = 0.0;
	float b = 0.0;
	float h = (double)step / numOfSteps;
	int i = (int32_t)(h * 6);
	float f = h * 6.0 - i;
	float q = 1 - f;

	switch (i % 6) {
		case 0:
		r = 1;
		g = f;
		b = 0;
		break;
		case 1:
		r = q;
		g = 1;
		b = 0;
		break;
		case 2:
		r = 0;
		g = 1;
		b = f;
		break;
		case 3:
		r = 0;
		g = q;
		b = 1;
		break;
		case 4:
		r = f;
		g = 0;
		b = 1;
		break;
		case 5:
		r = 1;
		g = 0;
		b = q;
		break;
	}
	r *= 255;
	g *= 255;
	b *= 255;
	return (((int)r)<<16)|(((int)g)<<8)+(int)b;
}
#endif

#if PL_CONFIG_HAS_NEO_SHADOW_BOX
static void NeoTask(void* pvParameters) {
	int i, cntr, val = 0;
	int inc = 1;
	int red, green, blue;
	NEO_Color color;
	uint8_t rowStartStep[8] = {0, 20, 50, 70, 90, 130, 150, 170};

	(void)pvParameters; /* parameter not used */
	cntr = 0;

	for(;;) {
		int row;
		int32_t color;

		for(row=0; row<8; row++) {
			color = Rainbow(256,rowStartStep[row]);
			rowStartStep[row]++;
			color = NEO_SetColorPercent(color, 10); /* reduce brightness */
			Layer(row, color);
		}
		NEO_TransferPixels();
		vTaskDelay(pdMS_TO_TICKS(30));
	}
}
#endif

#if PL_CONFIG_HAS_NEO_HOUR_GLASS

#define AREA_MIN_X  (0)
#define AREA_NOF_X  (PIXEL_NOF_X+2) /* with border */
#define AREA_MIN_Y  (0)
#define AREA_NOF_Y  (PIXEL_NOF_Y+2) /* with border

/* area:
 * 1: pixel set
 * 0: pixel not set
 * 0xff: border
 */
static uint8_t area[PIXEL_NOF_X+2][PIXEL_NOF_Y+2]; /* area with one pixel virtual border */

static void DrawArea(void) {
	int x, y;
	NEO_Color level;

	if (NEOA_useGammaCorrection) {
		level = NEO_GammaCorrect8(NEOA_LightLevel);
		if (level==0) {
			level = 1; /* have minimal light level */
		}
	} else {
		level = NEOA_LightLevel;
	}

	for(x=1;x<AREA_NOF_X-1;x++) { /* start inside border */
		for(y=1;y<AREA_NOF_Y-1;y++) {
			if (area[x][y]==0) { /* clear pixel area */
				SetPixel(x-1, y-1, NEO_MAKE_COLOR_RGB(0,0,level));
			} else if (area[x][y]==1) { /* set pixel area */
				SetPixel(x-1, y-1, NEO_MAKE_COLOR_RGB(0,level,0));
			} else if (area[x][y]==0xff) {
				SetPixel(x-1, y-1, NEO_MAKE_COLOR_RGB(0,0,0)); /* border, obstacle area */
			}
		}
	}
}

static void InitHourGlass(int nofGrains) {
	int x, y;

	for(x=AREA_MIN_X;x<AREA_NOF_X;x++) {
		for(y=AREA_MIN_Y;y<AREA_NOF_Y;y++) {
			area[x][y] = 0; /* init */
		}
	}
	/* border */
	for(x=AREA_MIN_X;x<AREA_NOF_X;x++) {
		area[x][0] = 0xff;
		area[x][AREA_NOF_Y-1] = 0xff;
	}
	for(y=AREA_MIN_Y;y<AREA_NOF_Y;y++) {
		area[0][y] = 0xff;
		area[AREA_NOF_X-1][y] = 0xff;
	}
	/* add sand... */
	for(x=AREA_MIN_X+1;x<AREA_NOF_X-1;x++) {
		for(y=AREA_MIN_Y+1;y<AREA_NOF_Y-1;y++) {
			area[x][y] = 1;
			nofGrains--;
			if (nofGrains==0) {
				break;
			}
		}
		if (nofGrains==0) {
			break;
		}
	}

	/* hourglass obstacle */
	area[9][1] = 0xff;
	area[10][1] = 0xff;
	area[11][1] = 0xff;
	area[12][1] = 0xff;
	area[13][1] = 0xff;
	area[14][1] = 0xff;
	area[15][1] = 0xff;

	area[10][2] = 0xff;
	area[11][2] = 0xff;
	area[12][2] = 0xff;
	area[13][2] = 0xff;
	area[14][2] = 0xff;

	area[11][3] = 0xff;
	area[12][3] = 0xff;
	area[13][3] = 0xff;

	area[12][4] = 0xff;

	area[10][8] = 0xff;
	area[11][8] = 0xff;
	area[12][8] = 0xff;
	area[13][8] = 0xff;
	area[14][8] = 0xff;
	area[15][8] = 0xff;
	area[16][8] = 0xff;

	area[11][7] = 0xff;
	area[12][7] = 0xff;
	area[13][7] = 0xff;
	area[14][7] = 0xff;
	area[15][7] = 0xff;

	area[12][6] = 0xff;
	area[13][6] = 0xff;
	area[14][6] = 0xff;

	area[13][5] = 0xff;
}

#define N (3)
/* An inplace function to rotate a N x N matrix
 by 90 degrees in cw (clockwise) or anti-clockwise direction */
static void rotateMatrixAntiClockWise(int mat[][N], bool cw) {
	/* source: https://www.geeksforgeeks.org/inplace-rotate-square-matrix-by-90-degrees/ */
// Consider all squares one by one
	for (int x = 0; x < N / 2; x++) { /* Consider elements in group of 4 in current square */
		for (int y = x; y < N-x-1; y++) {
			int temp = mat[x][y]; /* store current cell in temp variable */
			if (cw) { /* clockwise rotation */
				mat[x][y] = mat[N-1-y][x]; /* move bottom left to top left */
				mat[N-1-y][x] = mat[N-1-x][N-1-y]; /* move bottom right to bottom left */
				mat[N-1-x][N-1-y] = mat[y][N-1-x]; /* move top right to bottom right */
				mat[y][N-1-x] = temp; /* assign temp to top right */
			} else {
				mat[x][y] = mat[y][N-1-x]; /* move values from top right to top left */
				mat[y][N-1-x] = mat[N-1-x][N-1-y]; /* move values from bottom right to top right */
				mat[N-1-x][N-1-y] = mat[N-1-y][x]; /* move values from bottom left to bottom right */
				mat[N-1-y][x] = temp; /* assign temp to bottom left */
			}
		}
	}
}

static void ApplyDownForce(uint8_t *(matrix[3][3])) {
	/* x: don't care
	 * i: item to check
	 * 0: free space
	 * 1: occupied space
	 */
	if (*(matrix[1][1])!=1) {
		return; /* precondition: middle element shall be set! */
	}
	/* xxx    xxx
	 * xix => x0x
	 * x0x    xix
	 */
	if (*(matrix[2][1])==0) {
		*(matrix[2][1]) = *(matrix[1][1]);
		*(matrix[1][1]) = 0;
		return;
	}
	/* 0xx    0xx
	 * 0ix => 00x
	 * 01x    i1x
	 */
	if (*(matrix[2][0])==0 && *(matrix[1][0])==0 && *(matrix[0][0])!=1) {
		*(matrix[2][0]) = *(matrix[1][1]);
		*(matrix[1][1]) = 0;
		return;
	}
	/* xx0    xx0
	 * xi0 => x00
	 * x10    x1i
	 */
	if (*(matrix[2][2])==0 && *(matrix[1][2])==0 && *(matrix[0][2])!=1) {
		*(matrix[2][2]) = *(matrix[1][1]);
		*(matrix[1][1]) = 0;
		return;
	}
	/* xxx    xxx
	 * xib => x0b
	 * xb0    xbi
	 */
	if (*(matrix[2][2])==0 && *(matrix[1][2])==0xff && *(matrix[2][1])==0xff) {
		*(matrix[2][2]) = *(matrix[1][1]);
		*(matrix[1][1]) = 0;
		return;
	}
	/* xxx    xxx
	 * bix => b0x
	 * 0bx    xbx
	 */
	if (*(matrix[2][0])==0 && *(matrix[1][0])==0xff && *(matrix[2][1])==0xff) {
		*(matrix[2][0]) = *(matrix[1][1]);
		*(matrix[1][1]) = 0;
		return;
	}
}

static void ApplyGravity(int gx, int gy) {
	uint8_t *(matrix[3][3]);
	int x, y;
	bool changed;

	if (gx==0 && gy==1) { /* down */
		/* scan from bottom up */
		for(x=1;x<AREA_NOF_X-1;x++) {
			for(y=AREA_NOF_Y-2;y>0;y--) {
				if (area[x][y]==1) { /* we have an item */
					matrix[0][0] = &area[x-1][y-1];
					matrix[0][1] = &area[x][y-1];
					matrix[0][2] = &area[x+1][y-1];
					matrix[1][0] = &area[x-1][y];
					matrix[1][1] = &area[x][y];
					matrix[1][2] = &area[x+1][y];
					matrix[2][0] = &area[x-1][y+1];
					matrix[2][1] = &area[x][y+1];
					matrix[2][2] = &area[x+1][y+1];
					ApplyDownForce(matrix);
				}
			}
		} /* for */
	} else if (gx==1 && gy==1) { /* down-right */
		/* scan from bottom up */
		for(x=AREA_NOF_X-2;x>0;x--) {
			for(y=AREA_NOF_Y-2;y>0;y--) {
				if (area[x][y]==1) { /* we have an item */
					matrix[0][0] = &area[x-1][y];
					matrix[0][1] = &area[x-1][y+1];
					matrix[0][2] = &area[x][y-1];
					matrix[1][0] = &area[x-1][y+1];
					matrix[1][1] = &area[x][y];
					matrix[1][2] = &area[x+1][y-1];
					matrix[2][0] = &area[x][y+1];
					matrix[2][1] = &area[x+1][y+1];
					matrix[2][2] = &area[x+1][y];
					ApplyDownForce(matrix);
				}
			}
		} /* for */
	} else if (gx==1 && gy==0) { /* right */
		/* scan from right to left */
		for(x=AREA_NOF_X-2;x>0;x--) {
			for(y=AREA_NOF_Y-2;y>0;y--) {
				if (area[x][y]==1) { /* we have an item */
					matrix[0][0] = &area[x-1][y+1];
					matrix[0][1] = &area[x-1][y];
					matrix[0][2] = &area[x-1][y-1];
					matrix[1][0] = &area[x][y+1];
					matrix[1][1] = &area[x][y];
					matrix[1][2] = &area[x][y-1];
					matrix[2][0] = &area[x+1][y+1];
					matrix[2][1] = &area[x+1][y];
					matrix[2][2] = &area[x+1][y-1];
					ApplyDownForce(matrix);
				}
			}
		} /* for */
	} else if (gx==1 && gy==-1) { /* right-up */
		/* scan from right to left */
		for(x=AREA_NOF_X-2;x>0;x--) {
			for(y=AREA_NOF_Y-2;y>0;y--) {
				if (area[x][y]==1) { /* we have an item */
					matrix[0][0] = &area[x][y+1];
					matrix[0][1] = &area[x-1][y+1];
					matrix[0][2] = &area[x-1][y];
					matrix[1][0] = &area[x+1][y+1];
					matrix[1][1] = &area[x][y];
					matrix[1][2] = &area[x-1][y-1];
					matrix[2][0] = &area[x+1][y];
					matrix[2][1] = &area[x+1][y-1];
					matrix[2][2] = &area[x][y-1];
					ApplyDownForce(matrix);
				}
			}
		} /* for */
	} else if (gx==0 && gy==-1) { /* up */
		/* scan from top to bottom */
		for(x=1; x<AREA_NOF_X-1;x++) {
			for(y=1;y<AREA_NOF_Y-1;y++) {
				if (area[x][y]==1) { /* we have an item */
					matrix[0][0] = &area[x+1][y+1];
					matrix[0][1] = &area[x][y+1];
					matrix[0][2] = &area[x-1][y+1];
					matrix[1][0] = &area[x+1][y];
					matrix[1][1] = &area[x][y];
					matrix[1][2] = &area[x-1][y];
					matrix[2][0] = &area[x+1][y-1];
					matrix[2][1] = &area[x][y-1];
					matrix[2][2] = &area[x-1][y-1];
					ApplyDownForce(matrix);
				}
			}
		} /* for */
	} else if (gx==-1 && gy==-1) { /* left-up */
		/* scan from top to bottom */
		for(x=1; x<AREA_NOF_X-1;x++) {
			for(y=1;y<AREA_NOF_Y-1;y++) {
				if (area[x][y]==1) { /* we have an item */
					matrix[0][0] = &area[x+1][y];
					matrix[0][1] = &area[x+1][y+1];
					matrix[0][2] = &area[x][y+1];
					matrix[1][0] = &area[x+1][y-1];
					matrix[1][1] = &area[x][y];
					matrix[1][2] = &area[x-1][y+1];
					matrix[2][0] = &area[x][y-1];
					matrix[2][1] = &area[x-1][y-1];
					matrix[2][2] = &area[x-1][y];
					ApplyDownForce(matrix);
				}
			}
		} /* for */
	} else if (gx==-1 && gy==0) { /* left */
		/* scan from top to down */
		for(x=1;x<AREA_NOF_X-1;x++) {
			for(y=1;y<AREA_NOF_Y-1;y++) {
				if (area[x][y]==1) { /* we have an item */
					matrix[0][0] = &area[x+1][y-1];
					matrix[0][1] = &area[x+1][y];
					matrix[0][2] = &area[x+1][y+1];
					matrix[1][0] = &area[x][y-1];
					matrix[1][1] = &area[x][y];
					matrix[1][2] = &area[x][y+1];
					matrix[2][0] = &area[x-1][y-1];
					matrix[2][1] = &area[x-1][y];
					matrix[2][2] = &area[x-1][y+1];
					ApplyDownForce(matrix);
				}
			}
		} /* for */

	} else if (gx==-1 && gy==1) { /* left-down */
		/* scan from bottom to topo */
		for(x=1;x<AREA_NOF_X-1;x++) {
			for(y=AREA_NOF_Y-2;y>0;y--) {
				if (area[x][y]==1) { /* we have an item */
					matrix[0][0] = &area[x][y+1];
					matrix[0][1] = &area[x+1][y+1];
					matrix[0][2] = &area[x+1][y];
					matrix[1][0] = &area[x-1][y-1];
					matrix[1][1] = &area[x][y];
					matrix[1][2] = &area[x+1][y+1];
					matrix[2][0] = &area[x-1][y];
					matrix[2][1] = &area[x-1][y+1];
					matrix[2][2] = &area[x][y+1];
					ApplyDownForce(matrix);
				}
			}
		} /* for */
	} /* if */
}

static void NeoTask(void* pvParameters) {
#if PL_CONFIG_HAS_MMA8451
	int16_t xmg, ymg, zmg;
#endif
	int xg = 0, yg = 1;
#if PL_CONFIG_HAS_TSL2561
	uint32_t lux;
	NEOA_isAutoLightLevel = TRUE;
	TickType_t tickCnt, lastLightMeasurementTickCnt = 0;
	int new_light_level;
#else
	NEOA_isAutoLightLevel = FALSE;
#endif

	(void)pvParameters; /* parameter not used */
	InitHourGlass(64);
	NEOA_LightLevel = 50;
#if PL_CONFIG_HAS_TSL2561
	new_light_level = NEOA_LightLevel;
#endif
	for(;;) {
		DrawArea();
		NEO_TransferPixels();
		vTaskDelay(pdMS_TO_TICKS(200));
#if PL_CONFIG_HAS_TSL2561
		tickCnt = xTaskGetTickCount();
		if (NEOA_isAutoLightLevel && tickCnt-lastLightMeasurementTickCnt > pdMS_TO_TICKS(1000)) { /* every 2 seconds */
			lastLightMeasurementTickCnt = tickCnt;
			if (TSL1_GetLux(&lux)==ERR_OK) {
				/* 1 lux is pretty dark, 50 is rather dark, 200 is about office light */
				if (NEOA_useGammaCorrection) {
					new_light_level = lux;
				} else {
					new_light_level = lux/16 - 5; /* scale down with an offset */
				}
				if (new_light_level>100) {
					new_light_level = 100;
				} else if (new_light_level < 1) {
					new_light_level = 1;
				}
			}
		} /* if */
		/* gradually adopt the light level */
		if (NEOA_isAutoLightLevel) {
			if (new_light_level>NEOA_LightLevel) {
				NEOA_LightLevel++;
			} else if (new_light_level<NEOA_LightLevel) {
				NEOA_LightLevel--;
			}
		}
#endif
#if PL_CONFIG_HAS_MMA8451
		xmg = MMA1_GetXmg();
		ymg = MMA1_GetYmg();
		zmg = MMA1_GetZmg();
		if(xmg>200) {
			xg = 1;
		} else if (xmg<-200) {
			xg = -1;
		} else {
			xg = 0;
		}
		if(ymg>200) {
			yg = -1;
		} else if (ymg<-200) {
			yg = 1;
		} else {
			yg = 0;
		}
#endif
		ApplyGravity(xg, yg);
	}
}
#endif

#if PL_CONFIG_HAS_NEO_THERMAL_CAM
//low range of the sensor (this will be blue on the screen)
#define MINTEMP 20

//high range of the sensor (this will be red on the screen)
#define MAXTEMP 30

static float pixels[AMG88xx_PIXEL_ROWS][AMG88xx_PIXEL_COLS];

#define INTERPOLATED_LED_COLS 8
#define INTERPOLATED_LED_ROWS 8
static float dest_2d_LED[INTERPOLATED_LED_ROWS * INTERPOLATED_LED_COLS];

#if PL_CONFIG_HAS_SSD1351 && !PL_CONFIG_HAS_GUI
#define INTERPOLATED_LCD_IMAGE_SIZE   (128)  /* square size */
#define INTERPOLATED_LCD_BLOCK_SIZE   (4)
#define INTERPOLATED_LCD_COLS         (INTERPOLATED_LCD_IMAGE_SIZE/INTERPOLATED_LCD_BLOCK_SIZE)
#define INTERPOLATED_LCD_ROWS         (INTERPOLATED_LCD_IMAGE_SIZE/INTERPOLATED_LCD_BLOCK_SIZE)
static float dest_2d_LCD[INTERPOLATED_LCD_ROWS * INTERPOLATED_LCD_COLS];
#endif

static void drawpixelsLED(float *p, uint8_t rows, uint8_t cols, uint8_t boxWidth, uint8_t boxHeight) {
	int colorTemp;
	for (int y=0; y<rows; y++) {
		for (int x=0; x<cols; x++) {
			float val = get_point(p, rows, cols, x, y);
			if(val >= MAXTEMP) {
				colorTemp = MAXTEMP;
			} else if(val <= MINTEMP) {
				colorTemp = MINTEMP;
			} else {
				colorTemp = val;
			}
			uint8_t colorIndex = UTIL1_map(colorTemp, MINTEMP, MAXTEMP, 0, 255);
			colorIndex = UTIL1_constrain(colorIndex, 0, 255);
			//draw the pixels!
			SetPixel(x, y, NEO_MAKE_COLOR_RGB((colorIndex*15/100), 0, (255-colorIndex)*15/100));
		}
	}
}

#if PL_CONFIG_HAS_SSD1351 && !PL_CONFIG_HAS_GUI
//the colors we will be using
const uint16_t camColors[] = {0x480F,
	0x400F,0x400F,0x400F,0x4010,0x3810,0x3810,0x3810,0x3810,0x3010,0x3010,
	0x3010,0x2810,0x2810,0x2810,0x2810,0x2010,0x2010,0x2010,0x1810,0x1810,
	0x1811,0x1811,0x1011,0x1011,0x1011,0x0811,0x0811,0x0811,0x0011,0x0011,
	0x0011,0x0011,0x0011,0x0031,0x0031,0x0051,0x0072,0x0072,0x0092,0x00B2,
	0x00B2,0x00D2,0x00F2,0x00F2,0x0112,0x0132,0x0152,0x0152,0x0172,0x0192,
	0x0192,0x01B2,0x01D2,0x01F3,0x01F3,0x0213,0x0233,0x0253,0x0253,0x0273,
	0x0293,0x02B3,0x02D3,0x02D3,0x02F3,0x0313,0x0333,0x0333,0x0353,0x0373,
	0x0394,0x03B4,0x03D4,0x03D4,0x03F4,0x0414,0x0434,0x0454,0x0474,0x0474,
	0x0494,0x04B4,0x04D4,0x04F4,0x0514,0x0534,0x0534,0x0554,0x0554,0x0574,
	0x0574,0x0573,0x0573,0x0573,0x0572,0x0572,0x0572,0x0571,0x0591,0x0591,
	0x0590,0x0590,0x058F,0x058F,0x058F,0x058E,0x05AE,0x05AE,0x05AD,0x05AD,
	0x05AD,0x05AC,0x05AC,0x05AB,0x05CB,0x05CB,0x05CA,0x05CA,0x05CA,0x05C9,
	0x05C9,0x05C8,0x05E8,0x05E8,0x05E7,0x05E7,0x05E6,0x05E6,0x05E6,0x05E5,
	0x05E5,0x0604,0x0604,0x0604,0x0603,0x0603,0x0602,0x0602,0x0601,0x0621,
	0x0621,0x0620,0x0620,0x0620,0x0620,0x0E20,0x0E20,0x0E40,0x1640,0x1640,
	0x1E40,0x1E40,0x2640,0x2640,0x2E40,0x2E60,0x3660,0x3660,0x3E60,0x3E60,
	0x3E60,0x4660,0x4660,0x4E60,0x4E80,0x5680,0x5680,0x5E80,0x5E80,0x6680,
	0x6680,0x6E80,0x6EA0,0x76A0,0x76A0,0x7EA0,0x7EA0,0x86A0,0x86A0,0x8EA0,
	0x8EC0,0x96C0,0x96C0,0x9EC0,0x9EC0,0xA6C0,0xAEC0,0xAEC0,0xB6E0,0xB6E0,
	0xBEE0,0xBEE0,0xC6E0,0xC6E0,0xCEE0,0xCEE0,0xD6E0,0xD700,0xDF00,0xDEE0,
	0xDEC0,0xDEA0,0xDE80,0xDE80,0xE660,0xE640,0xE620,0xE600,0xE5E0,0xE5C0,
	0xE5A0,0xE580,0xE560,0xE540,0xE520,0xE500,0xE4E0,0xE4C0,0xE4A0,0xE480,
	0xE460,0xEC40,0xEC20,0xEC00,0xEBE0,0xEBC0,0xEBA0,0xEB80,0xEB60,0xEB40,
	0xEB20,0xEB00,0xEAE0,0xEAC0,0xEAA0,0xEA80,0xEA60,0xEA40,0xF220,0xF200,
	0xF1E0,0xF1C0,0xF1A0,0xF180,0xF160,0xF140,0xF100,0xF0E0,0xF0C0,0xF0A0,
	0xF080,0xF060,0xF040,0xF020,0xF800,};

static void drawpixelsLCD(float *p, uint8_t rows, uint8_t cols, uint8_t boxWidth, uint8_t boxHeight) {
	int colorTemp;
	for (int y=0; y<rows; y++) {
		for (int x=0; x<cols; x++) {
			float val = get_point(p, rows, cols, x, y);
			if(val >= MAXTEMP) {
				colorTemp = MAXTEMP;
			} else if(val <= MINTEMP) {
				colorTemp = MINTEMP;
			} else {
				colorTemp = val;
			}
			uint8_t colorIndex = UTIL1_map(colorTemp, MINTEMP, MAXTEMP, 0, 255);
			colorIndex = UTIL1_constrain(colorIndex, 0, 255);
			//draw the pixels!
			GDisp1_DrawFilledBox(boxWidth*x, boxHeight*y, boxWidth, boxHeight, camColors[colorIndex]);
		}
	}
	GDisp1_UpdateFull();
}
#endif

static void NeoTask(void* pvParameters) {
	(void)pvParameters; /* parameter not used */
	vTaskDelay(pdMS_TO_TICKS(1000)); /* give other tasks time to start */
#if PL_CONFIG_HAS_AMG8833
	if (AMG_Init()!=ERR_OK) {
		CLS1_SendStr((uint8_t*)"Failed initializing AMG8833!\r\n", CLS1_GetStdio()->stdErr);
	}
#endif
#if PL_CONFIG_HAS_SSD1351 && !PL_CONFIG_HAS_GUI
	LCD1_Init();
	GDisp1_Clear();
	GDisp1_UpdateFull();
#endif
	for(;;) {
		/* max 10 Hz */
		if (AMG88xx_readPixels((float*)pixels, AMG88xx_PIXEL_ROWS*AMG88xx_PIXEL_COLS)!=ERR_OK) {
			CLS1_SendStr((uint8_t*)"Failed AMG88xx_readPixels()!\r\n", CLS1_GetStdio()->stdErr);
		} else {
			AMG88xx_FlipHorizontal(pixels);
			/* draw image on 8x8 LEDs */
			//interpolate_image((float*)pixels, AMG88xx_PIXEL_ROWS, AMG88xx_PIXEL_COLS, dest_2d_LED, INTERPOLATED_LED_ROWS, INTERPOLATED_LED_COLS);
			//drawpixelsLED(dest_2d_LED, INTERPOLATED_LED_ROWS, INTERPOLATED_LED_COLS, 1, 1);
			drawpixelsLED((float*)pixels, AMG88xx_PIXEL_ROWS, AMG88xx_PIXEL_COLS, 1, 1);
#if 1 && PL_CONFIG_HAS_SSD1351 && !PL_CONFIG_HAS_GUI
			/* draw image on LCD */
			interpolate_image((float*)pixels, AMG88xx_PIXEL_ROWS, AMG88xx_PIXEL_COLS, dest_2d_LCD, INTERPOLATED_LCD_ROWS, INTERPOLATED_LCD_COLS);
			drawpixelsLCD(dest_2d_LCD, INTERPOLATED_LCD_ROWS, INTERPOLATED_LCD_COLS, INTERPOLATED_LCD_BLOCK_SIZE, INTERPOLATED_LCD_BLOCK_SIZE);
#endif
		}
		NEO_TransferPixels();
		vTaskDelay(pdMS_TO_TICKS(100)); /* max 10 Hz */
	} /* for */
}
#endif /* PL_CONFIG_HAS_NEO_THERMAL_CAM */

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
	uint32_t position;
	int j = 0;
	int k = 0;
	int i = 0;

	Navigation_t* rxNav;
	rxNav = &xNavigation;

	uint8_t res = ERR_OK;
	NEO_ClearAllPixel();
	NEO_TransferPixels();
	for (j = 0; j < NEOC_NOF_LANES; j++) {
		for (k = 0; k < SINGLE_MATRIX_SIDE_LENGTH; k++) {
			for (i = 0; i < MATRIX_RES; i++) {
				position = lookUpMatrix[k][i];
				res = NEO_SetPixelColor(j, position, 0x002020);
				if (res != ERR_OK) {
					break; /*Something went wrong*/

				}

				if (NEO_TransferPixels() != ERR_OK) {
					break; /*Something went wrong*/
				}
				if (TakeNavigationFromQueue(queue_handler_Navigation, rxNav)
						!= QUEUE_EMPTY) {
					switch (rxNav->menu) {
					case pause:
						for (;;) {
							if (TakeNavigationFromQueue(
									queue_handler_Navigation, rxNav)
									!= QUEUE_EMPTY) {
								switch (rxNav->menu) {
								case play:
									break;
								case stop:
									NEO_ClearAllPixel();
									NEO_TransferPixels();
									goto finish;
								}

							} else {
								vTaskDelay(pdMS_TO_TICKS(10));
							}
						}
					}
				}
				vTaskDelay(pdMS_TO_TICKS(100));

			}
		}

	}

	finish: return res;

}

uint8_t NEOA_Display_Image(BMPImage* image) {

	BMPImage* rxImage;
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
	int j = 0;
	int k = 0;
	int i = 0;
	char* data;

	NEO_ClearAllPixel();
	NEO_TransferPixels();





//	size = ((image->biWidth) * (image->biHeight));

	for (j = 0; j < NEOC_NOF_LANES; j++) {
		for (k = 0; k < SINGLE_MATRIX_SIDE_LENGTH; k++) {
			for (i = 0; i < MATRIX_RES; i++) {
				position = lookUpMatrix[k][i];
				red = (image->data[cnt]);
				green = (image->data[cnt + 1]);
				blue = (image->data[cnt + 2]);
				colorValue = (red << 16) + (green << 8) + (blue);
				NEO_SetPixelColor(j, position, colorValue);
				cnt = cnt + ((image->biBitCount) / 8);


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

static void NeoTask(void* pvParameters) {

	queue_handler = pvParameters;
	int value = -1;
	QUEUE_RESULT res = QUEUE_OK;
	Message_t *pxRxedMessage;
	pxRxedMessage = &xMessage;

	uint32_t size;
	uint32_t position = 0;
	uint8_t red = 0;
	uint8_t green = 0;
	uint8_t blue = 0;
	uint32_t colorValue = 0;
	uint8_t lane = 0;
	uint32_t color;
	uint32_t cnt = 0;
	int j = 0;
	int k = 0;
	int i = 0;
	char ** names;



	for (;;) {
		if (TakeMessageFromQueue(queue_handler, pxRxedMessage) == QUEUE_EMPTY) {
			vTaskDelay(pdMS_TO_TICKS(100)); /*Queue is Empty*/
		} else {
			CLS1_SendStr((unsigned char*) "\r\n ", CLS1_GetStdio()->stdOut);
			if (FRTOS1_xSemaphoreTake(mutex,pdMS_TO_TICKS(10)) != pdTRUE) {
				/*Access to Mutex denied*/
				CLS1_SendStr((unsigned char*) "Mutex nicht erhalten\r\n ",
						CLS1_GetStdio()->stdOut);

			} else {
				switch (pxRxedMessage->modus) {

				case ALL:

					names = getNamelist();
					SetCoordinate(8,1,0xff00ff);
					SetCoordinate(9,1,0xff00ff);
					SetCoordinate(8,2,0xff00ff);
					SetCoordinate(9,2,0xff00ff);
					SetCoordinate(8,3,0xff00ff);
					SetCoordinate(9,3,0xff00ff);
					SetCoordinate(8,4,0xff00ff);
					SetCoordinate(9,4,0xff00ff);
					SetCoordinate(8,5,0xff00ff);
					SetCoordinate(9,5,0xff00ff);
					SetCoordinate(8,6,0xff00ff);
					SetCoordinate(9,6,0xff00ff);
					SetCoordinate(8,7,0xff00ff);
					SetCoordinate(9,7,0xff00ff);
					SetCoordinate(8,8,0xff00ff);
					SetCoordinate(9,8,0xff00ff);
					NEO_TransferPixels();



/*

					for (j = 0; j < NEOC_NOF_LANES; j++) {
						for (k = 0; k < SINGLE_MATRIX_SIDE_LENGTH; k++) {
							for (i = 0; i < MATRIX_RES; i++) {
								position = lookUpMatrix[k][i];
								red = (pxRxedMessage->data[cnt]);
								green = (pxRxedMessage->data[cnt + 1]);
								blue = (pxRxedMessage->data[cnt + 2]);
								colorValue = (red << 16) + (green << 8)
										+ (blue);
								NEO_SetPixelColor(j, position, colorValue);
								cnt = cnt + 3;
							}
						}

					}
					NEO_TransferPixels();

					for (int k = 0; k < 40; k++) {
						for (int z = 1; z < 13; z++) {
							setRingData(z, 0xff00ff);
							NEO_TransferPixels();
							vTaskDelay(pdMS_TO_TICKS(50));
							NEO_ClearAllPixel();
							NEO_TransferPixels();
						}
					}
*/					NEOA_Lauflicht();

					break;
				case SINGLE:
					/*Display MODE 1*/
					CLS1_SendStr((unsigned char*) "Playing single Polle  ",
							CLS1_GetStdio()->stdOut);
					CLS1_SendStr((unsigned char*) "\r\n ",
							CLS1_GetStdio()->stdOut);
					value = 1;
					char ** names;
					names = getNamelist();

					SetTrail(0x200020, 16, 4, 60, 50);
					break;
				case LOGO:
					/*Display Mode 2*/
					CLS1_SendStr((unsigned char*) "Display Logo ",
							CLS1_GetStdio()->stdOut);
					CLS1_SendStr((unsigned char*) "\r\n ",
							CLS1_GetStdio()->stdOut);
					setRingData(2, 0xff0000);
					setRingData(3, 0xff0000);
					setRingData(4, 0xff0000);
					setRingData(5, 0xff0000);
					setRingData(6, 0xff0000);
					NEO_TransferPixels();
					DimmPercentRing(3, 20);
					DimmPercentRing(4, 80);
					DimmPercentRing(5, 95);
					DimmPercentRing(6, 99);
					NEO_TransferPixels();

					break;
				case LAUFLICHT:
					/*Display Mode 3*/
					CLS1_SendStr((unsigned char*) "Playing fancy Lauflicht  ",
							CLS1_GetStdio()->stdOut);
					CLS1_SendStr((unsigned char*) "\r\n ",
							CLS1_GetStdio()->stdOut);
					setRingData(2, 0xff0000);
					setRingData(3, 0xff0000);
					setRingData(4, 0xff0000);
					setRingData(5, 0xff0000);
					setRingData(6, 0xff0000);
					NEO_TransferPixels();
					vTaskDelay(pdMS_TO_TICKS(1000));
					DimmPercentRing(4, 80);

					NEO_TransferPixels();
					value = 3;
					break;

				case CALIBRATION:
					/*Calibration Mode*/
					CLS1_SendStr((unsigned char*) "Start calibration...  ",
							CLS1_GetStdio()->stdOut);
					CLS1_SendStr((unsigned char*) "\r\n ",
							CLS1_GetStdio()->stdOut);
					SetCoordinate(1, 1, 0xff0000);
					NEO_TransferPixels();
					vTaskDelay(pdMS_TO_TICKS(1000));
					SetCoordinate(9, 1, 0xff0000);
					NEO_TransferPixels();
					vTaskDelay(pdMS_TO_TICKS(1000));
					SetCoordinate(17, 1, 0xff0000);
					NEO_TransferPixels();
					vTaskDelay(pdMS_TO_TICKS(1000));
					SetCoordinate(1, 9, 0xff0000);
					NEO_TransferPixels();
					vTaskDelay(pdMS_TO_TICKS(1000));
					SetCoordinate(9, 9, 0xff0000);
					NEO_TransferPixels();
					vTaskDelay(pdMS_TO_TICKS(1000));
					SetCoordinate(17, 9, 0xff0000);
					NEO_TransferPixels();
					vTaskDelay(pdMS_TO_TICKS(1000));
					SetCoordinate(1, 17, 0xff0000);
					NEO_TransferPixels();
					vTaskDelay(pdMS_TO_TICKS(1000));
					SetCoordinate(9, 17, 0xff0000);
					NEO_TransferPixels();
					vTaskDelay(pdMS_TO_TICKS(1000));
					SetCoordinate(17, 17, 0xff0000);
					NEO_TransferPixels();
					CLS1_SendStr((unsigned char*) "...calibration done",
							CLS1_GetStdio()->stdOut);
					CLS1_SendStr((unsigned char*) "\r\n ",
							CLS1_GetStdio()->stdOut);
					break;
				}

			}
			if (FRTOS1_xSemaphoreGive(mutex) != pdTRUE) {
				/*Couldnt return Mutex*/
				for (;;) { /*shouldnt go here*/
				}
			}
		}

	}

#if PL_CONFIG_HAS_MMA8451
	int16_t xmg, ymg, zmg;
#endif

	NEO_SetAllPixelColor(0xff0000);
// do something on the LED Matrix
	for (;;) {
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

uint8_t SetTrail(uint32_t color, uint32_t end, uint32_t nofTail,
		uint8_t dimmPercent, uint16_t delayMs) {
	NEO_PixelIdxT pixel;
	unsigned int i;

	for (pixel = 1; pixel <= end + nofTail + 1; pixel++) {
		/* move head */
		if (pixel <= end) {
			SetCoordinate(pixel, 12, color);
			SetCoordinate(pixel, 11, color);
		}
		/* clear tail pixel */
		if ((pixel > (nofTail + 1)) && pixel - (nofTail + 1) <= end) {
			ClearCoordinate(pixel - (nofTail + 1), 12);
			ClearCoordinate(pixel - (nofTail + 1), 11);
		}
		/* dim remaining tail pixel */
		for (i = 0; i < nofTail; i++) {
			if (pixel > i && pixel - (i + 1) <= end) {
				DimmPercentPixel(pixel - (i + 1), 12, dimmPercent);
				DimmPercentPixel(pixel - (i + 1), 11, dimmPercent);
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
	tskIDLE_PRIORITY + 1, /* initial priority */
	(xTaskHandle*) NULL /* optional task handle to create */
	) != pdPASS) {
		/*lint -e527 */
		for (;;) {
		}; /* error! probably out of memory */
		/*lint +e527 */
	}
}

#endif /* PL_CONFIG_HAS_NEO_PIXEL */

