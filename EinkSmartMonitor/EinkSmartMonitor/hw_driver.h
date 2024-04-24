
/*
 * hw_driver.h
 *
 * Created: 23.04.2024 21:57:13
 *  Author: Vanya
 */ 
#ifndef HW_DRIVER_INCLUDED
#define HW_DRIVER_INCLUDED

#include "atmel_start_pins.h"

#ifdef __cplusplus
extern "C" {
#endif
	
#include <hal_atomic.h>
#include <hal_delay.h>
#include <hal_gpio.h>
#include <hal_init.h>
#include <hal_io.h>
#include <hal_sleep.h>
#include <hal_adc_sync.h>
#include <hal_ext_irq.h>
#include <hal_spi_m_sync.h>
#include <hal_i2c_m_sync.h>
#include <hal_delay.h>

extern struct adc_sync_descriptor   ADC_0;
extern struct spi_m_sync_descriptor SPI_0;
extern struct i2c_m_sync_desc I2C_0;

static void I2C_RTC_Handler(void);
uint8_t RTC_IRQ_Ready(void);
static void RF_int_Handler(void);
bool rf_isReady();


void EP_Select(void);
void EP_Unselect(void);

void RF_Select(void);
void RF_Unselect(void);

void SPI0_ReadBuff(uint8_t* buff, uint16_t len);
void SPI0_WriteBuff(uint8_t* buff, uint16_t len);
uint8_t SPI0_Read_byte(void);
void SPI0_Write_byte(uint8_t byte);

bool RTC_write_batch(uint8_t addres, uint8_t *data, uint8_t data_len);
bool RTC_read_batch(uint8_t addres ,uint8_t *data, uint8_t data_len);
bool I2C_write_batch(uint8_t addres, uint8_t *data, uint8_t data_len);
bool I2C_read_batch(uint8_t addres ,uint8_t *data, uint8_t data_len);
bool I2C_read_batch_addr(uint8_t addres, uint8_t reg, uint8_t *data, uint8_t data_len);

void GPIO_init(void);
void SPI_init(void);
void I2C_init(void);
void IRQ_init(void);



void mcu_init(void);


	
#ifdef __cplusplus
}
#endif
#endif // HW_DRIVER_INCLUDED	