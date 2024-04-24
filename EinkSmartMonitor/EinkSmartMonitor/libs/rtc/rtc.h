/*
 * rtc.h
 *
 * Created: 29.01.2023 21:32:59
 *  Author: User
 */ 

#include <stdint.h>

#ifndef RTC_H_
#define RTC_H_



#define RTC_ADDR		0x32

#define SECONDS_REG			0x00
#define MINUTES_REG			0x01
#define HOURS_REG			0x02
#define WEEKDAY_REG			0x03
#define DATE_REG			0x04
#define MONTS_REG			0x05
#define YEAR_REG			0x06
#define RAM_REG				0x07
#define MINUTEST_ALM_REG	0x08
#define HOURS_ALM_REG		0x09
#define WEEKDAY_ALM_REG		0x0A
#define DATE_ALM_REG		0x0A
#define TIMER_COUNTER0_REG	0x0B
#define TIMER_COUNTER1_REG	0x0C






//0Dh, 1Dh � Extension Register
#define EXT_REG				0x0D
#define TD					0	//
#define FD					2	//
#define TE					4	//
#define USEL				5	//
#define WADA				6	//Date is the source for the Alarm Interrupt function.
#define TEST				7	//This is a manufacturer�s test bit. Its value should always be 0. Avoid writing a 1 to this bit when writing in this register. Zero for normal operation.

//0Eh, 1Eh � Flag Register
#define FLAG_REG			0x0E
#define V1F					0	//
#define V2F					1	//
#define EVF					2	//
#define AF					3	//
#define TF					4	//
#define UF					5	//

//0Fh, 1Fh � Control Register
#define CONTROLL_REG		0x0F
#define RESET				0	//
#define EIE					2	//
#define AIE					3	//
#define TIE					4	//
#define UIE					5	//

//0Eh, 1Eh � Flag Register

#endif





typedef struct calendar_date {
	uint8_t second;        //!< 0-59
	uint8_t minute;        //!< 0-59
	uint8_t hour;          //!< 0-23
	uint8_t date;          //!< 0-30 \note First day of month is 0, not 1.
	uint8_t month;         //!< 0 January - 11 December
	uint16_t year;         //!< 1970-2105
	uint8_t dayofweek;     //!< 0 Sunday  - 6 Saturday
} rtc_date;

//second, minute, hour, date, month, year, dayofweek

	
	
void rtc_int_enable(rtc_date *RTCx);
void rtc_sync(rtc_date *RTCx);
void rtc_set(rtc_date *RTCx);

uint8_t BCDtoDEC(uint8_t val);
uint8_t DECtoBCD(uint8_t val);
uint32_t convert_to_timestamp(const rtc_date *date);