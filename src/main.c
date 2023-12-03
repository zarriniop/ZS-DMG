/*
 ============================================================================
 Name        : EC200Clan.c
 Author      : ImanMN and etc
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
extern connection 			lnWrapper , rs485;
DS1307_I2C_STRUCT_TYPEDEF	DS1307_Str	;

pthread_t 			RS_test;
void RS485_Test (void)
{
	while(1)
	{
		rs485.buffer.TX_Count = 100;
		sleep(5);
	}
}

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

    uint8_t buf_rs[100]={0};

    for (int i=0; i<100; i++)
    {
    	rs485.buffer.TX[i]=0x01;
    }

    pthread_create(&RS_test, 	NULL, RS485_Test, NULL);

    while (1)
    {
//    	Set_System_Date_Time(&DS1307_Str)	;
    	sleep(1)							;
    }
    return 0;
}












