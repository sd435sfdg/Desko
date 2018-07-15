#ifndef _WORDCLOCK_H
#define _WORDCLOCK_H

#include "stm32f1xx_hal.h"
#include "stdbool.h"

#define CS_PORT GPIOA
#define CS_PIN GPIO_PIN_6
#define BZ_PORT GPIOA
#define BZ_PIN GPIO_PIN_8
#define INTENSITY 0x05

enum {
	NUM_ROWS = 11,
	NUM_COLUMNS = 11,
	NUM_MAX7219 = 4
};

void send_packet(SPI_HandleTypeDef *hspi, uint8_t *p_data);
void send_all(SPI_HandleTypeDef *hspi, uint16_t data);
void set_row(SPI_HandleTypeDef *hspi, uint8_t row, uint16_t data);
void led_init(SPI_HandleTypeDef *hspi);
void clean_all_rows(SPI_HandleTypeDef *hspi);
void clean_all_bits(SPI_HandleTypeDef *hspi);
void display_word(SPI_HandleTypeDef *hspi, uint8_t hour, uint8_t minute, uint8_t second);
void display_number(SPI_HandleTypeDef *hspi, uint8_t hour, uint8_t minute);
void edit_position(SPI_HandleTypeDef *hspi, uint8_t position);
bool getAlarmStatus(void);
#endif //_WORDCLOCK_H
