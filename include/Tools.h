/*
 * Tools.h
 *
 *  Created on: Nov 7, 2023
 *      Author: mhn
 */

#ifndef TOOLS_H_
#define TOOLS_H_

#include "main.h"

/***** DEFINES *****/
#define	NONE_TRIG_LED_485		"echo none > /sys/devices/platform/leds/leds/LED1/trigger"
#define	NONE_TRIG_LED_DATA		"echo none > /sys/devices/platform/leds/leds/LED2/trigger"
#define	NONE_TRIG_LED_NET		"echo none > /sys/devices/platform/leds/leds/LED3/trigger"

/*-----------------------------------------------------------------*/
#define	NONE_TRIG_LED_DATA		"echo none > /sys/devices/platform/leds/leds/LED2_G/trigger"
#define	NONE_TRIG_LED_DATA_R	"echo none > /sys/devices/platform/leds/leds/LED2_R/trigger"
#define	NONE_TRIG_LED_485_G		"echo none > /sys/devices/platform/leds/leds/LED1_G/trigger"
#define	NONE_TRIG_LED_485		"echo none > /sys/devices/platform/leds/leds/LED1_R/trigger"
#define	NONE_TRIG_LED_NET_4G	"echo none > /sys/devices/platform/leds/leds/LED3_G/trigger"
#define	NONE_TRIG_LED_NET_2G	"echo none > /sys/devices/platform/leds/leds/LED3_R/trigger"
/*-----------------------------------------------------------------*/

#define	LED_DATA_ON_ALWAYS		"echo 1 > /sys/devices/platform/leds/leds/LED2/brightness"
#define	LED_485_ON_ALWAYS		"echo 1 > /sys/devices/platform/leds/leds/LED1/brightness"
#define	LED_NET_ON_ALWAYS		"echo 1 > /sys/devices/platform/leds/leds/LED3/brightness"

/*-----------------------------------------------------------------*/
#define	LED_DATA_ON_ALWAYS_R	"echo 1 > /sys/devices/platform/leds/leds/LED2_R/brightness"
#define	LED_485_ON_ALWAYS_R		"echo 1 > /sys/devices/platform/leds/leds/LED1_R/brightness"
#define	LED_NET_ON_ALWAYS_2G_R	"echo 1 > /sys/devices/platform/leds/leds/LED3_R/brightness"
#define	LED_DATA_ON_ALWAYS_G	"echo 1 > /sys/devices/platform/leds/leds/LED2_G/brightness"
#define	LED_485_ON_ALWAYS_G		"echo 1 > /sys/devices/platform/leds/leds/LED1_G/brightness"
#define	LED_NET_ON_ALWAYS_4G_G	"echo 1 > /sys/devices/platform/leds/leds/LED3_G/brightness"
/*-----------------------------------------------------------------*/

#define	LED_DATA_OFF_ALWAYS		"echo 0 > /sys/devices/platform/leds/leds/LED2/brightness"
#define	LED_485_OFF_ALWAYS		"echo 0 > /sys/devices/platform/leds/leds/LED1/brightness"
#define	LED_NET_OFF_ALWAYS		"echo 0 > /sys/devices/platform/leds/leds/LED3/brightness"

/*-----------------------------------------------------------------*/
#define	LED_DATA_OFF_ALWAYS_R		"echo 0 > /sys/devices/platform/leds/leds/LED2_R/brightness"
#define	LED_485_OFF_ALWAYS_R		"echo 0 > /sys/devices/platform/leds/leds/LED1_R/brightness"
#define	LED_NET_OFF_ALWAYS_2G_R		"echo 0 > /sys/devices/platform/leds/leds/LED3_R/brightness"
#define	LED_DATA_OFF_ALWAYS_G		"echo 0 > /sys/devices/platform/leds/leds/LED2_G/brightness"
#define	LED_485_OFF_ALWAYS_G		"echo 0 > /sys/devices/platform/leds/leds/LED1_G/brightness"
#define	LED_NET_OFF_ALWAYS_4G_G		"echo 0 > /sys/devices/platform/leds/leds/LED3_G/brightness"
/*-----------------------------------------------------------------*/

#define	ONESHOT_TRIG_LED_DATA	"echo oneshot > /sys/devices/platform/leds/leds/LED2/trigger"
#define	ONESHOT_TRIG_LED_485	"echo oneshot > /sys/devices/platform/leds/leds/LED1/trigger"

/*-----------------------------------------------------------------*/
#define	ONESHOT_TRIG_LED_DATA	"echo oneshot > /sys/devices/platform/leds/leds/LED2_G/trigger"
#define	ONESHOT_TRIG_LED_485	"echo oneshot > /sys/devices/platform/leds/leds/LED1_R/trigger"
/*-----------------------------------------------------------------*/


#define	PATTERN_TRIG_LED_NET	"echo pattern > /sys/devices/platform/leds/leds/LED3/trigger"
#define LED_DATA_OFFDLY			"echo 10 > /sys/devices/platform/leds/leds/LED2/delay_off"
#define LED_485_OFFDLY			"echo 10 > /sys/devices/platform/leds/leds/LED1/delay_off"
#define LED_DATA_ONDLY			"echo 10 > /sys/devices/platform/leds/leds/LED2/delay_on"
#define LED_485_ONDLY			"echo 10 > /sys/devices/platform/leds/leds/LED1/delay_on"


/*-----------------------------------------------------------------*/
#define LED_DATA_OFFDLY			"echo 10 > /sys/devices/platform/leds/leds/LED2_G/delay_off"
#define LED_485_OFFDLY			"echo 10 > /sys/devices/platform/leds/leds/LED1_R/delay_off"
#define LED_DATA_ONDLY			"echo 10 > /sys/devices/platform/leds/leds/LED2_G/delay_on"
#define LED_485_ONDLY			"echo 10 > /sys/devices/platform/leds/leds/LED1_R/delay_on"
/*-----------------------------------------------------------------*/


#define HDLC_HEADER_SIZE 		17
#define HDLC_BUFFER_SIZE 		128
#define PDU_BUFFER_SIZE 		1224
#define WRAPPER_BUFFER_SIZE 	8 + PDU_BUFFER_SIZE

#define MAX_FILE_SIZE			2097152	//Bytes == 2MBytes (2*1024^2)

#define GPRS_KAT_IDEAL_CND		(gprs_kat_option->ideal_time >= 30) && (gprs_kat_option->ideal_time <= 1800)
#define GPRS_KAT_INTVL_CND		(gprs_kat_option->delay_retry_interval_value >= 10) && (gprs_kat_option->delay_retry_interval_value <= 20)

/***** PROTOTYPES *****/
int 			Servers_Start	(int trace)	;
void 			Servers_Monitor (void)		;
const char * 	get_time		(void)		;
void 			LED_Init 		(void)		;
void 			File_Init 		(char* argv[])	;
int 			report 			(REPORT_INTERFACE Interface, REPORT_MESSAGE Message, char *Information, ...)			;
void 			M2Sh			(uint16_t *j_y, uint8_t *j_m, uint8_t *j_d, uint16_t  g_y, uint8_t  g_m, uint8_t  g_d)	;
void 			SH2M 			(uint16_t *My, uint8_t *Mm, uint8_t *Md,long jy, long jm, long jd)						;
void 			exec			(const char* cmd,char *resultt,uint32_t time);
void 			exec			(const char* cmd,char *resultt,uint32_t time)	;
void 			GPRS_kat_gxData_Get_Value 	(GPRS_KAT_GXDATA_STR* GPRS_Kat_gxData, gxData* gprs_keep_alive_gurux);

#endif /* TOOLS_H_ */
