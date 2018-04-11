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

typedef enum {
	IT_IS       = 0x06C0,
	AM          = 0x000C,
	PM          = 0x0003,
	M_FIVE      = 0x000F,
	M_TEN       = 0x0038,
	QUARTER     = 0x03F8,
	TWENTY      = 0x07E0,
	TWENTY_FIVE = 0x07EF,
	HALF        = 0x0780,
	PAST        = 0x0780,
	TO          = 0x0003,
	ONE         = 0x0700,
	TWO         = 0x00E0,
	THREE       = 0x001F,
	FOUR        = 0x0780,
	FIVE        = 0x0078,
	SIX         = 0x0007,
	SEVEN       = 0x003E,
	EIGHT       = 0x07C0,
	NINE        = 0x0780,
	TEN         = 0x0700,
	ELEVEN      = 0x003F,
	TWELVE      = 0x003F,
	OCLOCK      = 0x003F
}WORD_TABLE;

void send_packet(SPI_HandleTypeDef *hspi, uint8_t *p_data);
void send_all(SPI_HandleTypeDef *hspi, uint16_t data);
void set_row(SPI_HandleTypeDef *hspi, uint8_t row, uint16_t data);
void led_init(SPI_HandleTypeDef *hspi);
void clean_all_rows(SPI_HandleTypeDef *hspi);
void clean_all_bits(SPI_HandleTypeDef *hspi);
void led_run(SPI_HandleTypeDef *hspi, uint8_t hour, uint8_t minute);

#endif //_WORDCLOCK_H
