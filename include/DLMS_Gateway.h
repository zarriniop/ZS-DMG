/*
 * DLMS_Gateway.h
 *
 *  Created on: Oct 2, 2023
 *      Author: mhn
 */

#ifndef DLMS_GATEWAY_H_
#define DLMS_GATEWAY_H_

/****************
 *	Includes	*
 ****************/
#include "main.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <math.h>
//
//#include <termios.h>
//#include <fcntl.h>
//#include <unistd.h>
//
//#include <sys/types.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <netinet/tcp.h>
//#include <netinet/udp.h>
//#include <string.h>
//#include <netdb.h>
//#include <stdio.h>
//#include <fcntl.h>
//#include <string.h>
//#include <sys/time.h>
//#include <arpa/inet.h>  /* for sockaddr_in */
//#include <termios.h>
//#include <errno.h>
//#include <pthread.h>
//#include <poll.h>
//#include <stdarg.h>
//#include <sys/mman.h>
//#include <termios.h>
//#include <fcntl.h>
//#include <errno.h>
//
//
//#include <arpa/inet.h> // inet_addr()
//#include <netdb.h>
//#include <strings.h> // bzero()
//#include <sys/socket.h>
//#include <unistd.h> // read(), write(), close()
//#include <pthread.h>
//
//#include <stdbool.h>



/************
 *	Defines	*
 ************/
#define		MAX_BUFFER_LEN		8192

#define		MAX_CLIENT_NUMBER	5
#define		MAX_SERVER_NUMBER	3

#define		UP		1
#define		DOWN	0

#define		TRUE	1
#define		FALSE	0

#define		LLC_SUB_LAYER_SIZE	3

#define		HEADER_PREFIX_MIN_SIZE_GW_PRTCL	11
#define		MIN_HDLC_SIZE					10
#define		PHY_ADD_START_BYTE				11
#define		START_FLAG_BYTE					0
#define		FRAME_FRMT_START_BYTE			1
#define		DST_ADD_START_BYTE				3
#define		MIN_SNRM_SIZE					8
#define		MAC_MIN_APDU_START_BYTE			8

#define 	MIN_GW_APDU_LEN_SIZE			3
#define		VERSION_START_BYTE				0
#define		HES_SRC_ADD_START_BYTE			2
#define		HES_DST_ADD_START_BYTE			4
#define		APDU_LEN_START_BYTE				6
#define		HEADER_BYTE						8
#define		NETWORK_ID_BYTE					9
#define		ADD_LEN_BYTE					10
#define		HES_PHY_ADD_START_BYTE			11
#define		HES_MIN_FRAME_SIZE				11

#define		SINGLE_FRAME_IN_SEGMENTATION	1		//0b 001
#define		MIDDLE_FRAME_IN_SEGMENTATION	2		//0b 010
#define		LAST_FRAME_IN_SEGMENTATION		4		//0b 100

#define		AARQ_TAG						0x60
#define		RLRQ_TAG						0x62

#define 	SNRM_CONTROL_BYTE				0x93
#define		AARQ_CONNECTION_CNTROLBYTE		0x10

#define 	TIMEOUT_FOR_RECEIVE_FROM_METER  5000


/****************
 *	Typedefs	*
 ****************/

//typedef enum {
//	TCP		=2,
//	UDP		=1
//}SocketType;

//typedef struct
//{
//	char 		IP[30]	;
//	uint16_t	PORT	;
//	SocketType	Type	;
//}SOCKET_PARAM_TYPEDEF;

//typedef struct {
//	bool		Connected;
//	bool		Opened;
//	bool		Valid;
//}SOCKET_STS_TYPEDEF;

//typedef struct
//{
//    unsigned char TX[2048];
//    unsigned char RX[2048];
//    uint32_t TX_Count;
//    uint32_t RX_Count;
//    uint32_t Timeout_ms;
//} Buffer;


typedef enum
{
	INFORMATION 				= 0,
	RECEIVE_READY 				= 1
}FRAME_TYPE;


typedef enum {
	OK			=0,
	ERROR		=-1,
	NOT_ALLOW	=-2

}result;

typedef enum {
	BAUD0			=B0,
	BAUD50 			=B50,
	BAUD75 			=B75,
	BAUD110 		=B110,
	BAUD134 		=B134,
	BAUD150 		=B150,
	BAUD200 		=B200,
	BAUD300 		=B300,
	BAUD600 		=B600,
	BAUD1200 		=B1200,
	BAUD1800	 	=B1800,
	BAUD2400 		=B2400,
	BAUD4800 		=B4800,
	BAUD9600 		=B9600,
	BAUD19200 		=B19200,
	BAUD38400 		=B38400,
	BAUD57600 		=B57600,
	BAUD115200 		=B115200,
	BAUD230400 		=B230400,
	BAUD460800 		=B460800,

}Baud;


