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

const unsigned int  g_days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const unsigned int  j_days_in_month[12] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};

unsigned char lnframeBuff	[HDLC_BUFFER_SIZE + HDLC_HEADER_SIZE]	;
unsigned char lnpduBuff		[PDU_BUFFER_SIZE]						;
unsigned char ln47frameBuff	[WRAPPER_BUFFER_SIZE]					;
unsigned char ln47pduBuff	[PDU_BUFFER_SIZE]						;
connection lnWrapper , lniec , rs485;
pthread_t SVR_Monitor;

int Servers_Start(int trace)
{
    int ret;
    unsigned char KEK[16] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};

   //Initialize DLMS settings.
    svr_init(&lnWrapper.settings, 1, DLMS_INTERFACE_TYPE_WRAPPER, WRAPPER_BUFFER_SIZE, PDU_BUFFER_SIZE, ln47frameBuff, WRAPPER_BUFFER_SIZE, ln47pduBuff, PDU_BUFFER_SIZE);

    //We have several server that are using same objects. Just copy them.

    //memcpy(KEK, "00112233445566778899AABBCCDDEEFF\0", 33);
    BB_ATTACH(lnWrapper.settings.base.kek, KEK, sizeof(KEK));
    BB_ATTACH(lniec.settings.base.kek, KEK, sizeof(KEK));
    printf("\n --->>> KEK:");
    for(int i=0; i<16; i++)
    {
    	printf("%.2X  ", KEK[i]);
    }
    printf("\n");


    svr_InitObjects(&lnWrapper.settings);

    DLMS_INTERFACE_TYPE interfaceType;
    if(lnWrapper.settings.localPortSetup->defaultMode==0) interfaceType=DLMS_INTERFACE_TYPE_HDLC_WITH_MODE_E;
    else interfaceType = DLMS_INTERFACE_TYPE_HDLC;
//    interfaceType = DLMS_INTERFACE_TYPE_HDLC;

    //Initialize DLMS settings.
    svr_init(&lniec.settings, 1, interfaceType, HDLC_BUFFER_SIZE, PDU_BUFFER_SIZE, lnframeBuff, HDLC_HEADER_SIZE + HDLC_BUFFER_SIZE, lnpduBuff, PDU_BUFFER_SIZE);
    svr_InitObjects(&lniec.settings);

    BB_ATTACH(lnWrapper.settings.base.kek, KEK, sizeof(KEK));
    BB_ATTACH(lniec.settings.base.kek, KEK, sizeof(KEK));

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

    srv_Load(&lnWrapper.settings);

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


int report (REPORT_INTERFACE Interface, REPORT_MESSAGE Message, char *Information, ...)
{
	va_list args;
	va_start(args, Information);

	int 		ret;
	char		log[4096] = {0};
	char		info[4096] = {0};
	const char	*interface 	[] = {"RS485", "Server", "Client", "Optical", "Start App", "General", "Gateway"}	;
	const char	*message 	[] = {"RX", "TX", "Connection", "Start"}		;
	char		*Time_Tag;
	struct 		stat st;
	long int 	size;

	Time_Tag = get_time();
	vsprintf(info, Information, args);
	sprintf(log, "[%s] (%s): %s = %s", Time_Tag, interface[Interface], message[Message], info);
	va_end(args);

	ret = printf("%s\n",log);

	FILE* f = fopen("/root/log.txt", "a");
    if (f != NULL)
    {
    	stat("/root/log.txt", &st);
    	fprintf(f, "%s\r\n", log);

        fseek(f, 0L, SEEK_END);		//Measuring size and copying old logs
        size = ftell(f);
        if(size > MAX_FILE_SIZE)
        {
        	remove("log_old.txt");
        	rename("log.txt", "log_old.txt");
        }

    	fclose(f);
    }

	return ret;
}


