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

int main(int argc, char* argv[])
{
	report(START_APP, START, "**************ZS-DMG %d**************", VERSION_APP);

	File_Init (argv)					;
    LED_Init()							;
    DS1307_Init(&DS1307_Str)			;
    Set_System_Date_Time(&DS1307_Str)	;
    Servers_Start(GX_TRACE_LEVEL_INFO)	;
    LTE_Manager_Start()					;
//    pthread_create(&SVR_Monitor, NULL, Servers_Monitor, NULL);

    while (1)
    {
    	sleep(20)						;
//    	securitySetupHighGMac.securityPolicy = securitySetupManagementClient.securityPolicy;
    }
    return 0;
}












