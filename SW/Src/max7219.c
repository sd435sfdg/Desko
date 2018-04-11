#include "max7219.h"

void sendPacket(SPI_HandleTypeDef *hspi, uint8_t *pData){
	HAL_GPIO_WritePin(CS_port, CS_pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, pData, 1, 100);
	HAL_GPIO_WritePin(CS_port, CS_pin, GPIO_PIN_SET);
}