void LED_Init (void)
{
	int ret = 0;

	ret = system(NONE_TRIG_LED_DATA)	;
	ret = system(NONE_TRIG_LED_485)		;
	ret = system(NONE_TRIG_LED_NET)		;

	ret = system(LED_DATA_ON_ALWAYS)	;
	ret = system(LED_485_ON_ALWAYS)		;
	ret = system(LED_NET_ON_ALWAYS)		;
	sleep(1);
	ret = system(LED_DATA_OFF_ALWAYS)	;
	ret = system(LED_485_OFF_ALWAYS)	;
	ret = system(LED_NET_OFF_ALWAYS)	;

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

void M2Sh (uint16_t *j_y, uint8_t *j_m, uint8_t *j_d, uint16_t  g_y, uint8_t  g_m, uint8_t  g_d){ // Miladi To shamsi
	 unsigned  long gy, gm, gd;
	 unsigned long jy, jm, jd;
	 unsigned  long g_day_no, j_day_no;
	 unsigned long j_np;
	 unsigned long i;

	 if(g_y<2000 || g_y>2100) g_y=2012;
	 if(g_m==0   || g_m>12)   g_m=1;
	 if(g_d==0   || g_d>31)   g_d=1;

	 gy = g_y-1600;
	 gm = g_m-1;
	 gd = g_d-1;

	 g_day_no = 365*gy+(gy+3)/4-(gy+99)/100+(gy+399)/400;
	 for (i=0;i<gm;++i)
			g_day_no += g_days_in_month[i];
	 if (gm>1 && ((gy%4==0 && gy%100!=0) || (gy%400==0)))
			/* leap and after Feb */
			++g_day_no;
	 g_day_no += gd;

	 j_day_no = g_day_no-79;

	 j_np = j_day_no / 12053;
	 j_day_no %= 12053;

	 jy = 979+33*j_np+4*(j_day_no/1461);
	 j_day_no %= 1461;

	 if (j_day_no >= 366)		{
			jy += (j_day_no-1)/365;
			j_day_no = (j_day_no-1)%365;
	 }
	 for (i = 0; i < 11 && j_day_no >= j_days_in_month[i]; ++i)
	 {
			j_day_no -= j_days_in_month[i];
	 }

	 jm = i+1;
	 jd = j_day_no+1;
	 *j_y = jy;
	 *j_m = jm;
	 *j_d = jd;
}


void SH2M (uint16_t *My, uint8_t *Mm, uint8_t *Md,long jy, long jm, long jd){ // Miladi To shamsi
	long out[4];
  jy += 1595;
  out[2] = -355668 + (365 * jy) + (((int)(jy / 33)) * 8) + ((int)(((jy % 33) + 3) / 4)) + jd + ((jm < 7) ? (jm - 1) * 31 : ((jm - 7) * 30) + 186);
  out[0] = 400 * ((int)(out[2] / 146097));
  out[2] %= 146097;
  if (out[2] > 36524) {
    out[0] += 100 * ((int)(--out[2] / 36524));
    out[2] %= 36524;
    if (out[2] >= 365) out[2]++;
  }
  out[0] += 4 * ((int)(out[2] / 1461));
  out[2] %= 1461;
  if (out[2] > 365) {
    out[0] += (int)((out[2] - 1) / 365);
    out[2] = (out[2] - 1) % 365;
  }
  long sal_a[13] = {0, 31, ((out[0]%4 == 0 && out[0]%100 != 0) || (out[0]%400 == 0))?29:28 , 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  for (out[2]++, out[1] = 0; out[1] < 13 && out[2] > sal_a[out[1]]; out[1]++) out[2] -= sal_a[out[1]];
  *My =out[0];
	*Mm = out[1];
  *Md=out[2];
}

void exec(const char* cmd,char *resultt,uint32_t time) {
    uint16_t cnt=0;
    clock_t clk,t=0;
    char buffer[1000],count;

	FILE* pipe = popen(cmd, "r");
	if (!pipe) printf("popen() failed!\n");

	memset(buffer,0,1000);
	memset(resultt,0,sizeof(resultt));
	t=time*1000;
	clk=clock();

	while(clock()-clk<t)
	{
		if (fgets(buffer, sizeof(buffer), pipe) != NULL)
		{

			memcpy(resultt+cnt,buffer,strlen(buffer));
			cnt+=strlen(buffer);
			count=0;
		}
		else
		{
			count++;
			usleep(100);
			//if(count>20 && cnt>0) break;
		}

	}
	pclose(pipe);
}


void GPRS_kat_gxData_Get_Value (GPRS_KAT_GXDATA_STR* GPRS_Kat_gxData, gxData* gprs_keep_alive_gurux)
{
	int	pointer			= 0;
	int cntr			= 0;
	char buffer[10] 	= {0};
	char data_cpy[50] 	= {0};
	gxByteBuffer value_string;

	memset(GPRS_Kat_gxData, 0, sizeof(GPRS_KAT_GXDATA_STR));
	memset(&value_string, 0, sizeof(value_string));

	var_toString(&gprs_keep_alive_gurux->value, &value_string);
	strcpy(&data_cpy, value_string.data);

	for (int i=1; data_cpy[i] != ','; i++)
	{
		buffer[cntr] = data_cpy[i];
		cntr ++;
		pointer = i;
	}
	pointer += 2;

	if(!strcmp(&buffer ,"True"))
		GPRS_Kat_gxData->switch_enable = true;
	else
		GPRS_Kat_gxData->switch_enable = false;

	cntr = 0;
	memset(buffer, 0, sizeof(buffer));

	for (int i=pointer; data_cpy[i] != ','; i++)
	{
		buffer[cntr] = data_cpy[i];
		cntr ++;
		pointer = i;
	}
	pointer += 2;
	GPRS_Kat_gxData->ideal_time = atoi(&buffer);

	cntr = 0;
	memset(buffer, 0, sizeof(buffer));
	for (int i=pointer; data_cpy[i] != ']'; i++)
	{
		buffer[cntr] = data_cpy[i];
		cntr ++;
	}
	GPRS_Kat_gxData->delay_retry_interval_value = atoi(&buffer);
}


uint8_t Set_Socket_KAT_Option (int socket_fd, GPRS_KAT_GXDATA_STR* gprs_kat_option)
{
	int flags 	= 0;
	int res		= 0;
	uint8_t ret = 0;

	if(gprs_kat_option->switch_enable)
	{
		if(GPRS_KAT_IDEAL_CND)
		{
			flags = gprs_kat_option->ideal_time;
			res=setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPIDLE, &flags, sizeof(flags));
			if(res < 0)
			{
				printf("---------------- ERROR - TCP_KEEPIDLE\n");
			}
			else
				ret = (ret | 0b1);
		}


		flags = 1;
		res=setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &flags, sizeof(flags));
		if(res < 0)
		{
			printf("---------------- ERROR - TCP_KEEPCNT\n");
		}
		else
			ret = (ret | 0b10);


		if(GPRS_KAT_INTVL_CND)
		{
			flags = gprs_kat_option->delay_retry_interval_value;
			res=setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, &flags, sizeof(flags));
			if(res < 0)
			{
				printf("---------------- ERROR - TCP_KEEPINTVL\n");
			}
			else
				ret = (ret | 0b100);
		}


		flags = 1;
		res=setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, & flags, sizeof(flags));
		if(res < 0)
		{
			printf("---------------- ERROR - KAT ENABLING\n");
		}
		else
			ret = (ret | 0b1000);
	}
	else
	{
		ret = 0;
		flags = 0;
		res=setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, & flags, sizeof(flags));
	}

	return ret;
}




