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
#include "sys/stat.h"

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


#endif /* MAIN_H_ */