typedef enum {
	Bits5		=CS5,
	Bits6		=CS6,
	Bits7		=CS7,
	Bits8		=CS8,
}Bits;



typedef enum {
	PARITY_NONE			=1,
	PARITY_YES			=2,
}Pari;


typedef enum {
	STOPBITS_ONE		=1,
	STOPBITS_TWO		=2,
}StpBits;


typedef struct {
	char		*File;
	Baud		BaudRate;
	Bits		bitsPbyte;
	Pari		Parity;
	StpBits		StopBits;

}PortParameters;

typedef struct {
	int				Port_fd;
	PortParameters	Parameters;
	uint16_t		TX_Count;
	uint16_t		RX_Count;
	uint8_t 		TX_Buffer[8192];
	uint8_t 		RX_Buffer[8192];
	long			Timeout;
}SerialPort;


static const uint16_t FCS16Table[] = {

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

struct
{
	uint8_t RRR;
	uint8_t SSS;
	uint8_t Ctrl_Byte;
}Control_Byte_Struct;


typedef enum {
	READY_TO_GENERATE_SNRM		=	0,
	WAITING_FOR_SNRM_RESPONSE	=	1,
	RECEIVED_SNRM_RESPONSE		=	2,
	WAITING_FOR_RESPONSE		=	3,
	AARQ_RESPONSE_RECEIVED		=	4,
	WAITING_FOR_REQUEST			=	5,
	RECEIVE_READY_FOR_SB_MODE	=	6,
	WAITING_FOR_DISC_RESPONSE	=	7

}GW_STATE;


typedef enum {
	AARQ_RESPONSE_ERROR	= -3,
	AARQ_ERROR			= -2,
	SNRM_ERROR			= -1,
	SNRM_GENERATED		= 1 ,
	AARQ_CONVERTED		= 2 ,
	AARQ_RESPONSE		= 3
}HANDLE_GATEWAY;


typedef enum {
	GW_FRAME_IS_VALID 			= 0	,
	GW_FRAME_VALID_ERROR		= -1,
	GW_FRAME_VERSION_ERROR 		= -2,
	GW_FRAME_HEADER_ERROR 		= -3,
	GW_FRAME_NETWORK_ID_ERROR 	= -4
}GATEWAY_FRAME_VALIDATION;


/****************
 *	Prototypes	*
 ****************/
void			GW_Run							(Buffer* GW_STRUCT, Buffer* HDLC_STRUCT)						;
void 			GW_Run_Init						(Buffer* GW_STRUCT, Buffer* HDLC_STRUCT)						;
int8_t 			Handle_GW_Frame 				(Buffer* GW_STRUCT, Buffer* HDLC_STRUCT)						;
uint8_t 		GW2HDLC_SNRM_Generator 			(Buffer* GW_STRUCT, Buffer* HDLC_STRUCT)						;
uint16_t 		HDLC_Send_SNRM					(Buffer* GW_STRUCT, Buffer* HDLC_STRUCT)						;
int8_t 			GW2HDLC_DISC_Generator 			(Buffer* GW_STRUCT, Buffer* HDLC_STRUCT)						;
uint8_t 		HDLC_Send_DISC 					(Buffer* GW_STRUCT, Buffer* HDLC_STRUCT)						;
int8_t 			Check_GW_Frame_Valid 			(Buffer* GW_STRUCT)															;
uint8_t 		Check_GW_Frame_Type 			(Buffer* GW_STRUCT)															;
int16_t 		GW2HDLC_Frame_Convertor 		(Buffer* GW_STRUCT, Buffer* HDLC_STRUCT, uint8_t Control_Byte)	;
uint8_t 		GW2HDLC_Poll_For_Remained_Data 	(Buffer* GW_STRUCT, Buffer* HDLC_STRUCT, uint8_t Control_Byte)	;
int64_t 		Meter2GW_Frame_Convertor		(Buffer* HDLC_STRUCT, Buffer* GW_STRUCT)						;
uint8_t 		Control_Byte 					(uint8_t RRR, uint8_t SSS, FRAME_TYPE frame_type)										;
static uint16_t countCRC						(char* Buff, uint32_t index, uint32_t count)											;

#endif /* DLMS_GATEWAY_H_ */
