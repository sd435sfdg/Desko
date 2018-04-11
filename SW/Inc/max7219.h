#ifndef _MAX7219_H
#define _MAX7219_H

#include "stm32f1xx_hal.h"

#define CS_port GPIOA
#define CS_pin GPIO_PIN_6
typedef enum {
	OP_NO_OP 			  = 0x00 << 8,
	OP_ROW_1 	 	  = 0x01 << 8,
	OP_ROW_2 		  = 0x02 << 8,
	OP_ROW_3 		  = 0x03 << 8,
	OP_ROW_4 	  	= 0x04 << 8,
	OP_ROW_5 		  = 0x05 << 8,
	OP_ROW_6		  = 0x06 << 8,
	OP_ROW_7 		  = 0x07 << 8,
	OP_ROW_8 		  = 0x08 << 8,
	OP_DECODE_MODE  = 0x09 << 8,
	OP_INTENSITY 	  = 0x0A << 8,
	OP_SCANLIMIT 	  = 0x0B << 8,
	OP_SHUTDOWN 	  = 0x0C << 8,
	OP_DISPLAY_TEST = 0x0F << 8
} MAX7219_OPCODE;



void sendPacket(SPI_HandleTypeDef *hspi, uint8_t *pData);
//void powerOn();
//void powerOff();
//void setIntensity(uint8_t value);
//void scanLimit(uint8_t limit);

#endif //_MAX7219_H
