#include "WordClock.h"

enum {
	REG_NO_OP = 0x00<<8,
	REG_DIGIT_1 = 0x01<<8,
	REG_DIGIT_2 = 0x02<<8,
	REG_DIGIT_3 = 0x03<<8,
	REG_DIGIT_4 = 0x04<<8,
	REG_DIGIT_5 = 0x05<<8,
	REG_DIGIT_6 = 0x06<<8,
	REG_DIGIT_7 = 0x07<<8,
	REG_DIGIT_8 = 0x08<<8,
	REG_DECODE_MODE = 0x09 << 8,
	REG_INTENSITY = 0x0A << 8,
	REG_SCANLIMIT = 0x0B << 8,
	REG_SHUTDOWN = 0x0C << 8,
	REG_DISPLAY_TEST = 0x0F << 8
};

enum {
	IT_IS       = 0x06C0,
	AM          = 0x0018,
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

const uint8_t sec[10] = {0x00, 0x10, 0x18, 0x1C, 0x1E, 0x1F, 0x0F, 0x07, 0x03, 0x01};
const uint8_t number[10][5] = {
	0x0F, 0x09, 0x09, 0x09, 0x0F,
	0x01, 0x03, 0x01, 0x01, 0x01,
	0x0F, 0x01, 0x0F, 0x08, 0x0F,
	0x0F, 0x01, 0x0F, 0x01, 0x0F,
	0x09, 0x09, 0x0F, 0x01, 0x01,
	0x0F, 0x08, 0x0F, 0x01, 0x0F,
	0x0F, 0x08, 0x0F, 0x09, 0x0F,
	0x0F, 0x01, 0x01, 0x01, 0x01,
	0x0F, 0x09, 0x0F, 0x09, 0x0F,
	0x0F, 0x09, 0x0F, 0x01, 0x0F};

void send_packet(SPI_HandleTypeDef *hspi, uint8_t *p_data)
{
	HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
	for(int loop = 0;loop < NUM_MAX7219; loop++){
  	HAL_SPI_Transmit(hspi, p_data + loop*sizeof(uint16_t), 1, 100);
	}
	HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
}	

void send_all(SPI_HandleTypeDef *hspi, uint16_t data)
{
	uint16_t *p_data = &data;
	HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
	for(int loop = 0;loop < NUM_MAX7219; loop++){
  	HAL_SPI_Transmit(hspi, (uint8_t *)p_data, 1, 100);
	}
	HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
}

void set_row(SPI_HandleTypeDef *hspi, uint8_t row, uint16_t data)
{
	switch(row){
		case 1: case 2: case 3: case 4: case 5: case 6: case 7:	case 8:{
		  uint16_t packet[NUM_MAX7219] = {0x0000, 0x0000, row << 8 | (data & 0x0007), row << 8 | (data & 0x07F8)>>3};
			send_packet(hspi, (uint8_t *)packet);
			break;
		}	
		default:{
			uint16_t packet[NUM_MAX7219] = {(row - 8) << 8 | (data & 0x0007), (row - 8) << 8 | (data & 0x07F8)>>3, 0x0000, 0x0000};
			send_packet(hspi, (uint8_t *)packet);
			break;
		}
	}	
}

void led_init(SPI_HandleTypeDef *hspi)
{
  send_all(hspi, REG_DECODE_MODE|0x00);
	send_all(hspi, REG_INTENSITY|INTENSITY);
	send_all(hspi, REG_SCANLIMIT|0x07);
	send_all(hspi, REG_SHUTDOWN|0x01);
	send_all(hspi, REG_DISPLAY_TEST|0x01);
	clean_all_bits(hspi);
	HAL_Delay(3000);
	send_all(hspi, REG_DISPLAY_TEST|0x00);
}

void clean_all_rows(SPI_HandleTypeDef *hspi)
{
	for(int loop = 1; loop<=NUM_ROWS; loop++){
		set_row(hspi, loop, 0);
	}
}

void clean_all_bits(SPI_HandleTypeDef *hspi)
{
	for(int loop = 1; loop<=8; loop++){
		send_all(hspi, loop << 8 | 0x00);
	}
}

void display_word(SPI_HandleTypeDef *hspi, uint8_t hour, uint8_t minute, uint8_t second)
{
	clean_all_bits(hspi);
	uint8_t m_hour;
	if(hour < 12){
		m_hour = hour;
		set_row(hspi, 1, IT_IS|AM);
	}
	else{
		m_hour = hour - 12;
		set_row(hspi, 1, IT_IS|PM);
	}
	edit_position(hspi, minute%5);
	uint8_t m_minute = minute/5;
	if(m_minute > 6) {
		set_row(hspi, 4, TO);
		m_hour++;
	}
	else if(m_minute > 0) set_row(hspi, 5, PAST);
	switch(m_minute){
		case 0:{
			if(m_hour == 10) set_row(hspi, 10, TEN|OCLOCK);
			else set_row(hspi, 10, OCLOCK);	
			break;				
			}
		case 1: case 11:{
			set_row(hspi, 3, M_FIVE);	
			break;
			}
		case 2: case 10:{
			if(m_minute > 6) set_row(hspi, 4, M_TEN|TO);
			else set_row(hspi, 4, M_TEN);
			break;
			}
		case 3: case 9:{
			 set_row(hspi, 2, QUARTER);
			 break;
			}
		case 4: case 8:{
			 set_row(hspi, 3, TWENTY);
			 break;
			}
		case 5: case 7:{
			 set_row(hspi, 3, TWENTY_FIVE);
			 break;
			}
		case 6:{			 
			 set_row(hspi, 4, HALF);
			 break;
			}
		}
	switch(m_hour)
	 {
		 case 0: {
			 set_row(hspi, 8, TWELVE);
			 break;
		 }
		 case 1: {
			 set_row(hspi, 6, ONE);
			 break;
		 }
		 case 2: {
			 set_row(hspi, 6, TWO);
			 break;
		 }
		 case 3: {
			 set_row(hspi, 6, THREE);
			 break;
		 }
		 case 4: {
			 set_row(hspi, 7, FOUR);
			 break;
		 }
		 case 5: {
			 set_row(hspi, 7, FIVE);
			 break;
		 }
		 case 6: {
			 set_row(hspi, 7, SIX);
			 break;
		 }
		 case 7: {
			 if(m_minute > 0) set_row(hspi, 5, PAST|SEVEN);
			 else set_row(hspi, 5, SEVEN);
			 break;
		 }
		 case 8: {
			 set_row(hspi, 9, EIGHT);
			 break;
		 }
		 case 9: {
			 set_row(hspi, 8, NINE);
			 break;
		 }
		 case 10: {
			 if(m_minute == 0) set_row(hspi, 10, TEN|OCLOCK);
			 set_row(hspi, 10, TEN);
			 break;
		 }
		 case 11: {
			 set_row(hspi, 9, ELEVEN);
			 break;
		 }
	 }
	set_row(hspi, 11, sec[second/10] << 6 |  getAlarmStatus() << 5 |sec[second%10]);	 
	return;	 
}

void display_number(SPI_HandleTypeDef *hspi, uint8_t hour, uint8_t minute)
{
		set_row(hspi,  6, 0);
	
		set_row(hspi,  7, number[minute/10][0] << 6 | number[minute%10][0] << 1);
		set_row(hspi,  8, number[minute/10][1] << 6 | number[minute%10][1] << 1);
		set_row(hspi,  9, number[minute/10][2] << 6 | number[minute%10][2] << 1);
		set_row(hspi, 10, number[minute/10][3] << 6 | number[minute%10][3] << 1);
		set_row(hspi, 11, number[minute/10][4] << 6 | getAlarmStatus() << 5 | number[minute%10][4] << 1);	

		set_row(hspi, 1, number[hour/10][0] << 6 | number[hour%10][0] << 1);
		set_row(hspi, 2, number[hour/10][1] << 6 | number[hour%10][1] << 1);
		set_row(hspi, 3, number[hour/10][2] << 6 | number[hour%10][2] << 1);
		set_row(hspi, 4, number[hour/10][3] << 6 | number[hour%10][3] << 1);
		set_row(hspi, 5, number[hour/10][4] << 6 | number[hour%10][4] << 1);	
}	

void edit_position(SPI_HandleTypeDef *hspi, uint8_t position)
{
	if(position == 3 || position == 4) position ^= 7;
	uint16_t packet[NUM_MAX7219] = {REG_DIGIT_4 | 0x10 >> position, 0x0000, 0x0000, 0x0000};
	send_packet(hspi, (uint8_t *)packet);
}
