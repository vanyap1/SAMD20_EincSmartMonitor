/*

*/


#include "u8x8.h"

/*=================================================*/

static const u8x8_display_info_t u8x8_ssd1607_200x200_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* values from SSD1606 */
  /* post_chip_enable_wait_ns = */ 120,
  /* pre_chip_disable_wait_ns = */ 60,
  /* reset_pulse_width_ms = */ 100, 	
  /* post_reset_wait_ms = */ 100, 
  /* sda_setup_time_ns = */ 50,		/* SSD1606: */
  /* sck_pulse_width_ns = */ 100,	/* SSD1606: 100ns */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	
  /* tile_width = */ 38,		/* 38*8 = 304 */
  /* tile_height = */ 19,		
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 38*8,
  /* pixel_height = */ 19*8		
};


static const uint8_t u8x8_d_ssd1607_200x200_powersave0_seq[] = {
  
};

static const uint8_t u8x8_d_ssd1607_200x200_powersave1_seq[] = {

};


static const uint8_t u8x8_d_ssd1607_200x200_exec_1000dly_seq[] = {

};


static void u8x8_d_ssd1607_200x200_first_init(u8x8_t *u8x8)
{
	u8x8_cad_StartTransfer(u8x8);
    u8x8_cad_SendCmd(u8x8, 0x10);
    uint8_t dat[19] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	for(uint16_t bytesY = 0; bytesY < 304; bytesY ++){
		u8x8_cad_SendData(u8x8, 19, dat);
    }
	
	//u8x8_cad_SendCmd(u8x8, 0x11);
	//u8x8_cad_SendArg(u8x8, 0x0);
	
    u8x8_cad_SendCmd(u8x8, 0x13);
    for(uint16_t bytesY = 0; bytesY < 304; bytesY ++){
		u8x8_cad_SendData(u8x8, 19, dat);
    }
	//u8x8_cad_SendCmd(u8x8, 0x11);
	//u8x8_cad_SendArg(u8x8, 0x0);
	
	 u8x8_cad_EndTransfer(u8x8);
}

static uint8_t *u8x8_convert_tile_for_ssd1607(uint8_t *t)
{
  uint8_t i;
  static uint8_t buf[8];
  uint8_t *pbuf = buf;

  for( i = 0; i < 8; i++ )
  {
    *pbuf++ = (*t++);
  }
  return buf;
}

static void u8x8_d_ssd1607_draw_tile(u8x8_t *u8x8, uint8_t arg_int, void *arg_ptr) U8X8_NOINLINE;
static void u8x8_d_ssd1607_draw_tile(u8x8_t *u8x8, uint8_t arg_int, void *arg_ptr)
{
  uint16_t x;
  uint16_t c, page;
  uint8_t *ptr;
  u8x8_cad_StartTransfer(u8x8);
//
	//u8x8_cad_SendCmd(u8x8, 0x10);
	//uint8_t dat[19] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//for(uint16_t bytesY = 0; bytesY < 304; bytesY ++){
		//u8x8_cad_SendData(u8x8, 19, dat);
	//}
	//u8x8_cad_SendCmd(u8x8, 0x13);
	//for(uint16_t bytesY = 0; bytesY < 304; bytesY ++){
		//u8x8_cad_SendData(u8x8, 19, dat);
	//}

  page = u8x8->display_info->tile_height;
  page --;
  page -= (((u8x8_tile_t *)arg_ptr)->y_pos);
  
  x = ((u8x8_tile_t *)arg_ptr)->x_pos;
  x *= 8;
  x += u8x8->x_offset;
  
  
  u8x8_cad_SendCmd(u8x8, 0x91);		//Partial Window (PTL)
  u8x8_cad_SendCmd(u8x8, 0x90);		//Partial In (PTIN)
  
  u8x8_cad_SendArg(u8x8, page << 3);		//HRST[7:3]
  u8x8_cad_SendArg(u8x8, page << 3);		//HRED[7:3]

  u8x8_cad_SendArg(u8x8, 0x00);		//VRST[8]
  u8x8_cad_SendArg(u8x8, 0x00);		//VRST[7:0]

  u8x8_cad_SendArg(u8x8, 0x01);		//VRED[8]
  u8x8_cad_SendArg(u8x8, 0x30);		//VRED[7:0]
  u8x8_cad_SendArg(u8x8, 1);		//PT_SCAN
  
  
 
  u8x8_cad_SendCmd(u8x8, 0x10 );
  
  
  do
  {
    c = ((u8x8_tile_t *)arg_ptr)->cnt;
    
	ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
    do
    {
      u8x8_cad_SendData(u8x8, 8, u8x8_convert_tile_for_ssd1607(ptr));
	  //u8x8_cad_SendData(u8x8, 8, 0xff);
      ptr += 8;
      x += 8;
      c--;
    } while( c > 0 );
    
    arg_int--;
  } while( arg_int > 0 );
  
  
	u8x8_cad_SendCmd(u8x8, 0x92);		//Partial Out (PTOUT)
	
	
	//u8x8_cad_SendCmd(u8x8, 0x50);
	//u8x8_cad_SendArg(u8x8, 0b00000001);
	
  u8x8_cad_EndTransfer(u8x8);
}



/*=================================================*/


#define L(a,b,c,d) (((a)<<6)|((b)<<4)|((c)<<2)|(d))


static const uint8_t u8x8_d_ssd1607_200x200_init_seq[] = {    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  //U8X8_CA(0x10, 0x00),	/* Deep Sleep mode Control: Disable */
  U8X8_C(0x06), U8X8_A(0x17),U8X8_A(0x17),U8X8_A(0x17),
  U8X8_C(0x01), U8X8_A(0x03),U8X8_A(0x00),U8X8_A(0x2b),U8X8_A(0x2b),U8X8_A(0x09),
  U8X8_C(0x3c),
  U8X8_C(0x04),
  U8X8_DLY(20),
  U8X8_C(0x0), U8X8_A(0x0F), //out_form (Need to check it)
  U8X8_C(0x30), U8X8_A(0x3a),
  U8X8_C(0x61), U8X8_A(152), U8X8_A(304-255), U8X8_A(255),
  U8X8_C(0x82), U8X8_A(0x12),
  U8X8_C(0x50), U8X8_A(0x87),
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1607_to_display_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  U8X8_C(0x12),						/* Refresh screen*/
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_ssd1607_200x200(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1607_200x200_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
	 
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_init_seq);    
	  delay_ms(20);
	  u8x8_d_ssd1607_200x200_first_init(u8x8);
	  delay_ms(20);
      //u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_powersave0_seq);
      
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      break;
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_d_ssd1607_draw_tile(u8x8, arg_int, arg_ptr);
      break;
    case U8X8_MSG_DISPLAY_REFRESH:
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_to_display_seq);
      break;
    default:
      return 0;
  }
  return 1;
}


/*=================================================*/
/* there is no improvement possible... so i consider the v2 version as obsolete */


static const uint8_t u8x8_d_ssd1607_v2_to_display_seq[] = {
};

/*=================================================*/
/* GDEP015OC1 */
/* https://github.com/olikraus/u8g2/issues/454 */


static const uint8_t u8x8_d_ssd1607_gd_to_display_seq[] = {
  
};





/*=================================================*/

static const uint8_t u8x8_d_ssd1607_ws_to_display_seq[] = {
  
};


static const uint8_t u8x8_d_ssd1607_ws_to_refresh_seq[] = {
 
};


/* waveshare 200x200 */
static const uint8_t u8x8_d_ssd1607_ws_200x200_init_seq[] = {    
           			/* end of sequence */  
  
};

