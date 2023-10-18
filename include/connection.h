//
// --------------------------------------------------------------------------
//  Gurux Ltd
//
//
//
// Filename:        $HeadURL:  $
//
// Version:         $Revision:  $,
//                  $Date:  $
//                  $Author: $
//
// Copyright (c) Gurux Ltd
//
//---------------------------------------------------------------------------

#ifndef CONNECTION_H
#define CONNECTION_H

#include "main.h"

#include "../../development/include/bytebuffer.h"
#include "../../development/include/dlmssettings.h"
#include <stdio.h>


#include <pthread.h>
#include "DSI_ConnectManager.h"
#include "ql_nw.h"
#include "ql_dev.h"

static const unsigned int RECEIVE_BUFFER_SIZE = 200;

#ifdef  __cplusplus
extern "C" {
#endif


#define true 	1
#define false 	0


static uint32_t Boudrate[]={ 300 , 600 , 1200 , 2400 , 4800 , 9600 , 19200 , 38400 , 57600 , 115200 };

typedef enum
{
	STOP_A_DATA_CALL 				= 0,
	START_A_DATA_CALL 				= 1,
	START_AND_REDIAL 				= 2,
	START_AND_REDIAL_OR_POWER_ON 	= 3
}START_STOP_DATACALL;

typedef enum
{
	IPV4V6	= 0,
	IPV4	= 1,
	IPV6	= 2
}IP_TYPE;

typedef enum {
	TCP		=2,
	UDP		=1
}SocketType;

//typedef struct
//{
//    unsigned char TX[2048];
//    unsigned char RX[2048];
//    uint32_t TX_Count;
//    uint32_t RX_Count;
//    uint32_t	Timeout_ms;
//} Buffer;

typedef struct {
	char		Connected;
	char		Opened;
	char		Valid;
}SocktStatus;

typedef struct {
	unsigned char	IP[16];
	uint16_t		PORT;
	SocketType		Type;
}SocktParameters;

typedef struct{
	SocktParameters		Parameters;
	SocktStatus			Status;
	int					Socket_fd;
	char 				ReceiveFlag;
//	struct timeval		Flag_Time;
	struct sockaddr_in 	Serv_addr;
	int 				Error;
	socklen_t 			ErrorLen;
	uint16_t			keepalive;
} CLIENTSOCKET;

typedef struct
{
    //Is trace used.
    unsigned char trace;

    //Socked handle.
    CLIENTSOCKET socket;
    //Serial port handle.
    int comPort;
    //Receiver thread handle.
    pthread_t receiverThread;
    pthread_t sendThread;
    pthread_t managerThread;
    unsigned long   waitTime;
    //Received data.
    gxByteBuffer data;
    //If receiver thread is closing.
    unsigned char closing;
    dlmsServerSettings settings;
    Buffer buffer;
} connection;


typedef struct
{
	int 	op			;
	char 	ip_addr[32]	;

}WAN_PARAM_STRUCT_TYPEDEF;

typedef struct
{
	int				op			;
	int				profile_idx	;
	int 			ip_type		;
	unsigned long 	pin_code	;
	char* 			apn			;
	char 			userName[20];
	char 			password[20];
}APN_PARAM_STRUCT_TYPEDEF;

void con_initializeBuffers(
    connection* connection,
    int size);


uint16_t GetLinuxBaudRate(uint32_t baudRate);





int com_updateSerialportSettings(connection* con,
    unsigned char iec,
    uint32_t baudRate);





int com_initializeSerialPort(connection* con,
    char* serialPort,
    unsigned char iec);


//void ListenerThread(void* pVoid);
void Socket_Receive_Thread(void* pVoid);

void Socket_Send_Thread(void* pVoid);

int Socket_Connection_Start(connection* con);

int Socket_Manage (connection* con);

void Socket_get_open(connection* con);

int	Socket_get_close(connection* con);

int Socket_create(connection* con);

void* UnixListenerThread(void* pVoid);

void* UnixSerialPortThread(void* pVoid);


void* Unixrs485RecSerialThread(void* pVoid);

void* Unixrs485SendSerialThread(void* pVoid);

void GW_Start (void* pVoid);

int svr_listen_serial(
    connection* con,
     char *file);


int rs485_listen_serial(
    connection* con,
     char *file);


int svr_listen_TCP(
    connection* con,
    unsigned short port);




//Close connection..
int con_close(
    connection* con);




// void report(char *format, ... );

void Initialize (void);


void LTE_Manager_Start (void);


void IMEI_Get (void);


void Sim_Init (void);


void ICCID_Get (void);


void Device_Init (void);


void NW_Init (void);


void WAN_Init (void);


void WAN_Connection (void);



#ifdef  __cplusplus
}
#endif

#endif //CONNECTION_H
