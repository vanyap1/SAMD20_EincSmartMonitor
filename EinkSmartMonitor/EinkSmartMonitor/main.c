#include <atmel_start.h>
#include <string.h>
#include "hw_driver.h"
#include "rtc.h"
#include "RFM69registers.h"
#include "RFM69.h"
#include "u8g2.h"
#include "displaySupport.h"

u8g2_t lcd;


#define POWERBANKID		0x21
#define VFDSCREEN		0x03
#define DEVMODULE		0x22
#define DEVMODULE2		0xFE
#define DEVUSBHID		0xFD


#define NETWORKID		33
#define NODEID			DEVUSBHID
#define ALLNODES		0xfe
#define SMARTSCREEN		0xf0
#define RX_MODE			1
#define RTC_SYNC		0x81
#define MSG				0x82
#define POWERBANK		0x83

#define TX_MUTE			0
#define TX_UNMUTE		1
#define TX_MODE			TX_MUTE





rtc_date sys_rtc = {
	.date = 4,
	.month = 12,
	.year = 2023,
	.dayofweek = 1,
	.hour = 23,
	.minute = 54,
	.second = 00
};


int main(void)
{
	/* Initializes MCU, drivers and middleware */
	//atmel_start_init();
	mcu_init();
	
	rtc_set(&sys_rtc);
	rtc_int_enable(&sys_rtc);

	u8g2_Setup_GDEY0154_152x304_f(&lcd, U8G2_R0, screenSpi, screenSpi);
	
	
	u8g2_InitDisplay(&lcd);
	u8g2_ClearBuffer(&lcd);
	
	
	
	
	
	
	
	
	u8g2_DrawLine(&lcd, 70, 1, 70 , 90);
	u8g2_DrawLine(&lcd, 140, 1, 140 , 90);
	u8g2_DrawLine(&lcd, 210, 1, 210 , 90);
	u8g2_DrawLine(&lcd, 280, 1, 280 , 90);
	u8g2_DrawLine(&lcd, 1, 90, 320 , 90);
	
	
	u8g2_SetFont(&lcd, u8g2_font_open_iconic_weather_6x_t);
	u8g2_DrawGlyph(&lcd, 10, 51, 69);
	u8g2_DrawGlyph(&lcd, 81, 51, 64);
	u8g2_DrawGlyph(&lcd, 151, 51, 65);
	u8g2_DrawGlyph(&lcd, 221, 51, 67);
	
	
	
	
	u8g2_SetFont(&lcd, u8g2_font_ncenB14_tr);
	
	u8g2_DrawStr(&lcd, 10, 70, (void *)"+13°C");
	u8g2_DrawStr(&lcd, 10, 86, (void *)"734P");
	
	u8g2_DrawStr(&lcd, 10+70, 70, (void *)"+16°C");
	u8g2_DrawStr(&lcd, 10+70, 86, (void *)"736P");
	
	u8g2_DrawStr(&lcd, 10+140, 70, (void *)"+14°C");
	u8g2_DrawStr(&lcd, 10+140, 86, (void *)"737P");

	u8g2_DrawStr(&lcd, 10+210, 70, (void *)"+10°C");
	u8g2_DrawStr(&lcd, 10+210, 86, (void *)"737P");
	
	u8g2_DrawStr(&lcd, 15, 110, (void *)"Thursday 25-04-2024");
	u8g2_DrawStr(&lcd, 15, 132, (void *)"Now 13.6C , 735mm, 63%");
	
	u8g2_SetFont(&lcd, u8g2_font_5x8_tf);
	u8g2_DrawStr(&lcd, 260, 142, (void *)"RF-79dB");
	u8g2_DrawStr(&lcd, 260, 150, (void *)"B 3.94V");

	//u8g2_DrawRFrame(&lcd, 50 , 120, 20, 20, 3);
	//u8g2_DrawCircle(&lcd, 200, 120, 25, U8G2_DRAW_ALL);
	
	u8g2_SendBuffer(&lcd);
	delay_ms(50);
	
	
	while(!gpio_get_pin_level(EP_BUSY)){
		gpio_toggle_pin_level(GLD);
		delay_ms(50);
	}
	gpio_set_pin_level(EP_RST, false);
	gpio_set_pin_level(EP_EN, false);
	gpio_set_pin_level(GLD, false);
	
	
	//SPI0 - SERCOM0
	//I2C0 - SERCOM3
	while (1) {
		if(RTC_IRQ_Ready()){
			rtc_sync(&sys_rtc);
		}
		
		//gpio_toggle_pin_level(GLD);
		delay_ms(50);
		//SPI0_Write_byte(0xA8);
	}
}
