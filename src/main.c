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
/********************************************|
|*			Variables & Definitions			*|
|********************************************/
unsigned char lnframeBuff	[HDLC_BUFFER_SIZE + HDLC_HEADER_SIZE]	;
unsigned char lnpduBuff		[PDU_BUFFER_SIZE]						;
unsigned char ln47frameBuff	[WRAPPER_BUFFER_SIZE]					;
unsigned char ln47pduBuff	[PDU_BUFFER_SIZE]						;

connection lnWrapper , lniec , rs485;

pthread_t SVR_Monitor;
/********************
 *	pthread Param	*
 ********************/

/********************************************|
|*				  Functions					*|
|********************************************/
//salam
//Hey
/***********************************************/
/********** HAJIAN - Servers_Start ***********/
/***********************************************/
int Servers_Start(int trace)
{
    int ret;

   //Initialize DLMS settings.
    svr_init(&lnWrapper.settings, 1, DLMS_INTERFACE_TYPE_WRAPPER, WRAPPER_BUFFER_SIZE, PDU_BUFFER_SIZE, ln47frameBuff, WRAPPER_BUFFER_SIZE, ln47pduBuff, PDU_BUFFER_SIZE);

    //We have several server that are using same objects. Just copy them.
    unsigned char KEK[16] = { 0x31,0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31 };
    BB_ATTACH(lnWrapper.settings.base.kek, KEK, sizeof(KEK));
    svr_InitObjects(&lnWrapper.settings);

    DLMS_INTERFACE_TYPE interfaceType;
    if(lnWrapper.settings.localPortSetup->defaultMode==0) interfaceType=DLMS_INTERFACE_TYPE_HDLC_WITH_MODE_E;
    else interfaceType = DLMS_INTERFACE_TYPE_HDLC;

    //Initialize DLMS settings.
    svr_init(&lniec.settings, 1, interfaceType, HDLC_BUFFER_SIZE, PDU_BUFFER_SIZE, lnframeBuff, HDLC_HEADER_SIZE + HDLC_BUFFER_SIZE, lnpduBuff, PDU_BUFFER_SIZE);

    svr_InitObjects(&lniec.settings);

    //Start server
    if ((ret = TCP_start(&lnWrapper)) != 0)
    {
        return ret;
    }

    //Start server
    // if ((ret = IEC_start(&lniec, "/dev/ttyUSB0")) != 0)
    // {
    //     return ret;
    // }


    if((ret = rs485_start(&rs485, RS485_SERIAL_FD)) != 0 )
    {
        return ret;
    }

    lnWrapper.trace =lniec.trace = rs485.trace = trace;

    return 0;
}


void Servers_Monitor (void)
{
	int ret;
    uint32_t lastMonitor = 0;
    while (1)
    {
        //Monitor values only once/second.
		lastMonitor = time_current();
		if ((ret = svr_monitorAll(&lnWrapper.settings)) != 0)
		{
			printf("lnWrapper monitor failed.\r\n");
		}
		if ((ret = svr_monitorAll(&lniec.settings)) != 0)
		{
			printf("lniec monitor failed.\r\n");
		}
		sleep(1);
    }
    con_close(&lnWrapper);
    con_close(&lniec);
}

/****************************************/
/********** Get time Function ***********/
/****************************************/
const char * get_time(void)
{
    static char time_buf[128];
    struct timeval  tv;
    time_t time;
    suseconds_t millitm;
    struct tm *ti;

    gettimeofday (&tv, NULL);

    time= tv.tv_sec;
    millitm = (tv.tv_usec + 500) / 1000;

    if (millitm == 1000) {
        ++time;
        millitm = 0;
    }

    ti = localtime(&time);
    sprintf(time_buf, "%02d-%02d_%02d:%02d:%02d:%03d", ti->tm_mon+1, ti->tm_mday, ti->tm_hour, ti->tm_min, ti->tm_sec, (int)millitm);
    return time_buf;
}

/**************************************/
/********** Report Function ***********/
/**************************************/
int report (REPORT_INTERFACE Interface, REPORT_MESSAGE Message, char *Information)
{
	int 		ret;
//	char		cmd[2048] = {0};
	char		log[4096] = {0};
	const char	*interface 	[] = {"RS485", "Server", "Client"}	;
	const char	*message 	[] = {"RX", "TX", "Connection"}		;
	char		*Time_Tag;

//	memset(cmd, 0, sizeof(cmd));

	Time_Tag = get_time();

	sprintf(log, "[%s] (%s): %s = %s", Time_Tag, interface[Interface], message[Message], Information);
	ret = printf("%s\n",log);

	//	sprintf(cmd, "echo \"%s\" >> /root/log.txt", log);
	//	ret = system("ls");

	FILE* f = fopen("log.txt", "a");
    if (f != NULL)
    {
    	fprintf(f, "%s\r\n", log);
    	fclose(f);
    }

	return ret;
}


//void LED_Init (void)
//{
//	system
//}


/************************************/
/********** Main Function ***********/
/************************************/
int main(int argc, char* argv[])
{
    strcpy(DATAFILE, argv[0]);

    char* p = strrchr(DATAFILE, '/');
    *p = '\0';
    strcpy(IMAGEFILE, DATAFILE);
    strcpy(TRACEFILE, DATAFILE);
    //Add empty file name.
    strcat(IMAGEFILE, "/empty.bin");
    strcat(DATAFILE, "/data.csv");
    strcat(TRACEFILE, "/trace.txt");
    FILE* f = fopen(TRACEFILE, "w");
    fclose(f);

    Servers_Start(GX_TRACE_LEVEL_INFO);

    LTE_Manager_Start();

    pthread_create(&SVR_Monitor, NULL, Servers_Monitor, NULL);

    while (1)
    {
    	sleep(1);
    }
    return 0;
}












