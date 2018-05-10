#ifndef _WORDCLOCK_H
#define _WORDCLOCK_H

#include "stm32f1xx_hal.h"

#define CS_PORT GPIOA
#define CS_PIN GPIO_PIN_6
#define INTENSITY 0x03

enum {
	NUM_ROWS = 11,
	NUM_COLUMNS = 11,
	NUM_MAX7219 = 4
};
typedef enum {
	REG_NO_OP = 0x00<<8,
	REG_SEG_1 = 0x01<<8,
	REG_SEG_2 = 0x02<<8,
	REG_SEG_3 = 0x03<<8,
	REG_SEG_4 = 0x04<<8,
	REG_SEG_5 = 0x05<<8,
	REG_SEG_6 = 0x06<<8,
	REG_SEG_7 = 0x07<<8,
	REG_SEG_8 = 0x08<<8,
	REG_DECODE_MODE = 0x09 << 8,
	REG_INTENSITY = 0x0A << 8,
	REG_SCANLIMIT = 0x0B << 8,
	REG_SHUTDOWN = 0x0C << 8,
	REG_DISPLAY_TEST = 0x0F << 8
}MAX7219_REGISTERS;

typedef enum {
	RUNNING, EDITING
}MODE;

void send_packet(SPI_HandleTypeDef *hspi, uint8_t *p_data);
void send_all(SPI_HandleTypeDef *hspi, uint16_t data);
void set_row(SPI_HandleTypeDef *hspi, uint8_t row, uint16_t data);
void led_init(SPI_HandleTypeDef *hspi);
void clean_all_rows(SPI_HandleTypeDef *hspi);
void clean_all_bits(SPI_HandleTypeDef *hspi);
void display_word(SPI_HandleTypeDef *hspi, uint8_t hour, uint8_t minute);
void display_number(SPI_HandleTypeDef *hspi, uint8_t hour, uint8_t minute);

#endif //_WORDCLOCK_H
