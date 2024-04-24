
/*
 * hw_driver.c
 *
 * Created: 23.04.2024 21:57:01
 *  Author: Vanya
 */ 
#include "hw_driver.h"
#include <peripheral_clk_config.h>
#include <utils.h>
#include <hal_init.h>
#include <hpl_gclk_base.h>
#include <hpl_pm_base.h>

#include <hpl_adc_base.h>

struct spi_m_sync_descriptor SPI_0;
struct adc_sync_descriptor ADC_0;
struct i2c_m_sync_desc I2C_0;

struct io_descriptor *spi0;
struct io_descriptor *i2c0;

uint8_t rtcIntCount = 0;
uint8_t rfIntCount = 0;

static void I2C_RTC_Handler(void){
	if (rtcIntCount < 255) {
		rtcIntCount++;
	}
}

uint8_t RTC_IRQ_Ready(void){
	uint8_t res = rtcIntCount;
	rtcIntCount = 0;
	return res;
}


static void RF_int_Handler(void){
	if(rfIntCount < 255){
		rfIntCount++;
	}	
}

bool rf_isReady(){
	if (rfIntCount != 0){
		//ptr_rfHeader = data_ready();
		rfIntCount = 0;
		//return (ptr_rfHeader->dataValid == 0 ) ? false : true;
		return true;
	}
	return false;
}

void EP_Select(void){
	gpio_set_pin_level(EP_CS, false);
}
void EP_Unselect(void){
	gpio_set_pin_level(EP_CS, true);
}
void RF_Select(void){
	gpio_set_pin_level(RF_CS, false);
}
void RF_Unselect(void){
	gpio_set_pin_level(RF_CS, true);
}

void RF_Reset(void){
	gpio_set_pin_level(RF_RST, true);
	delay_ms(20);
	gpio_set_pin_level(RF_RST, false);
	delay_ms(20);
}

void SPI0_ReadBuff(uint8_t* buff, uint16_t len){
	io_read(spi0,buff,len);
}
void SPI0_WriteBuff(uint8_t* buff, uint16_t len){
	io_write(spi0,buff,len);
}
uint8_t SPI0_Read_byte(void){
	uint8_t data_byte = 0;
	io_read(spi0,&data_byte,1);
	return data_byte;
}
void SPI0_Write_byte(uint8_t byte){
	io_write(spi0, &byte, 1);
}


bool RTC_write_batch(uint8_t addres, uint8_t *data, uint8_t data_len){
	i2c_m_sync_set_slaveaddr(&I2C_0, addres, I2C_M_SEVEN);
	//i2c_m_sync_cmd_write(&EXT_I2C, 0x00, data, data_len);	
	return (io_write(i2c0, (uint8_t *)data, data_len) >= 0) ? true : false;
}

bool RTC_read_batch(uint8_t addres ,uint8_t *data, uint8_t data_len){
	i2c_m_sync_set_slaveaddr(&I2C_0, addres, I2C_M_SEVEN);
	i2c_m_sync_cmd_read(&I2C_0, 0x00, data, data_len);
	//return (io_read(I2C_io, (uint8_t *)data, data_len) >= 0) ? true : false;
}

bool I2C_write_batch(uint8_t addres, uint8_t *data, uint8_t data_len){
	i2c_m_sync_set_slaveaddr(&I2C_0, addres, I2C_M_SEVEN);
	//i2c_m_sync_cmd_write(&EXT_I2C, 0x00, data, data_len);
	return (io_write(i2c0, (uint8_t *)data, data_len) >= 0) ? true : false;
}

bool I2C_read_batch(uint8_t addres ,uint8_t *data, uint8_t data_len){
	i2c_m_sync_set_slaveaddr(&I2C_0, addres, I2C_M_SEVEN);
	i2c_m_sync_cmd_read(&I2C_0, 0x00, data, data_len);
	return (io_read(i2c0, (uint8_t *)data, data_len) >= 0) ? true : false;
}
bool I2C_read_batch_addr(uint8_t addres, uint8_t reg, uint8_t *data, uint8_t data_len){
	i2c_m_sync_set_slaveaddr(&I2C_0, addres, I2C_M_SEVEN);
	i2c_m_sync_cmd_read(&I2C_0, reg, data, data_len);
}


