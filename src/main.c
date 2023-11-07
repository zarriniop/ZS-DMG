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


int main(int argc, char* argv[])
{
	report(START_APP, START, "**********************************");
	DS1307_I2C_STRUCT_TYPEDEF	DS1307_Str;

	File_Init (argv)					;
    DS1307_Init(&DS1307_Str)			;

//    DS1307_Str.year		=23	;
//    DS1307_Str.month	=11	;
//    DS1307_Str.date		=7	;
//    DS1307_Str.day		=4	;
//    DS1307_Str.hour		=9	;
//    DS1307_Str.minute	=43	;
//    DS1307_Str.second	=0	;
//    DS1307_Str.H_12		=0	;
//    DS1307_Set_Time(DS1307_Str);



    Set_System_Date_Time(&DS1307_Str)	;
    LED_Init()							;
    Servers_Start(GX_TRACE_LEVEL_INFO)	;
    LTE_Manager_Start()					;
//    pthread_create(&SVR_Monitor, NULL, Servers_Monitor, NULL);

    while (1)
    {
    	Set_System_Date_Time(&DS1307_Str)	;
    	sleep(10)							;
    }
    return 0;
}












