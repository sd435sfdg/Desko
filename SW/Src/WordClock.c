#include "WordClock.h"
#include "stdbool.h"
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
    			uint8_t temp = 0x00;
			for(int loop = 0; loop<7; loop++){
				temp |= ((data >> (6-loop) & 0x0001) << loop);
			}
			temp &= 0x7F;
			temp |= (data & 0x0080);
		  	uint16_t packet[NUM_MAX7219] = {0x0000, row << 8 | (data & 0x0400)>>6 |
			(data & 0x0200)>>4 | (data & 0x0100)>>2, 0x0000, row << 8 | temp};
			send_packet(hspi, (uint8_t *)packet);
		}	
	  	default:{
      			uint16_t packet[NUM_MAX7219] = {(row - 6) << 8 | (data & 0x0400)>>6 |
			(data & 0x0200)>>4 | (data & 0x0100)>>2, 0x0000, 0x0000, 0x0000};
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
	uint16_t data[4] = {0x0000, 0x0000, REG_SHUTDOWN|0x00, 0x0000};
	send_packet(hspi, (uint8_t *)data);
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

void display_word(SPI_HandleTypeDef *hspi, uint8_t hour, uint8_t minute)
{
	static bool is_first_run = true;
	static uint8_t update_minute, update_hour;
	if(is_first_run == true){
		update_minute = minute/5;
		update_hour = hour < 12 ? hour : hour-12;
		set_row(hspi, 1, IT_IS);
	}
	else if(minute/5 == update_minute) return;
	update_minute = minute/5;
	if(update_minute > 6) set_row(hspi, 4, TO);
	else if(update_minute > 0)  set_row(hspi, 5, PAST);
	switch(update_minute){
		case 0:{
			 if(hour == 10 || hour == 22) set_row(hspi, 10, TEN|OCLOCK);
     			 else set_row(hspi, 10, OCLOCK);		
			 set_row(hspi, 3, 0);
			 set_row(hspi, 4, 0);
		  }
		case 1:{
			 set_row(hspi, 3, M_FIVE);
			 if(hour == 7 || hour == 19) set_row(hspi, 5, PAST|SEVEN);
       			 else set_row(hspi, 5, PAST);	
      			 if(hour == 10 || hour == 22) set_row(hspi, 10, TEN);
       			 else set_row(hspi, 10, 0);				
			 break;
		  }
		case 2: {
			 set_row(hspi, 4, M_TEN);
			 set_row(hspi, 3, 0);
			 break;
		  }
		case 3: {
			 set_row(hspi, 2, QUARTER);
			 set_row(hspi, 4, 0);
			 break;
		  }
		case 4: {
			 set_row(hspi, 3, TWENTY);
			 set_row(hspi, 2, 0);
			 break;
		  }
		case 5: {
			 set_row(hspi, 3, TWENTY_FIVE);
			 break;
		  }
		case 6:{			 
			 set_row(hspi, 4, HALF);
			 set_row(hspi, 3, 0);
			 break;
		  }
		case 7:{			 
			 set_row(hspi, 3, TWENTY_FIVE);
			 set_row(hspi, 4, TO);
			 hour++;
			 if(hour == 7 || hour == 19) set_row(hspi, 5, SEVEN);
			 else set_row(hspi, 5, 0);
			 break;
		  }
			case 8: {
			 set_row(hspi, 3, TWENTY);
			 break;
		  }
			case 9: {
			 set_row(hspi, 2, QUARTER);
			 set_row(hspi, 3, 0);
			 break;
		  }
			case 10: {
			 set_row(hspi, 4, TEN|TO);
			 set_row(hspi, 2, 0);
			 break;
		  }
			case 11: {
			 set_row(hspi, 3, FIVE);
			 set_row(hspi, 4, TO);
			 break;
		  }
	  }
	if(is_first_run == false){
	 if(hour == update_hour || update_hour == hour - 12) return;	
	}
	update_hour = hour < 12 ? hour : hour-12;
	update_hour = update_hour < 12 ? update_hour : update_hour-12;
	switch(update_hour)
	 {
		 case 0: {
			 set_row(hspi, 8, TWELVE);
			 set_row(hspi, 9, 0);
			 break;
		 }
		 case 1: {
			 set_row(hspi, 6, ONE);
			 set_row(hspi, 8, 0);
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
			 set_row(hspi, 6, 0);
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
			 set_row(hspi, 5, SEVEN);
			 set_row(hspi, 7, 0);
			 break;
		 }
		 case 8: {
			 set_row(hspi, 9, EIGHT);
			 set_row(hspi, 5, 0);
			 break;
		 }
		 case 9: {
			 set_row(hspi, 8, NINE);
			 set_row(hspi, 9, 0);
			 break;
		 }
		 case 10: {
			 set_row(hspi, 10, TEN);
			 set_row(hspi, 8, 0);
			 break;
		 }
		 case 11: {
			 set_row(hspi, 9, ELEVEN);
			 set_row(hspi, 10, 0);
			 break;
		 }
	 }	
 is_first_run = false;	
 return;	 
}

void display_number(SPI_HandleTypeDef *hspi, uint8_t hour, uint8_t minute)
{
	
}
