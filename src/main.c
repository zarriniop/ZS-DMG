/*
 ============================================================================
 Name        : EC200Clan.c
 Author      : ImanMN, HadiMHN, AmirHJN
 Version     :
 Copyright   : Zarrin Samane Co
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

/************
 * INCLUDES *
 ************/
#include <main.h>
#include "exampleserver.h"
#include "connection.h"
#include "DLMS_Gateway.h"
#include "Tools.h"
#include <ql_oe.h>

DS1307_I2C_STRUCT_TYPEDEF	DS1307_Str	;

extern connection rs485;

int main(int argc, char* argv[])
{
	report(START_APP, START, "**********************************");

	File_Init (argv)					;
    LED_Init()							;
    DS1307_Init(&DS1307_Str)			;
    Set_System_Date_Time(&DS1307_Str)	;
    Servers_Start(GX_TRACE_LEVEL_INFO)	;
    LTE_Manager_Start()					;
//    pthread_create(&SVR_Monitor, NULL, Servers_Monitor, NULL);

    /***********************************/
//	for(int i=0; i<16; i++)
//	{
//		rs485.buffer.RX[i] = i;
//	}
//	rs485.buffer.RX[0] = 0x7E;
//	rs485.buffer.RX[1] = 0xA0;
//	rs485.buffer.RX[2] = 0x07;
//	rs485.buffer.RX[3] = 0x03;
//	rs485.buffer.RX[4] = 0x21;
//	rs485.buffer.RX[5] = 0x93;
//	rs485.buffer.RX[6] = 0x0F;
//	rs485.buffer.RX[7] = 0x01;
//	rs485.buffer.RX[8] = 0x7E;
//	rs485.buffer.RX[9] = 0;
//	rs485.buffer.RX[10] = 0;
//	rs485.buffer.RX[11] = 0;
//	rs485.buffer.RX[12] = 0;
//	rs485.buffer.RX[13] = 0;
//	rs485.buffer.RX[14] = 0;
//	rs485.buffer.RX[15] = 0;
//	rs485.buffer.RX_Count = 9;
	/**********************************/

    while (1)
    {
    	sleep(20)						;
    }
    return 0;
}












