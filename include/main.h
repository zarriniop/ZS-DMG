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
#include <uci.h>
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

/*FLAGS*/
#define 	UP						1
#define 	DOWN 					0

/*TCP*/
#define		LANSOCKET_BUFFER_SIZE	8192

/*UART*/
#define 	QL_UART_MAIN_DEV  		"/dev/ttyS2"
#define		UART_READ_BUF_SIZE		8192
#define		UART_WRITE_BUF_SIZE		8192

/*I2C*/
#define 	I2C_DEV          		"/dev/i2c-0"
#define 	DS1307_I2C_SLAVE_ADDR	0x68			//codec 5616

/*HDML*/
#define		MAX_HES2GW_BUFF_SIZE	65803
#define		MAX_GW2HES_BUFF_SIZE	65803

/*HAJIAN*/
#define HDLC_HEADER_SIZE 		17
#define HDLC_BUFFER_SIZE 		128
#define PDU_BUFFER_SIZE 		1024
#define WRAPPER_BUFFER_SIZE 	8 + PDU_BUFFER_SIZE


/*************************
 * TYPEDEFS & STRUCTURES *
 *************************/

typedef struct
{
	char		TEL1	[20];
	char		TEL2	[20];
	char		TEL3	[20];
	char		TEL4	[20];
	char		TEL5	[20];
	char		S_IP	[20];
	char		APN		[20];
	char		APN_USER[20];
	char		APN_PASS[20];
	uint32_t	S_PORT		;
}SETTINGS_STRUCT_TYPEDEF;


typedef struct
{
	bool WAN;
}RESET_STRUCT_TYPEDEF;


typedef struct
{
	bool WAN_IPv4_Started;
}WAN_STRUCT_TYPEDEF;


typedef struct
{
	bool TCP_Ready_To_Start	;
	bool Connected			;
	bool Error_Send_Receive	;
}TCP_STRUCT_TYPEDEF;


typedef struct
{
	RESET_STRUCT_TYPEDEF 	Reset_Struct;
	WAN_STRUCT_TYPEDEF		WAN_Struct	;
	TCP_STRUCT_TYPEDEF		TCP_Struct	;
}FLAGS_STRUCT_TYPEDEF;


typedef struct
{
	int		profile_idx	;
	int 	ip_type		;
	char 	apn		[20];
	char 	userName[20];
	char 	password[20];

}APN_PARAM_STRUCT_TYPEDEF;


typedef struct
{
	int 	op			;
	char 	ip_addr[32]	;

}WAN_PARAM_STRUCT_TYPEDEF;


typedef struct
{
	char 	phone_number[20]		;
	char 	SMS_data[1024]			;
	char	SMS_response[1024]		;
	int 	SMS_type				;
	int		dealmode				;
	int 	messagemode				;

}SMS_PARAM_STRUCT_TYPEDEF;


typedef struct
{
	uint16_t 	Server_Port						;
	char 		Server_IP[30]					;
	struct 		sockaddr_in 	Dest			;
	int		 	tcp_client_sockfd				;
	char		RX_Buffer[LANSOCKET_BUFFER_SIZE];
	int			RX_Size							;
}TCP_PARAM_STRUCT_TYPEDEF;


typedef struct
{
	int 			fd_uart							;
	char			RX_buffer[UART_READ_BUF_SIZE]	;
	char			TX_buffer[UART_WRITE_BUF_SIZE]	;
	uint32_t		RX_size							;
	uint32_t		TX_size							;
	uint32_t		baud_rate						;
	Enum_FlowCtrl 	flowctrl						;
}MAIN_UART_STRUCT_TYPEDEF;


typedef struct
{
	MAIN_UART_STRUCT_TYPEDEF Main_UART_Struct;
}UART_STRUCT_TYPEDEF;

typedef struct
{
	int fd_i2c;
}I2C_STUCT_TYPEDEF;

typedef struct
{
	uint8_t second	;
	uint8_t minute	;
	uint8_t hour	;
	uint8_t day		;
	uint8_t date	;
	uint8_t month	;
	uint8_t year	;
	bool	H_12	;
}DS1307_I2C_STRUCT_TYPEDEF;


typedef struct
{
	uint8_t 	HES2GW_Buf[MAX_HES2GW_BUFF_SIZE];
	uint8_t 	GW2HES_Buf[MAX_GW2HES_BUFF_SIZE];
	uint16_t	HES2GW_Size						;
	uint16_t 	GW2HES_Size						;
}GW_PRTCL_TYPEDEF_STRUCT;


typedef struct
{
	GW_PRTCL_TYPEDEF_STRUCT GW_prtcl_struct;
}DLMS_STRUCT_TYPEDEF;


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

typedef enum
{
	PDU_SMS_MODE 	= 0,
	TEXT_SMS_MODE 	= 1
}SMS_MODE;

typedef enum
{
	GSM_7BIT_SMS_ENCOD_MODE	= 0,
	UCS_2_SMS_ENCOD_MODE 	= 1
}SMS_TYPE;


/**********
 * TABLES *
 **********/
static const u_int16_t FCS16Table[] = {

       0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
       0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
       0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
       0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876,
       0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD,
       0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
       0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C,
       0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974,
       0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
       0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3,
       0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A,
       0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
       0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9,
       0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1,
       0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
       0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70,
       0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7,
       0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
       0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036,
       0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E,
       0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
       0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD,
       0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134,
       0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
       0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3,
       0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB,
       0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
       0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
       0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1,
       0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
       0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330,
       0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78};


/***********************
 * FUNCTION PROTOTYPES *
 ***********************/
void 			Err_Disp (char section[250], uint32_t ret);
void 			NW_Init (void);
void 			WAN_Init (void);
void 			WAN_Connection (void);
void 			SMS_Processing (char *SMS_Text, int SMS_Len, char *SMS_Response);
void 			SMS_All_Del (void);
void 			Reset_Processing (void);
uint8_t 		Check_Valid_Phone_Numbers (char *Phone_Number);
void 			SMS_Receive_handlerPtr (QL_SMS_NFY_MSG_ID msg_id, void *pv_data, int pv_data_len, void *contextPtr);
int 			SMS_Init (void);
int 			SMS_Send(char *Tel_Num, char *Text, SMS_MODE SMS_Mode, SMS_TYPE SMS_Type);
void 			Param_Init (void);
void 			Initialize (void);
void 			Pthreads_Func (void);
void 			TCP_Connection (void);
int 			TCP_Send_Data (int TCP_Socket, char* Data_Buffer);
int 			TCP_Client_Check (void);
void 			Close_Socket(int* TCP_Client_SockFD);
void 			TCP_Rece_Data (void);
void 			UART_Init (void);
void 			UART_Main_Read (void);
uint32_t 		UART_Main_Write (char *TX_Buff);
void 			I2C_Init (void);
int 			DS1307_Set_Time (uint8_t year, uint8_t month, uint8_t date, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, bool H_12);
void 			DS1307_Get_Time (DS1307_I2C_STRUCT_TYPEDEF *DS1307_Time);
uint8_t 		DLMS2GW_Frame_Convertor (uint8_t* DLMS2GW_Prtcl_Frame, uint8_t Logical_Address, uint8_t Client_Address, uint8_t Control_Byte, uint16_t GW_DST_Add);
uint8_t 		Meter2GW_Frame_Convertor (uint8_t* Meter2GW_Prtcl_Frame, uint8_t Source_Address);
static uint16_t countCRC(char* Buff, uint32_t index, uint32_t count);







#endif /* MAIN_H_ */
