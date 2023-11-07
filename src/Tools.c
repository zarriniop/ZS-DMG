/*
 * Tools.c
 *
 *  Created on: Nov 7, 2023
 *      Author: mhn
 */

#include "Tools.h"
#include "exampleserver.h"
//#include "connection.h"
//#include "DLMS_Gateway.h"

unsigned char lnframeBuff	[HDLC_BUFFER_SIZE + HDLC_HEADER_SIZE]	;
unsigned char lnpduBuff		[PDU_BUFFER_SIZE]						;
unsigned char ln47frameBuff	[WRAPPER_BUFFER_SIZE]					;
unsigned char ln47pduBuff	[PDU_BUFFER_SIZE]						;
connection lnWrapper , lniec , rs485;
pthread_t SVR_Monitor;

int Servers_Start(int trace)
{
    int ret;

   //Initialize DLMS settings.
    svr_init(&lnWrapper.settings, 1, DLMS_INTERFACE_TYPE_WRAPPER, WRAPPER_BUFFER_SIZE, PDU_BUFFER_SIZE, ln47frameBuff, WRAPPER_BUFFER_SIZE, ln47pduBuff, PDU_BUFFER_SIZE);

    //We have several server that are using same objects. Just copy them.
    unsigned char KEK[16] = {0};
    memcpy(KEK, Settings.KEK, 16);
    BB_ATTACH(lnWrapper.settings.base.kek, KEK, sizeof(KEK));
    svr_InitObjects(&lnWrapper.settings);

    DLMS_INTERFACE_TYPE interfaceType;
    if(lnWrapper.settings.localPortSetup->defaultMode==0) interfaceType=DLMS_INTERFACE_TYPE_HDLC_WITH_MODE_E;
    else interfaceType = DLMS_INTERFACE_TYPE_HDLC;
//    interfaceType = DLMS_INTERFACE_TYPE_HDLC;


    //Initialize DLMS settings.
    svr_init(&lniec.settings, 1, interfaceType, HDLC_BUFFER_SIZE, PDU_BUFFER_SIZE, lnframeBuff, HDLC_HEADER_SIZE + HDLC_BUFFER_SIZE, lnpduBuff, PDU_BUFFER_SIZE);

    svr_InitObjects(&lniec.settings);

    //Start server
    if ((ret = TCP_start(&lnWrapper)) != 0)
    {
        return ret;
    }

    //Start server
     if ((ret = IEC_start(&lniec, OPTIC_SERIAL_FD)) != 0)
     {
         return ret;
     }


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


int report (REPORT_INTERFACE Interface, REPORT_MESSAGE Message, char *Information)
{
	int 		ret;
//	char		cmd[2048] = {0};
	char		log[4096] = {0};
	const char	*interface 	[] = {"RS485", "Server", "Client", "Optical", "Start App"}	;
	const char	*message 	[] = {"RX", "TX", "Connection", "Start"}		;
	char		*Time_Tag;

//	memset(cmd, 0, sizeof(cmd));

	Time_Tag = get_time();

	sprintf(log, "[%s] (%s): %s = %s", Time_Tag, interface[Interface], message[Message], Information);

	ret = printf("%s\n",log);

	//	sprintf(cmd, "echo \"%s\" >> /root/log.txt", log);
	//	ret = system("ls");

	FILE* f = fopen("/root/log.txt", "a");
    if (f != NULL)
    {
    	fprintf(f, "%s\r\n", log);
    	fclose(f);
    }

	return ret;
}


void LED_Init (void)
{
	int ret = 0;
	ret = system(ONESHOT_TRIG_LED_DATA)	;
	ret = system(ONESHOT_TRIG_LED_485)	;
	ret = system(PATTERN_TRIG_LED_NET)	;
	ret = system(LED_DATA_OFFDLY)		;
	ret = system(LED_485_OFFDLY)		;
	ret = system(LED_DATA_ONDLY)		;
	ret = system(LED_485_ONDLY)			;
}

void File_Init (char* argv[])
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
}