void GPIO_init(void){
	gpio_set_pin_level(RF_RST, true);
	gpio_set_pin_direction(RF_RST, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(RF_RST, GPIO_PIN_FUNCTION_OFF);
	
	gpio_set_pin_level(RF_CS, true);
	gpio_set_pin_direction(RF_CS, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(RF_CS, GPIO_PIN_FUNCTION_OFF);


	gpio_set_pin_level(EP_CS, true);
	gpio_set_pin_direction(EP_CS, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(EP_CS, GPIO_PIN_FUNCTION_OFF);

	gpio_set_pin_level(EP_CMD, false);
	gpio_set_pin_direction(EP_CMD, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(EP_CMD, GPIO_PIN_FUNCTION_OFF);

	gpio_set_pin_direction(EP_BUSY, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(EP_BUSY, GPIO_PULL_UP);
	gpio_set_pin_function(EP_BUSY, GPIO_PIN_FUNCTION_OFF);

	gpio_set_pin_level(GLD, false);
	gpio_set_pin_direction(GLD, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(GLD, GPIO_PIN_FUNCTION_OFF);

	gpio_set_pin_level(EP_EN, true);
	gpio_set_pin_direction(EP_EN, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(EP_EN, GPIO_PIN_FUNCTION_OFF);
	
	gpio_set_pin_level(EP_RST, true);
	gpio_set_pin_direction(EP_RST, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(EP_RST, GPIO_PIN_FUNCTION_OFF);	
}


void SPI_init(void){
	
	gpio_set_pin_level(PA08, false);
	gpio_set_pin_direction(PA08, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(PA08, PINMUX_PA08C_SERCOM0_PAD0);

	gpio_set_pin_level(PA09, false);

	gpio_set_pin_direction(PA09, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(PA09, PINMUX_PA09C_SERCOM0_PAD1);

	gpio_set_pin_direction(PA10, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(PA10, GPIO_PULL_OFF);
	gpio_set_pin_function(PA10, PINMUX_PA10C_SERCOM0_PAD2);
	
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM0);
	_gclk_enable_channel(SERCOM0_GCLK_ID_CORE, CONF_GCLK_SERCOM0_CORE_SRC);
	
	spi_m_sync_init(&SPI_0, SERCOM0);
	spi_m_sync_get_io_descriptor(&SPI_0, &spi0);
	spi_m_sync_enable(&SPI_0);
	
}



void I2C_init(void){
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM3);
	_gclk_enable_channel(SERCOM3_GCLK_ID_CORE, CONF_GCLK_SERCOM3_CORE_SRC);
	_gclk_enable_channel(SERCOM3_GCLK_ID_SLOW, CONF_GCLK_SERCOM3_SLOW_SRC);
	
	i2c_m_sync_init(&I2C_0, SERCOM3);
	
	gpio_set_pin_pull_mode(PA22, GPIO_PULL_OFF);
	gpio_set_pin_function(PA22, PINMUX_PA22C_SERCOM3_PAD0);

	gpio_set_pin_pull_mode(PA23, GPIO_PULL_OFF);
	gpio_set_pin_function(PA23, PINMUX_PA23C_SERCOM3_PAD1);

	i2c_m_sync_get_io_descriptor(&I2C_0, &i2c0);
	i2c_m_sync_enable(&I2C_0);
	
}

void IRQ_init(void){
	_gclk_enable_channel(EIC_GCLK_ID, CONF_GCLK_EIC_SRC);
	gpio_set_pin_direction(RF_INT, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(RF_INT, GPIO_PULL_OFF);
	gpio_set_pin_function(RF_INT, PINMUX_PA07A_EIC_EXTINT7);
	
	gpio_set_pin_direction(RTC_INT, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(RTC_INT, GPIO_PULL_OFF);
	gpio_set_pin_function(RTC_INT, PINMUX_PA25A_EIC_EXTINT13);

	ext_irq_init();
}



void mcu_init(void)
{
	init_mcu();
	GPIO_init();
	SPI_init();
	I2C_init();
	IRQ_init();
	
	
	ext_irq_register(RTC_INT, I2C_RTC_Handler);
	ext_irq_register(RF_INT, RF_int_Handler);	
	
	delay_driver_init();
	//...Peripherial init
	
}