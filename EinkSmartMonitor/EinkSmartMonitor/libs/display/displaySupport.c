
/*
 * displaySupport.c
 *
 * Created: 27.03.2024 22:16:39
 *  Author: Vanya
 */ 
#include "displaySupport.h"


uint8_t fake_delay_fn(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr){
	return 0;
}

void epdResetScreen(void){
	gpio_set_pin_level(EP_RST, false);
	delay_ms(50);
	gpio_set_pin_level(EP_RST, true);
	
}

void epdReset(bool arg){
	gpio_set_pin_level(EP_RST, arg);
}

void epdSelect(bool arg){
	gpio_set_pin_level(EP_CS, arg);
}

void epdCommand(bool arg){
	gpio_set_pin_level(EP_CMD, arg);
}


void epdWriteByte(uint8_t arg){
	SPI0_WriteBuff(arg);
}

void epdWriteBuff(uint8_t* buff, uint16_t len){
	SPI0_WriteBuff(buff, len);
}

uint8_t screenSpi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	
	switch(msg)
	{
		case U8X8_MSG_GPIO_AND_DELAY_INIT:
			//GPIO Init
			break;
		case U8X8_MSG_GPIO_RESET:
			epdReset((arg_int != 1) ?  false : true);
			break;
		case U8X8_MSG_DELAY_MILLI:
			delay_ms(arg_int);
			break;
		
		case U8X8_MSG_BYTE_SEND:
			epdWriteBuff(arg_ptr, arg_int);
			break;
		case U8X8_MSG_BYTE_INIT:
			/* disable chipselect */
			epdSelect(false);
			break;
		case U8X8_MSG_BYTE_SET_DC:
			epdCommand(arg_int);
			break;
		case U8X8_MSG_BYTE_START_TRANSFER:
			epdSelect(false);
			break;
		case U8X8_MSG_BYTE_END_TRANSFER:
			epdSelect(true);
			epdCommand(true);
			break;
		default:
			return 0;
	}
	return 1;
}
