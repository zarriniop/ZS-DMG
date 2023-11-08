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
#define	NONE_TRIG_LED_DATA		"echo none > /sys/devices/platform/leds/leds/LED1/trigger"
#define	NONE_TRIG_LED_485		"echo none > /sys/devices/platform/leds/leds/LED2/trigger"
#define	NONE_TRIG_LED_NET		"echo none > /sys/devices/platform/leds/leds/LED3/trigger"
#define	LED_DATA_ON_ALWAYS		"echo 1 > /sys/devices/platform/leds/leds/LED1/brightness"
#define	LED_485_ON_ALWAYS		"echo 1 > /sys/devices/platform/leds/leds/LED2/brightness"
#define	LED_NET_ON_ALWAYS		"echo 1 > /sys/devices/platform/leds/leds/LED3/brightness"
#define	LED_DATA_OFF_ALWAYS		"echo 0 > /sys/devices/platform/leds/leds/LED1/brightness"
#define	LED_485_OFF_ALWAYS		"echo 0 > /sys/devices/platform/leds/leds/LED2/brightness"
#define	LED_NET_OFF_ALWAYS		"echo 0 > /sys/devices/platform/leds/leds/LED3/brightness"
#define	ONESHOT_TRIG_LED_DATA	"echo oneshot > /sys/devices/platform/leds/leds/LED1/trigger"
#define	ONESHOT_TRIG_LED_485	"echo oneshot > /sys/devices/platform/leds/leds/LED2/trigger"
#define	PATTERN_TRIG_LED_NET	"echo pattern > /sys/devices/platform/leds/leds/LED3/trigger"
#define LED_DATA_OFFDLY			"echo 10 > /sys/devices/platform/leds/leds/LED1/delay_off"
#define LED_485_OFFDLY			"echo 10 > /sys/devices/platform/leds/leds/LED2/delay_off"
#define LED_DATA_ONDLY			"echo 10 > /sys/devices/platform/leds/leds/LED1/delay_on"
#define LED_485_ONDLY			"echo 10 > /sys/devices/platform/leds/leds/LED2/delay_on"

#define HDLC_HEADER_SIZE 		17
#define HDLC_BUFFER_SIZE 		128
#define PDU_BUFFER_SIZE 		1024
#define WRAPPER_BUFFER_SIZE 	8 + PDU_BUFFER_SIZE

#define OPTIC_SERIAL_FD 		"/dev/ttyS1"
#define RS485_SERIAL_FD 		"/dev/ttyS2"

/***** PROTOTYPES *****/
int 			Servers_Start	(int trace)	;
void 			Servers_Monitor (void)		;
const char * 	get_time		(void)		;
void 			LED_Init 		(void)		;
void 			File_Init 		(char* argv[])	;
int 			report 			(REPORT_INTERFACE Interface, REPORT_MESSAGE Message, char *Information);

#endif /* TOOLS_H_ */
