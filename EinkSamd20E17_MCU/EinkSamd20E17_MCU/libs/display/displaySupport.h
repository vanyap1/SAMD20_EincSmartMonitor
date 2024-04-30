
/*
 * displaySupport.h
 *
 * Created: 27.03.2024 22:16:31
 *  Author: Vanya
 */ 
#include "u8g2.h"
#include "atmel_start_pins.h"
#include "hal_delay.h"

//#include <peripheral_clk_config.h>
//#include <utils.h>
//#include <hal_init.h>
//#include <hpl_gclk_base.h>
//#include <hpl_pm_base.h>



#ifndef DISPLAY_SUPPORT_FNS
#define DISPLAY_SUPPORT_FNS


uint8_t fake_delay_fn(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

void epdResetScreen(void);
void epdReset(bool arg);
void epdSelect(bool arg);
void epdCommand(bool arg);
void epdWriteByte(uint8_t arg);
void epdWriteBuff(uint8_t* buff, uint16_t len);


uint8_t screenSpi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);



#endif