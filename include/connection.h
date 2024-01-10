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

extern gxPushSetup 	pushSetup;
extern gxData 		gprskeepalivetimeinterval;

#define LED_DATA_SHOT	"echo 1 > /sys/devices/platform/leds/leds/LED1/shot"
#define LED_485_SHOT	"echo 1 > /sys/devices/platform/leds/leds/LED2/shot"
#define PAT_0T_LED_NET	"echo 1 0 0 20000 > /sys/devices/platform/leds/leds/LED3/pattern"
#define PAT_1T_LED_NET	"echo 1 100 0 20000 > /sys/devices/platform/leds/leds/LED3/pattern"
#define PAT_2T_LED_NET	"echo 1 100 0 200 1 100 0 20000 > /sys/devices/platform/leds/leds/LED3/pattern"
#define PAT_3T_LED_NET	"echo 1 100 0 200 1 100 0 200 1 100 0 20000 > /sys/devices/platform/leds/leds/LED3/pattern"
#define GET_GATEWAY_IP	"uci get network.wan0.gateway"

#define GPIO_OUT_DIR	24

/*I2C*/
#define I2C_DEV          		"/dev/i2c-0"
#define DS1307_I2C_SLAVE_ADDR	0x68			//codec 5616


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


typedef enum {
	SIM_CARD_REMOVED =0,
	SIM_CARD_INSERTED=1
}SIM_Card_State;


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
	struct sockaddr_in 	Serv_addr;
	int 				Error;
	socklen_t 			ErrorLen;
} CLIENTSOCKET;

typedef struct{
	SocktStatus			Status;
	int					Socket_fd;
	int					Accept_fd;
	struct sockaddr_in 	Serv_addr;
	int 				Error;
	socklen_t 			ErrorLen;
	unsigned short		server_port;
} SERVERSOCKET;

typedef struct
{
    //Is trace used.
    unsigned char trace;

    //Socked handle.
    CLIENTSOCKET 		socket;
    SERVERSOCKET 		serversocket;
    //Serial port handle.
    int 				comPort;
    uint32_t			BASE_ADDR;
    //Receiver thread handle.
    pthread_t 			receiverThread;
    pthread_t 			sendThread;
    pthread_t 			managerThread;
    pthread_t 			serverstart;
    pthread_t 			serverlistenThread;
    unsigned long   	waitTime;
    //Received data.
    gxByteBuffer 		data;
    //If receiver thread is closing.
    unsigned char 		closing;
    dlmsServerSettings 	settings;
    Buffer 				buffer;
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

typedef struct
{
	int		I2C_fd	;
	uint8_t second	;		//0-60
	uint8_t minute	;		//0-60
	uint8_t hour	;		//0-24
	uint8_t day		;		//1-7
	uint8_t date	;		//1-31
	uint8_t month	;		//1-12
	uint8_t year	;		//0-99
	bool	H_12	;		//0-1
}DS1307_I2C_STRUCT_TYPEDEF;

void con_initializeBuffers(connection* connection, int size);


uint16_t GetLinuxBaudRate(uint32_t baudRate);

int com_updateSerialportSettings(connection* con, unsigned char iec, uint32_t baudRate);

int Quectel_Update_Serial_Port_Settings(connection* con, unsigned char iec, uint32_t baudRate);

int com_initializeSerialPort(connection* con, char* serialPort, unsigned char iec);



//void ListenerThread(void* pVoid);
void Socket_Receive_Thread(void* pVoid);

void Socket_Listen_Thread(void* pVoid);

void Socket_Send_Thread(void* pVoid);

int Socket_Connection_Start(connection* con);

int Socket_Manage_Thread (connection* con);

void Socket_get_open(connection* con);

int	Socket_get_close(connection* con);

int Socket_create(connection* con);

int Socket_Server(connection* con);


void GW_Start (void* pVoid);



int IEC_Serial_Start(connection* con, char *file);

void* IEC_Serial_Thread(void* pVoid);



int RS485_Serial_Start(connection* con, char *file);

void* RS485_Receive_Thread(void* pVoid);

void* RS485_Send_Thread(void* pVoid);




//Close connection..
int con_close(connection* con);




// void report(char *format, ... );

void Initialize (void);


void LTE_Manager_Start (void);


void IMEI_Get (void);


void Sim_Init (void);


void ICCID_Get (void);


void Device_Init (void);


void NW_Init (void);


void SIM_Card_Detection (void);


void WAN_Init (void);


void WAN_Connection (void);

long diff_time_us(struct timeval *start);
long diff_time_ms(struct timeval *start);
long diff_time_s (struct timeval *start);

int PushSetup_OnConnectivity();

int connectServer_pushon(char* address, char *port, int* s);

int closeServer(int* s);

void DS1307_Init (DS1307_I2C_STRUCT_TYPEDEF* DS1307_Time);

int DS1307_Set_Time (DS1307_I2C_STRUCT_TYPEDEF DS1307_Time);

void DS1307_Get_Time (DS1307_I2C_STRUCT_TYPEDEF* DS1307_Time);

void Set_System_Date_Time (DS1307_I2C_STRUCT_TYPEDEF* DS1307_Time);

void DS1307_Time_Date_Correcting (DS1307_I2C_STRUCT_TYPEDEF* DS1307_Time);


#ifdef  __cplusplus
}
#endif

#endif //CONNECTION_H
