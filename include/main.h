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

/* HAJIAN */
#define closesocket close
#include <unistd.h>
#include <termios.h>

#include "exampleserver.h"
#include "connection.h"
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

#define RS485_SERIAL_FD 		"/dev/ttyS1"
#define OPTIC_SERIAL_FD 		"/dev/ttyS2"

/*************************
 * TYPEDEFS & STRUCTURES *
 *************************/


/***********************
 * FUNCTION PROTOTYPES *
 ***********************/
int startServers(int port, int trace);

void Svr_Monitor (void);




#endif /* MAIN_H_ */
