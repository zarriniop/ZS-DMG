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


#define TCP_NODELAY 			1
#define TCP_MAXSEG	 			2
#define TCP_CORK	 			3
#define TCP_KEEPIDLE	 		4
#define TCP_KEEPINTVL	 		5
#define TCP_KEEPCNT	 			6
#define TCP_SYNCNT	 			7
#define TCP_LINGER2	 			8
#define TCP_DEFER_ACCEPT 		9
#define TCP_WINDOW_CLAMP 		10
#define TCP_INFO	 			11
#define	TCP_QUICKACK	 		12
#define TCP_CONGESTION	 		13
#define TCP_MD5SIG	 			14
#define TCP_THIN_LINEAR_TIMEOUTS 16
#define TCP_THIN_DUPACK  		17
#define TCP_USER_TIMEOUT 		18
#define TCP_REPAIR       		19
#define TCP_REPAIR_QUEUE 		20
#define TCP_QUEUE_SEQ    		21
#define TCP_REPAIR_OPTIONS 		22
#define TCP_FASTOPEN     		23
#define TCP_TIMESTAMP    		24
#define TCP_NOTSENT_LOWAT 		25
#define TCP_CC_INFO      		26
#define TCP_SAVE_SYN     		27
#define TCP_SAVED_SYN    		28
#define TCP_REPAIR_WINDOW 		29
#define TCP_FASTOPEN_CONNECT 	30
#define TCP_ULP          		31
#define TCP_MD5SIG_EXT   		32
#define TCP_FASTOPEN_KEY 		33
#define TCP_FASTOPEN_NO_COOKIE 	34
#define TCP_ZEROCOPY_RECEIVE   	35
#define TCP_INQ          		36

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
