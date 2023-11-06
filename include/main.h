/*
 * EC200Clan.h
 *
 *  Created on: Jul 19, 2023
 *      Author: Iman Mehraban
 */

#ifndef MAIN_H_
#define MAIN_H_

/************
 * INCLUDES *
 ************/
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include "DSI_ConnectManager.h"
#include "ql_nw.h"
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include "ql_sms.h"
#include "ql_sim.h"
#include <string.h>
#include <ql_gpio.h>
#include <pthread.h>
#include <bits/alltypes.h>
#include <arpa/inet.h>
#include "ql_uart.h"
#include "ql_i2c.h"
#include "stdbool.h"
#include "ql_i2c.h"

/* HAJIAN */
#define closesocket close
#include <unistd.h>
#include <termios.h>

#include "cosem.h"
#include "gxaes.h"

/***********
 * DEFINES *
 ***********/

/*HAJIAN*/
#define HDLC_HEADER_SIZE 		17
#define HDLC_BUFFER_SIZE 		128
#define PDU_BUFFER_SIZE 		1024
#define WRAPPER_BUFFER_SIZE 	8 + PDU_BUFFER_SIZE

#define OPTIC_SERIAL_FD 		"/dev/ttyS1"
#define RS485_SERIAL_FD 		"/dev/ttyS2"

#define	ONESHOT_TRIG_LED_DATA	"echo oneshot > /sys/devices/platform/leds/leds/LED1/trigger"
#define	ONESHOT_TRIG_LED_485	"echo oneshot > /sys/devices/platform/leds/leds/LED2/trigger"
#define	PATTERN_TRIG_LED_NET	"echo pattern > /sys/devices/platform/leds/leds/LED3/trigger"
#define LED_DATA_OFFDLY			"echo 10 > /sys/devices/platform/leds/leds/LED1/delay_off"
#define LED_485_OFFDLY			"echo 10 > /sys/devices/platform/leds/leds/LED2/delay_off"
#define LED_DATA_ONDLY			"echo 10 > /sys/devices/platform/leds/leds/LED1/delay_on"
#define LED_485_ONDLY			"echo 10 > /sys/devices/platform/leds/leds/LED2/delay_on"

/*************************
 * TYPEDEFS & STRUCTURES *
 *************************/
typedef struct
{
    unsigned char TX[2048];
    unsigned char RX[2048];
    uint32_t TX_Count;
    uint32_t RX_Count;
    uint32_t	Timeout_ms;
} Buffer;


typedef enum
{
	RS485 		= 0,
	SERVER		= 1,
	CLIENT		= 2,
	OPTICAL		= 3,
	START_APP	= 4
}REPORT_INTERFACE;


typedef enum
{
	RX 			= 0,
	TX			= 1,
	CONNECTION	= 2,
	START		= 3
}REPORT_MESSAGE;

/***********************
 * FUNCTION PROTOTYPES *
 ***********************/
int Servers_Start(int trace);

void Servers_Monitor (void);

int report (REPORT_INTERFACE Interface, REPORT_MESSAGE Message, char *Information);

const char * get_time(void);

void LED_Init (void);




#endif /* MAIN_H_ */
