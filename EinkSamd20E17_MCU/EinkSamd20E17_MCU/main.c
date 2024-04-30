#include <atmel_start.h>
#include <string.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include "hw_driver.h"
#include "rtc.h"
#include "RFM69registers.h"
#include "RFM69.h"
#include "u8g2.h"
#include "displaySupport.h"

#define POWERBANKID		0x21
#define VFDSCREEN		0x03
#define EINKMONITOR		0x04
#define DEVMODULE		0x22
#define DEVMODULE2		0xFE
#define DEVUSBHID		0xFD
#define ALLNODES		0xfe
#define GATEWAY			0xfc
#define SMARTSCREEN		0xf0

// OPcodes
	#define RTC_SYNC		0x81
	#define MSG				0x82
	#define POWERBANK		0x83
	#define RTC_SYNC_REQ	0x84
	#define WET_SYNC_REQ	0x85

//Working options
	#define NETWORKID		33
	#define TX_MUTE			0
	#define TX_UNMUTE		1
	#define TX_MODE			TX_MUTE
	#define MYID			EINKMONITOR
	#define TAGID			0x031E					//must be 2 bytes	

u8g2_t einkPanel;
uint8_t txLen;
uint8_t txCRC;
rfHeader rfTxDataPack;
uint8_t einkSynkReq[16];
uint8_t einkUpdate = 1;
int8_t lastRssi = -120;
uint16_t einkUpdSafeTimer = 0;

rtc_date sys_rtc = {
	.date = 30,
	.month = 4,
	.year = 24,
	.dayofweek = 6,
	.hour = 23,
	.minute = 25,
	.second = 00
};

requestData syncRequest;







int main(void)
{
	/* Initializes MCU, drivers and middleware */
	mcu_init();
	//rtc_set(&sys_rtc);
	rtc_int_enable(&sys_rtc);
	u8g2_Setup_GDEY0154_152x304_f(&einkPanel, U8G2_R0, screenSpi, screenSpi);

	//SPI0 - SERCOM0
	//I2C0 - SERCOM3
	while (1) {
		if(RTC_IRQ_Ready()){
			rtc_sync(&sys_rtc);
			gpio_set_pin_level(GLD, true);
			delay_ms(1);
			gpio_set_pin_level(GLD, false);

			if(sys_rtc.minute == 0 & sys_rtc.second == 0){
				einkUpdate = 1;
			}
			
			if(einkUpdate){
				lastRssi = (rfTxDataPack.rssi == 0)? -99 : rfTxDataPack.rssi;
				gpio_set_pin_level(EP_RST, true);
				gpio_set_pin_level(EP_EN, true);
				u8g2_InitDisplay(&einkPanel);
				u8g2_ClearBuffer(&einkPanel);
				
				
				uint8_t infoStr[32];
				uint8_t line1[8], line2[8];
				
				u8g2_SetFont(&einkPanel, u8g2_font_ncenB14_tr);
				
				
				sprintf(infoStr, "%02d-%02d-20%02d", sys_rtc.date, sys_rtc.month, sys_rtc.year);
				u8g2_DrawStr(&einkPanel, 30, 25, (void *)infoStr);
				
				u8g2_DrawStr(&einkPanel, 5, 50, (void *)"J3QQ4-H7H2V");
				u8g2_DrawStr(&einkPanel, 5, 75, (void *)"2HCH4-M3HK8");
				u8g2_DrawStr(&einkPanel, 5, 100, (void *)"6M8VW");
								
				
				u8g2_SetFont(&einkPanel, u8g2_font_5x8_tf);
				sprintf(line2, "ID %04X", TAGID);
				u8g2_DrawStr(&einkPanel, 115, 134, (void *)line2);
				
				sprintf(line1, "%ddB", lastRssi);
				u8g2_DrawStr(&einkPanel, 115, 142, (void *)line1);
				
				if(syncRequest.myBatVoltage){
					sprintf(line2, "%dmV", syncRequest.myBatVoltage);
					u8g2_DrawStr(&einkPanel, 115, 150, (void *)line2);
				}else{
					u8g2_DrawStr(&einkPanel, 115, 150, (void *)"BAT 84%");
				}
				
				//u8g2_DrawRFrame(&lcd, 50 , 120, 20, 20, 3);
				//u8g2_DrawCircle(&lcd, 200, 120, 25, U8G2_DRAW_ALL);
				
				
				u8g2_SetFont(&einkPanel, u8g2_font_streamline_all_t);
				u8g2_DrawGlyph(&einkPanel, 5, 150, 508);
				u8g2_DrawGlyph(&einkPanel, 30, 150, 509);
				u8g2_DrawGlyph(&einkPanel, 55, 150, 527);
				u8g2_SendBuffer(&einkPanel);
				delay_ms(50);
				
				while(!gpio_get_pin_level(EP_BUSY)){
					gpio_toggle_pin_level(GLD);
					einkUpdSafeTimer++;
					delay_ms(50);
					if(einkUpdSafeTimer >= 300)break;
				}
				einkUpdSafeTimer=0;
				gpio_set_pin_level(EP_RST, false);
				gpio_set_pin_level(EP_EN, false);
				gpio_set_pin_level(GLD, false);
				einkUpdate = 0;	
			}
						
			SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;// | SCB_SCR_SLEEPONEXIT_Msk;  
			NVMCTRL->CTRLB.reg |= NVMCTRL_CTRLB_SLEEPPRM_DISABLED;        			
			__DSB();
			__WFI();
		}
	}
}
