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

extern connection lnWrapper , lniec , rs485;

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
    	printf("main|========>>>>>> lniec.kek=%s - size=%d \n", lniec.settings.base.kek.data, lniec.settings.base.kek.size);
    	printf("main|========>>>>>> lnWrapper.kek=%s - size=%d \n", lnWrapper.settings.base.kek.data, lnWrapper.settings.base.kek.size);
        for(int i=0; i<lnWrapper.settings.base.kek.size; i++)
        {
        	printf("%.2X  ", lnWrapper.settings.base.kek.data[i]);
        }
        printf("\n");
//    	//securitySetupHighGMac.securityPolicy = securitySetupManagementClient.securityPolicy;
    	printf("cipherKey:");
    	for (int i=0; i<lnWrapper.settings.base.cipher.blockCipherKey.size; i++)
    	{
    		printf("%.2X  ", lnWrapper.settings.base.cipher.blockCipherKey.data[i]);
    	}
    	printf("\n");

    	printf("autheticationKey:");
    	for (int i=0; i<lnWrapper.settings.base.cipher.authenticationKey.size; i++)
    	{
    		printf("%.2X  ", lnWrapper.settings.base.cipher.authenticationKey.data[i]);
    	}
    	printf("\n");
    }
    return 0;
}












