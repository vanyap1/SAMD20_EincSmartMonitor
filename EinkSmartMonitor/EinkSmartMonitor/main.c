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
	#define TX_MODE			TX_UNMUTE
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
	.date = 29,
	.month = 4,
	.year = 24,
	.dayofweek = 6,
	.hour = 1,
	.minute = 02,
	.second = 00
};

requestData syncRequest;




weather_day weather = {
	.currentTemp = 234,
	.currentHum = 45,
	.currentPress = 755,
	
	.dayTempPart[0] = 200,
	.dayPressPart[0] = 700,
	.dayWetIcon[0] = 67,
	
	.dayTempPart[1] = -210,
	.dayPressPart[1] = 701,
	.dayWetIcon[1] = 65,
	
	.dayTempPart[2] = 220,
	.dayPressPart[2] = 702,
	.dayWetIcon[2] = 64,
	
	.dayTempPart[3] = 230,
	.dayPressPart[3] = 703,
	.dayWetIcon[3] = 69
};


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
			
			
			if((sys_rtc.second % 10) == 0){
				
				
				//syncRequest.myBatVoltage = getBatVoltage();
				syncRequest.myId = TAGID;			//My tag ID
				syncRequest.responseRequest = 1;	//Sync asc
				syncRequest.myHumidity = 0;			//Sensor is not connected
				syncRequest.myTemperature = 0;		//Sensor is not connected
				
				
				rfm69_init(868, MYID, NETWORKID);
				setHighPower(true);
				
				sprintf(einkSynkReq, "N%02d", sys_rtc.second);
				rfTxDataPack.destinationAddr = GATEWAY;
				rfTxDataPack.senderAddr = MYID;
				rfTxDataPack.opcode = MSG;
				rfTxDataPack.rxtxBuffLenght = sizeof(einkSynkReq);
				rfTxDataPack.dataCRC = simpleCRC(&einkSynkReq, sizeof(einkSynkReq));
				sendFrame(&rfTxDataPack, &einkSynkReq);
				delay_ms(3);
				gpio_set_pin_level(RF_RST, true);
			}
			
			if(sys_rtc.minute == 0 & sys_rtc.second == 0){
				einkUpdate = 1;
			}
			
			if(einkUpdate){
				lastRssi = (rfTxDataPack.rssi == 0)? -99 : rfTxDataPack.rssi;
				gpio_set_pin_level(EP_RST, true);
				gpio_set_pin_level(EP_EN, true);
				u8g2_InitDisplay(&einkPanel);
				u8g2_ClearBuffer(&einkPanel);
				
				u8g2_DrawLine(&einkPanel, 70, 1, 70 , 90);
				u8g2_DrawLine(&einkPanel, 140, 1, 140 , 90);
				u8g2_DrawLine(&einkPanel, 210, 1, 210 , 90);
				u8g2_DrawLine(&einkPanel, 280, 1, 280 , 90);
				u8g2_DrawLine(&einkPanel, 1, 90, 320 , 90);
				
				
				u8g2_SetFont(&einkPanel, u8g2_font_open_iconic_weather_6x_t);
				u8g2_DrawGlyph(&einkPanel, 10, 51, weather.dayWetIcon[0]);
				u8g2_DrawGlyph(&einkPanel, 81, 51, weather.dayWetIcon[1]);
				u8g2_DrawGlyph(&einkPanel, 151, 51, weather.dayWetIcon[2]);
				u8g2_DrawGlyph(&einkPanel, 221, 51, weather.dayWetIcon[3]);
				
				uint8_t infoStr[32];
				uint8_t line1[8], line2[8];
				
				u8g2_SetFont(&einkPanel, u8g2_font_ncenB14_tr);
				
				sprintf(line1, "%02dC", weather.dayTempPart[0]/10);
				sprintf(line2, "%03dP", weather.dayPressPart[0]);
				u8g2_DrawStr(&einkPanel, 10, 70, (void *)line1);
				u8g2_DrawStr(&einkPanel, 10, 86, (void *)line2);
				
				sprintf(line1, "%02dC", weather.dayTempPart[1]/10);
				sprintf(line2, "%03dP", weather.dayPressPart[1]);
				u8g2_DrawStr(&einkPanel, 10+70, 70, (void *)line1);
				u8g2_DrawStr(&einkPanel, 10+70, 86, (void *)line2);
				
				sprintf(line1, "%02dC", weather.dayTempPart[2]/10);
				sprintf(line2, "%03dP", weather.dayPressPart[2]);
				u8g2_DrawStr(&einkPanel, 10+140, 70, (void *)line1);
				u8g2_DrawStr(&einkPanel, 10+140, 86, (void *)line2);

				sprintf(line1, "%02dC", weather.dayTempPart[3]/10);
				sprintf(line2, "%03dP", weather.dayPressPart[3]);
				u8g2_DrawStr(&einkPanel, 10+210, 70, (void *)line1);
				u8g2_DrawStr(&einkPanel, 10+210, 86, (void *)line2);
				
				sprintf(infoStr, "%s %02d-%02d-20%02d", daysFull[sys_rtc.dayofweek], sys_rtc.date, sys_rtc.month, sys_rtc.year);
				u8g2_DrawStr(&einkPanel, 15, 110, (void *)infoStr);
								
				float temp =  weather.currentTemp/10;
				sprintf(infoStr, "Now: %3.1fC; %03dmm; %02d%%", temp, weather.currentPress, weather.currentHum);
				u8g2_DrawStr(&einkPanel, 15, 132, (void *)infoStr); 
				
				u8g2_SetFont(&einkPanel, u8g2_font_5x8_tf);
				
				sprintf(line2, "ID %04X", TAGID);
				u8g2_DrawStr(&einkPanel, 260, 134, (void *)line2);
				
				sprintf(line1, "%ddB", lastRssi);
				u8g2_DrawStr(&einkPanel, 260, 142, (void *)line1);
				
				if(syncRequest.myBatVoltage){
					sprintf(line2, "%dmV", syncRequest.myBatVoltage);
					u8g2_DrawStr(&einkPanel, 260, 150, (void *)line2);
				}else{
					u8g2_DrawStr(&einkPanel, 260, 150, (void *)"BAT ERR");
				}
				
				//u8g2_DrawRFrame(&lcd, 50 , 120, 20, 20, 3);
				//u8g2_DrawCircle(&lcd, 200, 120, 25, U8G2_DRAW_ALL);
				
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
