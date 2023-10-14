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

/********************************************|
|*			Variables & Definitions			*|
|********************************************/

uint32_t ret=0;

/****************
 *	APN Param	*
 ****************/
APN_PARAM_STRUCT_TYPEDEF 	APN_Param_Struct		;

/****************
 *	WAN Param	*
 ****************/
WAN_PARAM_STRUCT_TYPEDEF 	WAN_Param_Struct		;
nw_status_cb 				nw_cb					;
QL_DSI_AUTH_PREF_T 			auth					;
QL_NW_REG_STATUS_INFO_T 	ref_status				;
ql_data_call_info 			payload					;

/****************
 *	TCP Param	*
 ****************/
TCP_PARAM_STRUCT_TYPEDEF 	TCP_Param_Struct		;

/****************
 *	SMS Param	*
 ****************/
SMS_PARAM_STRUCT_TYPEDEF 	SMS_Param_Struct		;
ql_sms_mem_info_t 			mem_info				;
size_t 						location				;
recvmessage					SMS_payload				;

/********************
 *	Settings Param	*
 ********************/
SETTINGS_STRUCT_TYPEDEF 	Settings_Struct			;

/****************
 *	UART Param	*
 ****************/
UART_STRUCT_TYPEDEF 		UART_Struct				;
uint32_t					UART_ret				;
ST_UARTDCB					dcb						;

/****************
 *	I2C Param	*
 ****************/
I2C_STUCT_TYPEDEF			I2C_Struct				;
DS1307_I2C_STRUCT_TYPEDEF	DS1307_I2C_Struct		;

/****************
 *	DLMS Param	*
 ****************/
DLMS_STRUCT_TYPEDEF			DLMS_struct				;
uint8_t 					Gate_Meter_Prtcl[500]	;
uint16_t 					Gate_Meter_size			;
uint8_t 					Gate_HES_Prtcl[500]		;
uint16_t 					Gate_HES_size			;


/****************
 *	FLAG Param	*
 ****************/
FLAGS_STRUCT_TYPEDEF 		Flags_Struct			;


/********************
 *	HAJIAN Param	*
 ********************/
unsigned char lnframeBuff	[HDLC_BUFFER_SIZE + HDLC_HEADER_SIZE]	;
unsigned char lnpduBuff		[PDU_BUFFER_SIZE]						;
unsigned char ln47frameBuff	[WRAPPER_BUFFER_SIZE]					;
unsigned char ln47pduBuff	[PDU_BUFFER_SIZE]						;


/********************
 *	pthread Param	*
 ********************/
pthread_t Wan_Connection_pthread_var				;
pthread_t TCP_Connection_pthread_var				;
pthread_t TCP_Client_Check_pthread_var				;
pthread_t TCP_Rece_Data_pthread_var					;
pthread_t UART_Main_Read_Data_pthread_var			;
pthread_t UART_Main_Write_Data_pthread_var			;


/********************************************|
|*				  Functions					*|
|********************************************/

/************************************/
/***** Error Display Function *******/
/************************************/
void Err_Disp (char section[250], uint32_t ret)
{
	printf("<= ! Error ! - Section:%s - Ret:%d =>\n", section, ret);
}

/************************************/
/*** Network Initialize Function ****/
/************************************/
void NW_Init (void)
{
	ret = ql_nw_release();
	if(ret!=0) Err_Disp("ql_nw_release", ret);

	ret = ql_nw_init();
	if(ret!=0) Err_Disp("ql_nw_init", ret);
}

/************************************/
/**** WAN Initializing Function *****/
/************************************/
void WAN_Init (void)
{
	memset(&WAN_Param_Struct, 0, sizeof(WAN_PARAM_STRUCT_TYPEDEF));
	WAN_Param_Struct.op = START_A_DATA_CALL;

	ret = ql_wan_release()	;
	if(ret!=0) Err_Disp("ql_wan_release", ret);

	ret = ql_wan_init()		;
	if(ret!=0) Err_Disp("ql_wan_init", ret);

	printf("<= WAN Initialization: DONE! =>\n");
}

/************************************/
/***** WAN Connection Function ******/
/************************************/
void WAN_Connection (void)
{
	strcpy(Settings_Struct.APN, "mtnirancell\0");
//	printf("<= WAN Connection - APN:%s =>\n", Settings_Struct.APN);
	ret = ql_wan_setapn(APN_Param_Struct.profile_idx, APN_Param_Struct.ip_type, &Settings_Struct.APN, &APN_Param_Struct.userName, &APN_Param_Struct.password, auth);
	if(ret!=0) Err_Disp("ql_wan_setapn", ret);

	int 	ip_type_get 	= 0	;
    char 	apn_get[128]	={0};
    char 	userName_get[64]={0};
    char 	password_get[64]={0};

    ret = ql_wan_getapn(APN_Param_Struct.profile_idx, &ip_type_get, apn_get, sizeof(apn_get), userName_get, sizeof(userName_get), password_get, sizeof(password_get));
	if(ret!=0) Err_Disp("ql_wan_getapn", ret);
	else
	{
		printf("<= APN SET:%s =>\n", apn_get);
	}


	ret = ql_wan_start(APN_Param_Struct.profile_idx, WAN_Param_Struct.op, nw_cb);
	if(ret!=0) Err_Disp("ql_wan_start", ret);

	while(1)
	{
		ret = ql_get_data_call_info(APN_Param_Struct.profile_idx, &payload);

		if (ret == 0)
		{
			if (payload.v4.state == 1)
			{
				if(Flags_Struct.WAN_Struct.WAN_IPv4_Started == DOWN)
				{
					Flags_Struct.WAN_Struct.WAN_IPv4_Started 	= UP;
					Flags_Struct.TCP_Struct.TCP_Ready_To_Start 	= UP;
				}

//				printf("<= IP:%s =>\n", payload.v4.addr.ip);

//				printf("<= data_call_info v4: {\n    profile_idx:%d,\n    ip_type:%d,\n    state:%d,\n    ip:%s,\n    name:%s,\n    gateway:%s,\n    pri_dns:%s,\n    sec_dns:%s\n} =>\n",
//					payload.profile_idx, payload.ip_type, payload.v4.state, payload.v4.addr.ip, payload.v4.addr.name, payload.v4.addr.gateway,
//					payload.v4.addr.pri_dns, payload.v4.addr.sec_dns);

//				#define QUEC_AT_PORT        "/dev/smd1"
//				int smd_fd = 0;
//				smd_fd = open(QUEC_AT_PORT, O_RDWR | O_NONBLOCK | O_NOCTTY);
//				printf("<= open(\"%s\")=%d =>\n", QUEC_AT_PORT, smd_fd);
			}
			else
			{
				Flags_Struct.WAN_Struct.WAN_IPv4_Started 	= DOWN;
				Flags_Struct.TCP_Struct.TCP_Ready_To_Start 	= DOWN;

//				ret = ql_wan_release()	;
//				if(ret!=0)
//					Err_Disp("ql_wan_release", ret);
//				else
//					printf("<= WAN RELEASED! =>\n");
//
//				ret = ql_wan_init()		;
//				if(ret!=0)
//					Err_Disp("ql_wan_init", ret);
//				else
//					printf("<= WAN INITIALIZED! =>\n");

				ret = ql_wan_start(APN_Param_Struct.profile_idx, WAN_Param_Struct.op, nw_cb);
				if(ret!=0)
					Err_Disp("ql_wan_start", ret);
				else
					printf("<= WAN STARTED! nw_cb=%d =>\n", nw_cb);
			}
		}
		else
		{
			Err_Disp("ql_get_data_call_info", ret);
		}
		sleep(2);
	}
}

/******************************/
/****** SMS Processing ********/
/******************************/
void SMS_Processing(char *SMS_Text, int SMS_Len, char *SMS_Response)
{
	printf("<= SMS Processing begins ... =>\n");

	char SMS_Text_Copy[280];
	char tmp[30];

	for(int i=0; i<SMS_Len; i++)
	{
		SMS_Text[i] = toupper (SMS_Text[i]);
	}

	printf("<= Text:%s\n   SMS_Len:%d =>\n", SMS_Text, SMS_Len);

	memset(SMS_Text_Copy, 	0, 			sizeof(SMS_Text_Copy))	;
	memset(SMS_Response,	0, 			sizeof(SMS_Response))	;
	memset(tmp,				0, 			sizeof(tmp))			;
	memcpy(SMS_Text_Copy,	SMS_Text, 	sizeof(SMS_Text))		;

	/********************************
	 *	Looking For Phone number 1	*
	 ********************************/
	for (int i=0; i<SMS_Len; i++)
	{
		if(strncmp(SMS_Text+i,"TEL1:",5)==0)
		{
			UINT16 v=i+5, n=0;
			for(n=v ; n < v+21 ; n++)
			{
				if(SMS_Text[n]!='+' && (SMS_Text[n]>'9' || SMS_Text[n]<'0') ) break; 		//check for the end of numbers
			}

			memset(tmp,0,sizeof(tmp));

			if(n-v > 4)							//if tel number length > 4
			{
				char str[250];

				memset(str,						0,	sizeof(str))					;
				memset(Settings_Struct.TEL1, 	0,	sizeof(Settings_Struct.TEL1))	;

				memcpy(tmp, SMS_Text+v, n-v);

				strcpy(Settings_Struct.TEL1, tmp);

				sprintf(str,"TEL1:%s OK\n",Settings_Struct.TEL1);
				strcat(SMS_Response,str);

//				TEL1_SETTING=1;

				printf("<= NEW TEL1: %s =>\n", Settings_Struct.TEL1);
			}
			else if(SMS_Text[v]=='N')
			{
				memset(Settings_Struct.TEL1, 0, sizeof(Settings_Struct.TEL1));

				strcpy(Settings_Struct.TEL1, "N");

				strcat(SMS_Response,"TEL1:DELETE\n");

//				TEL1_SETTING=0;

				printf("<= NEW TEL1: %s =>\n", Settings_Struct.TEL1);
			}
			else
			{
				strcat(SMS_Response,"TEL1:ERROR\n");
			}
			break;
		}
	}

	/********************************
	 *	Looking For Phone number 2	*
	 ********************************/
	for (int i=0; i<SMS_Len; i++)
	{
		if(strncmp(SMS_Text+i,"TEL2:",5)==0)
		{
			UINT16 v=i+5, n=0;
			for(n=v ; n < v+21 ; n++)
			{
				if(SMS_Text[n]!='+' && (SMS_Text[n]>'9' || SMS_Text[n]<'0') ) break; 		//check for the end of numbers
			}

			memset(tmp,0,sizeof(tmp));

			if(n-v > 4)							//if tel number length > 4
			{
				char str[250];

				memset(str,						0,	sizeof(str))					;
				memset(Settings_Struct.TEL2, 	0,	sizeof(Settings_Struct.TEL2))	;

				memcpy(tmp, SMS_Text+v, n-v);

				strcpy(Settings_Struct.TEL2, tmp);

				sprintf(str,"TEL2:%s OK\n",Settings_Struct.TEL2);
				strcat(SMS_Response,str);

//				TEL2_SETTING=1;

				printf("<= NEW TEL2: %s =>\n", Settings_Struct.TEL2);
			}
			else if(SMS_Text[v]=='N')
			{
				memset(Settings_Struct.TEL2, 0, sizeof(Settings_Struct.TEL2));

				strcpy(Settings_Struct.TEL2, "N");

				strcat(SMS_Response,"TEL2:DELETE\n");

//				TEL2_SETTING=0;

				printf("<= NEW TEL2: %s =>\n", Settings_Struct.TEL2);
			}
			else
			{
				strcat(SMS_Response,"TEL2:ERROR\n");
			}
			break;
		}
	}

	/********************************
	 *	Looking For Phone number 3	*
	 ********************************/
	for (int i=0; i<SMS_Len; i++)
	{
		if(strncmp(SMS_Text+i,"TEL3:",5)==0)
		{
			UINT16 v=i+5, n=0;
			for(n=v ; n < v+21 ; n++)
			{
				if(SMS_Text[n]!='+' && (SMS_Text[n]>'9' || SMS_Text[n]<'0') ) break; 		//check for the end of numbers
			}

			memset(tmp,0,sizeof(tmp));

			if(n-v > 4)							//if tel number length > 4
			{
				char str[250];

				memset(str,						0,	sizeof(str))					;
				memset(Settings_Struct.TEL3, 	0,	sizeof(Settings_Struct.TEL3))	;

				memcpy(tmp, SMS_Text+v, n-v);

				strcpy(Settings_Struct.TEL3, tmp);

				sprintf(str,"TEL3:%s OK\n",Settings_Struct.TEL3);
				strcat(SMS_Response,str);

//				TEL3_SETTING=1;

				printf("<= NEW TEL3: %s =>\n", Settings_Struct.TEL3);
			}
			else if(SMS_Text[v]=='N')
			{
				memset(Settings_Struct.TEL3, 0, sizeof(Settings_Struct.TEL3));

				strcpy(Settings_Struct.TEL3, "N");

				strcat(SMS_Response,"TEL3:DELETE\n");

//				TEL3_SETTING=0;

				printf("<= NEW TEL3: %s =>\n", Settings_Struct.TEL3);
			}
			else
			{
				strcat(SMS_Response,"TEL3:ERROR\n");
			}
			break;
		}
	}

	/********************************
	 *	Looking For Phone number 4	*
	 ********************************/
	for (int i=0; i<SMS_Len; i++)
	{
		if(strncmp(SMS_Text+i,"TEL4:",5)==0)
		{
			UINT16 v=i+5, n=0;
			for(n=v ; n < v+21 ; n++)
			{
				if(SMS_Text[n]!='+' && (SMS_Text[n]>'9' || SMS_Text[n]<'0') ) break; 		//check for the end of numbers
			}

			memset(tmp,0,sizeof(tmp));

			if(n-v > 4)							//if tel number length > 4
			{
				char str[250];

				memset(str,						0,	sizeof(str))					;
				memset(Settings_Struct.TEL4, 	0,	sizeof(Settings_Struct.TEL4))	;

				memcpy(tmp, SMS_Text+v, n-v);

				strcpy(Settings_Struct.TEL4, tmp);

				sprintf(str,"TEL4:%s OK\n",Settings_Struct.TEL4);
				strcat(SMS_Response,str);

//				TEL4_SETTING=1;

				printf("<= NEW TEL4: %s =>\n", Settings_Struct.TEL4);
			}
			else if(SMS_Text[v]=='N')
			{
				memset(Settings_Struct.TEL4, 0, sizeof(Settings_Struct.TEL4));

				strcpy(Settings_Struct.TEL4, "N");

				strcat(SMS_Response,"TEL4:DELETE\n");

//				TEL4_SETTING=0;

				printf("<= NEW TEL4: %s =>\n", Settings_Struct.TEL4);
			}
			else
			{
				strcat(SMS_Response,"TEL4:ERROR\n");
			}
			break;
		}
	}

	/********************************
	 *	Looking For Phone number 5	*
	 ********************************/
	for (int i=0; i<SMS_Len; i++)
	{
		if(strncmp(SMS_Text+i,"TEL5:",5)==0)
		{
			UINT16 v=i+5, n=0;
			for(n=v ; n < v+21 ; n++)
			{
				if(SMS_Text[n]!='+' && (SMS_Text[n]>'9' || SMS_Text[n]<'0') ) break; 		//check for the end of numbers
			}

			memset(tmp,0,sizeof(tmp));

			if(n-v > 4)							//if tel number length > 4
			{
				char str[250];

				memset(str,						0,	sizeof(str))					;
				memset(Settings_Struct.TEL5, 	0,	sizeof(Settings_Struct.TEL5))	;

				memcpy(tmp, SMS_Text+v, n-v);

				strcpy(Settings_Struct.TEL5, tmp);

				sprintf(str,"TEL5:%s OK\n",Settings_Struct.TEL5);
				strcat(SMS_Response,str);

//				TEL5_SETTING=1;

				printf("<= NEW TEL5: %s =>\n", Settings_Struct.TEL5);
			}
			else if(SMS_Text[v]=='N')
			{
				memset(Settings_Struct.TEL5, 0, sizeof(Settings_Struct.TEL5));

				strcpy(Settings_Struct.TEL5, "N");

				strcat(SMS_Response,"TEL5:DELETE\n");

//				TEL5_SETTING=0;

				printf("<= NEW TEL5: %s =>\n", Settings_Struct.TEL5);
			}
			else
			{
				strcat(SMS_Response,"TEL5:ERROR\n");
			}
			break;
		}
	}

	/****************************
	 *	Looking For Server IP	*
	 ****************************/
	for(int i=0 ; i<SMS_Len ; i++)
	{
		if(strncmp(SMS_Text+i,"S-IP:",5)==0)		//Search baraye IP address
		{
			int n, v=i+5;
			char ip_buffer[30];
			struct in_addr addr;

			for(n=v; n<=15+v; n++)
			{
				if(SMS_Text[n]==' ' || SMS_Text[n]=='\n')
				{
					break;
				}
			}

			memset(tmp, 0, sizeof(tmp));
			memcpy(tmp, SMS_Text+v, n-v);

			memset(ip_buffer, 0, sizeof(ip_buffer));
			strcpy(ip_buffer,tmp);

			int ret_ip = inet_pton(AF_INET, tmp, &(addr.s_addr));
			printf("<= ret_ip , IP:%lld =>\n", addr.s_addr);

			if((strcmp(tmp,"N"))==0)
			{
				ret_ip=1;
			}

			if(ret_ip!=1)
			{
				strcat(SMS_Response,"S-IP:ERROR\n");
			}
			else
			{
				char str[250];

				memset(Settings_Struct.S_IP, 0, sizeof(Settings_Struct.S_IP));
				strcpy(Settings_Struct.S_IP, tmp);

				memset(str, 0, sizeof(str));
				sprintf(str,"S-IP:%s OK\n",Settings_Struct.S_IP);

				strcat(SMS_Response,str);

//				Flags_Struct.Reset_Struct.WAN = UP;

				printf("<= NEW SERVER IP: %s =>\n", Settings_Struct.S_IP);
			}

			break;
		}
	}

	/****************************
	 *	Looking For Server PORT	*
	 ****************************/
	for(int i=0 ; i<SMS_Len ; i++)
	{
		if(strncmp(SMS_Text+i,"S-PORT:",7)==0)			//Search baraye PORT
		{
			int n, v=i+7;
			for(n=v; n<=15+v; n++)
			{
				if(SMS_Text[n]>'9' || SMS_Text[n]<'0')
				{
					break;
				}
			}

			if(n!=v)
			{
				int32_t port_buffer = 0;

				memset(tmp, 0, sizeof(tmp));
				memcpy(tmp, SMS_Text+v, n-v);

				sscanf(tmp, "%ld", &port_buffer);

				if(port_buffer<0 || port_buffer>65535)
				{
					printf("<= PORT is out of range. =>\n");
					strcat(SMS_Response,"S-PORT:ERROR\n");
				}
				else
				{
					char str[250];

					Settings_Struct.S_PORT=port_buffer;

					memset(str,0,sizeof(str));
					sprintf(str,"S-PORT:%ld OK\n", Settings_Struct.S_PORT);
					strcat(SMS_Response, str);

//					Flags_Struct.Reset_Struct.WAN = UP;
				}

				printf("<= The Current port: %ld =>\r\n", Settings_Struct.S_PORT);

				break;
			}
			else
			{
				printf("<= S-PORT:INVALID VALUE =>\r\n");
				strcat(SMS_Response,"S-PORT:SET IN RANGE NUMBER\n");
			}

		}
	}

	/****************************
	 *	Looking For Phone APN	*
	 ****************************/
	for(int i=0 ; i<SMS_Len ; i++)
	{
		if(strncmp(SMS_Text+i,"APN-NAME:",9)==0)		//Search baraye APN address
		{
			int n, v=i+9;
			for(n=v; n<=15+v; n++)
			{
				if(SMS_Text[n]==' ' || SMS_Text[n]=='\n')		//peyda kardane entehaye apn address dar matn sms
				{
					break;
				}
			}

			if(n-v<=1)
			{
				strcat(SMS_Response,"APN-NAME:ERROR\n");
			}
			else
			{
				char str[250];

				memset(str,	0,	sizeof(str));
				memset(tmp,	0,	sizeof(tmp));
				memset(Settings_Struct.APN, 0, sizeof(Settings_Struct.APN));

				memcpy(tmp, SMS_Text+v, n-v);

				strcpy(Settings_Struct.APN, tmp);

				sprintf(str,"APN-NAME:%s OK\n",Settings_Struct.APN);
				strcat(SMS_Response,str);
				printf("<= NEW APN-NAME: %s =>\n", Settings_Struct.APN);

				Flags_Struct.Reset_Struct.WAN = UP;
			}

			break;
		}
	}

	/************************************
	 *	Looking For Phone APN User Name	*
	 ************************************/
	for(int i=0 ; i<SMS_Len ; i++)
	{
		if(strncmp(SMS_Text+i,"APN-USER:",9)==0)		//Search baraye APN User Name
		{
			int n, v=i+9;
			for(n=v; n<=20+v; n++)
			{
				if(SMS_Text[n]==' ' || SMS_Text[n]=='\n')		//peyda kardane entehaye apn address dar matn sms
				{
					break;
				}
			}

			if(n == v)		//Set nothing (Text-> APN-UN:)
			{
				strcat(SMS_Response,"APN-USER:ERROR\n");
				printf("<= APN USER NAME IS WRONG =>\n");
			}
			else
			{
				char str[250];

				memset(str,	0,	sizeof(str));
				memset(tmp,	0,	sizeof(tmp));
				memset(Settings_Struct.APN_USER, 0, sizeof(Settings_Struct.APN_USER));

				memcpy(tmp, SMS_Text+v, n-v);

				strcpy(Settings_Struct.APN_USER, tmp);

				sprintf(str,"APN-USER:%s OK\n",Settings_Struct.APN_USER);
				strcat(SMS_Response,str);
				printf("<= NEW APN-USER:%s =>\n", Settings_Struct.APN_USER);

//				Flags_Struct.Reset_Struct.WAN = UP;
			}

			break;
		}
	}

	/************************************
	 *	Looking For Phone APN Password	*
	 ************************************/
	for(int i=0 ; i<SMS_Len ; i++)
	{
		if(strncmp(SMS_Text+i,"APN-PASS:",9)==0)		//Search baraye APN PASSWORD
		{
			int n, v=i+9;
			for(n=v; n<=20+v; n++)
			{
				if(SMS_Text[n]==' ' || SMS_Text[n]=='\n')		//peyda kardane entehaye apn address dar matn sms
				{
					break;
				}
			}

			if(n == v)		//Set nothing (Text-> APN-UN:)
			{
				strcat(SMS_Response,"APN-PASS:ERROR\n");
				printf("<= APN PASSWORD IS WRONG =>\n");
			}
			else
			{
				char str[250];

				memset(str,	0,	sizeof(str));
				memset(tmp,	0,	sizeof(tmp));
				memset(Settings_Struct.APN_PASS, 0, sizeof(Settings_Struct.APN_PASS));

				memcpy(tmp, SMS_Text+v, n-v);

				strcpy(Settings_Struct.APN_PASS, tmp);

				sprintf(str,"APN-PASS:%s OK\n",Settings_Struct.APN_PASS);
				strcat(SMS_Response,str);
				printf("<= NEW APN-PASS:%s =>\n", Settings_Struct.APN_PASS);

//				Flags_Struct.Reset_Struct.WAN = UP;
			}

			break;
		}
	}


	printf("<= ... SMS Processing ends! =>\n");
}

/************************************/
/***** Deleting All SMS Function ****/
/************************************/
void SMS_All_Del(void)
{
//	for(int i=0; i<=200; i++)
//	{
//		ret = ql_sms_delete_msg(i);
//	}
}

/**********************************/
/***** Reset Process Function *****/
/**********************************/
void Reset_Processing(void)
{
	int result = 0;

	if(Flags_Struct.Reset_Struct.WAN) 			//Flags_Struct.Reset_Struct.WAN == UP
	{
		sleep(1);
		printf("<= Flags_Struct.Reset_Struct.WAN = %d (1:UP) =>\n", Flags_Struct.Reset_Struct.WAN);
		result = pthread_cancel(Wan_Connection_pthread_var);
		if(result!=0) Err_Disp("pthread_cancel - Wan_Connection_pthread_var", result);
		else
		{
			printf("<= pthread canceled. =>\n");
			sleep(1);

			result = ql_wan_stop(APN_Param_Struct.profile_idx);
			if(result!=0) Err_Disp("ql_wan_stop", result);
			else printf("<= WAN STOPPED. =>\n");

			result = ql_wan_release()	;
			if(ret!=0) Err_Disp("ql_wan_release", result);
			else printf("<= WAN RELEASED. =>\n");

//			result = ql_nw_release();
//			if(ret!=0) Err_Disp("ql_nw_release", result);
//			else printf("<= NW RELEASED. =>\n");

//			result = ql_nw_init();
//			if(result!=0) Err_Disp("ql_nw_init", ret);
//			else printf("<= NW INITIALIZED. =>\n");

			ret = ql_wan_init();
			if(ret!=0) Err_Disp("ql_wan_init", ret);
			else printf("<= WAN INITIALIZED. =>\n");

			result = pthread_create(&Wan_Connection_pthread_var, NULL, WAN_Connection, NULL);
			if(result!=0) Err_Disp("pthread_create - Wan_Connection_pthread_var", result);
			else
			{
				Flags_Struct.Reset_Struct.WAN = DOWN;
				printf("<= Flags_Struct.Reset_Struct.WAN = %d (1:UP) =>\n", Flags_Struct.Reset_Struct.WAN);
			}
		}
	}
}

/***********************************************/
/****** Checking for Valid Phone Numbers *******/
/***********************************************/
uint8_t Check_Valid_Phone_Numbers(char *Phone_Number)
{
	printf("<= Phone number:%s , Set Number:%s =>\n", Phone_Number+3, Settings_Struct.TEL1+3);
	if(strcmp(Settings_Struct.TEL1,"N")==0 && strcmp(Settings_Struct.TEL2,"N")==0 && strcmp(Settings_Struct.TEL3,"N")==0 && strcmp(Settings_Struct.TEL4,"N")==0 && strcmp(Settings_Struct.TEL5,"N")==0)
	{
		printf("<= Phone number has been set with Nothing =>\n");
		return 0;
	}
	else
	{
		uint8_t n=strlen(Phone_Number)-3;
		n=strlen(Settings_Struct.TEL1)-n;
		if(strcmp(Settings_Struct.TEL1+n, Phone_Number+3) == 0)
		{
			printf("<= Phone number has been set with TEL1 =>\n");
			return 1;
		}

		n=strlen(Phone_Number)-3;
		n=strlen(Settings_Struct.TEL2)-n;
		if(strcmp(Settings_Struct.TEL2+n, Phone_Number+3) == 0)
		{
			printf("<= Phone number has been set with TEL2 =>\n");
			return 2;
		}

		n=strlen(Phone_Number)-3;
		n=strlen(Settings_Struct.TEL3)-n;
		if(strcmp(Settings_Struct.TEL3+n, Phone_Number+3) == 0)
		{
			printf("<= Phone number has been set with TEL3 =>\n");
			return 3;
		}

		n=strlen(Phone_Number)-3;
		n=strlen(Settings_Struct.TEL4)-n;
		if(strcmp(Settings_Struct.TEL4+n, Phone_Number+3) == 0)
		{
			printf("<= Phone number has been set with TEL4 =>\n");
			return 4;
		}

		n=strlen(Phone_Number)-3;
		n=strlen(Settings_Struct.TEL5)-n;
		if(strcmp(Settings_Struct.TEL5+n, Phone_Number+3) == 0)
		{
			printf("<= Phone number has been set with TEL5 =>\n");
			return 5;
		}

		printf("<= Phone number has not been set =>\n");
		return 10;
	}
}

/***********************************************/
/****** Receiving SMS Function Callback ********/
/***********************************************/
void SMS_Receive_handlerPtr(QL_SMS_NFY_MSG_ID msg_id, void *pv_data, int pv_data_len, void *contextPtr)		//Depend on ql_sms_add_event_handler - Callback function
{
	int t_msg_id = 0;
	t_msg_id = msg_id;

	QL_SMS_STATUS_INFO *sms_body = (QL_SMS_STATUS_INFO *)pv_data;

	if (contextPtr)
		printf("<-- [%s] -->\n", contextPtr);

	switch (msg_id)
	{
		case QL_SMS_RECV_EVENT:
			{
//				printf("<=\n  Recv message\n\tTEL:%s\n\tmessage lenght:%d\n\tmessage context:\n\t\t%s\n\ttime:%s\n=>\n",sms_body->number,sms_body->body_len,sms_body->body,sms_body->time);
				printf("<=\n  PDU SMS\n\tTEL:%s\n\tmessage length:%d\n\tmessage context:\n\t\t%s\n\t\t%s\n\ttime:%s\n\tsms_mode:%d (0:GSM , 1:UCS)\n=>\n",
						sms_body->number,
						sms_body->body_len,
						sms_body->body,
						sms_body->pdu_str,
						sms_body->time,
						sms_body->code_mode);

				ret = Check_Valid_Phone_Numbers(&sms_body->number);
				if(ret != 10)
				{
					SMS_Processing(&sms_body->body, sms_body->body_len, &SMS_Param_Struct.SMS_response);

					if(strlen(SMS_Param_Struct.SMS_response)>0)
						SMS_Send(&sms_body->number, &SMS_Param_Struct.SMS_response, TEXT_SMS_MODE, GSM_7BIT_SMS_ENCOD_MODE);

					SMS_All_Del();
					Reset_Processing();
				}
			}
			break;

		default:
			break;
	}
}

/************************************/
/***** Initializing SMS Function ****/
/************************************/
int SMS_Init (void)
{
	ret = ql_sms_release();
	if(ret) Err_Disp("ql_sms_release", ret);

	ret = ql_sms_init();
	if(ret) Err_Disp("ql_sms_init", ret);

	if(ret) Err_Disp("ql_set_sms_msg_mode", ret);
	else
	{
		int messagemode = 1;
		ret = ql_get_sms_msg_mode(&messagemode);
		printf("<= ret=%d , messagemode=%d (0:PDU , 1:TXT)=>\n", ret, messagemode);
	}

//	ql_set_sms_recive_dealmode(3);
//	if(ret) Err_Disp("ql_sms_add_event_handler", ret);

	ret = ql_sms_add_event_handler(SMS_Receive_handlerPtr, NULL);
	if(ret) Err_Disp("ql_sms_add_event_handler", ret);

	SMS_Param_Struct.SMS_type = GSM_7BIT_SMS_ENCOD_MODE;

	SMS_All_Del();

	ret = ql_sms_get_sms_pref_storage(&mem_info);
	if(ret)	Err_Disp("ql_sms_get_sms_pref_storage", ret);
//	else
//	{
//		printf("<=\n  Storage:%s\tNum. SMS:%d\tMax. Num.:%d\n  Storage:%s\tNum. SMS:%d\tMax. Num.:%d\n  Storage:%s\tNum. SMS:%d\tMax. Num.:%d\n=>\n",
//				mem_info.mem1.mem,mem_info.mem1.cur_num,mem_info.mem1.max_nums,
//				mem_info.mem2.mem,mem_info.mem2.cur_num,mem_info.mem2.max_nums,
//				mem_info.mem3.mem,mem_info.mem3.cur_num,mem_info.mem3.max_nums);
//	}

	return ret;
}

/************************************/
/****** Sending SMS Function ********/
/************************************/
int SMS_Send(char* Tel_Num, char* Text, SMS_MODE SMS_Mode, SMS_TYPE SMS_Type)
{
	int Ret = 0;

	printf("<= *SENDING SMS*\n    TEL:%s\n    Text:%s\n=>\n", Tel_Num, Text);

	if(SMS_Mode == TEXT_SMS_MODE)
	{
		Ret = ql_sms_send_text_msg(Tel_Num, Text, SMS_Type);
	}
	else if(SMS_Mode == PDU_SMS_MODE)
	{
		Ret = ql_sms_send_pdu_msg(Tel_Num, Text, SMS_Type);
	}

	if(Ret)
		Err_Disp("ql_sms_send_msg", Ret);
	else
		printf("<= SMS successfully sent - ret:%d =>\n", Ret);

	return Ret;
}

/***********************************/
/**** Parameters Initialization ****/
/***********************************/
void Param_Init(void)
{
	memset(&APN_Param_Struct, 0, sizeof(APN_PARAM_STRUCT_TYPEDEF));
	APN_Param_Struct.profile_idx 	= 1;
	APN_Param_Struct.ip_type		= IPV4V6;

	memset(&Flags_Struct, DOWN, sizeof(FLAGS_STRUCT_TYPEDEF));

	strcpy(Settings_Struct.TEL1, "N");
	strcpy(Settings_Struct.TEL2, "N");
	strcpy(Settings_Struct.TEL3, "N");
	strcpy(Settings_Struct.TEL4, "N");
	strcpy(Settings_Struct.TEL5, "N");

//	printf("<= TEL1: %s,\n   TEL2: %s,\n   TEL3: %s,\n   TEL4: %s,\n   TEL5: %s\n=>\n",
//			Settings_Struct.TEL1,
//			Settings_Struct.TEL2,
//			Settings_Struct.TEL3,
//			Settings_Struct.TEL4,
//			Settings_Struct.TEL5);

	/* UART DCB */
	dcb.baudrate	= B_115200	;
	dcb.databit		= DB_CS8	;
	dcb.flowctrl	= FC_NONE	;
	dcb.stopbit 	= SB_1		;
	dcb.parity 		= PB_NONE	;

	printf("<= Parameters Initialized. =>\n");
}

/************************************/
/******* Initialize Function ********/
/************************************/
void Initialize(void)
{
	Param_Init	() ;
	NW_Init		() ;
//	WAN_Init	() ;
//	SMS_Init	() ;
	UART_Init	() ;
	I2C_Init	() ;

	printf("<= Initialization done. =>\n");
}

/**************************************/
/******* Initialize UART Ports ********/
/**************************************/
void UART_Init(void)
{
	UART_Struct.Main_UART_Struct.fd_uart = Ql_UART_Open(QL_UART_MAIN_DEV, B_115200, FC_NONE);
	if(UART_Struct.Main_UART_Struct.fd_uart == -1)
		Err_Disp("Ql_UART_Open", UART_Struct.Main_UART_Struct.fd_uart);
	else
		printf("<= Ql_UART_Open - ret: %d =>\n", UART_Struct.Main_UART_Struct.fd_uart);

	Ql_UART_SetDCB(UART_Struct.Main_UART_Struct.fd_uart, &dcb);
}

/****************************************/
/********** Pthreads Function ***********/
/****************************************/
void Pthreads_Func (void)
{

}

/**********************************************/
/********** TCP Connection Function ***********/
/**********************************************/
void TCP_Connection (void)
{
	int TCP_Connection_Ret;
	while(1)
	{
		if(Flags_Struct.TCP_Struct.TCP_Ready_To_Start == UP)
		{
			TCP_Param_Struct.tcp_client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if (TCP_Param_Struct.tcp_client_sockfd < 0)
			{
				Err_Disp("tcp_client_sockfd", TCP_Param_Struct.tcp_client_sockfd);
			}
			else
			{
				do
				{
					TCP_Param_Struct.Server_Port = 30105;
					strcpy(TCP_Param_Struct.Server_IP, "109.125.142.200");

					ret = inet_aton(TCP_Param_Struct.Server_IP, &TCP_Param_Struct.Dest.sin_addr.s_addr);

					TCP_Param_Struct.Dest.sin_family 	= AF_INET;
					TCP_Param_Struct.Dest.sin_port		= htons(TCP_Param_Struct.Server_Port);

					TCP_Connection_Ret = connect(TCP_Param_Struct.tcp_client_sockfd, &TCP_Param_Struct.Dest, sizeof(TCP_Param_Struct.Dest));
					if (!TCP_Connection_Ret)
					{
						Flags_Struct.TCP_Struct.Connected = UP;
						printf("<=\n\tTCP Connected\n\t\tRet:%d\n\t\tS-IP:%s\n\t\tS-Port:%d\n=>\n", TCP_Connection_Ret, TCP_Param_Struct.Server_IP, TCP_Param_Struct.Server_Port);
					}
					else
					{
						Flags_Struct.TCP_Struct.Connected = DOWN;
						Err_Disp("TCP_Connection_Ret", TCP_Connection_Ret);
					}

					sleep(1);
				}
				while(TCP_Connection_Ret);
			}

			Flags_Struct.TCP_Struct.TCP_Ready_To_Start = DOWN;
			break;
		}
		sleep(1);
	}
}

/*********************************************/
/********** TCP Send Data Function ***********/
/*********************************************/
int TCP_Send_Data (int TCP_Socket, char* Data_Buffer)
{
	if(Flags_Struct.TCP_Struct.Connected == UP)
	{
		uint32_t Total_Sent_Data_Size 	= 0;
		int32_t  Sent_Data_Size 		= 0;
		uint32_t Data_buffer_Size		= strlen(Data_Buffer);

		printf("<= Data: %s , Size: %d =>\n", Data_Buffer, Data_buffer_Size);

		do
		{
			Sent_Data_Size = send(TCP_Socket, Data_Buffer+Total_Sent_Data_Size, Data_buffer_Size-Total_Sent_Data_Size, 0);

			if(Sent_Data_Size < 0)
			{
				Err_Disp("TCP_Send_Data - send", Data_buffer_Size);
				Flags_Struct.TCP_Struct.Error_Send_Receive = UP;
				return ret;
			}
			else
			{
				Total_Sent_Data_Size += Sent_Data_Size;
				Flags_Struct.TCP_Struct.Error_Send_Receive = DOWN;
			}

			usleep(500);
			printf("<= All bytes sent: %d =>\n", Sent_Data_Size);
		}
		while(Total_Sent_Data_Size < Data_buffer_Size);
		printf("<= Total_Sent_Data_Size : %d =>\n", Total_Sent_Data_Size);
		return ret;

	}
}

/************************************************/
/********** TCP Receive Data Function ***********/
/************************************************/
void TCP_Rece_Data (void)
{
	printf("<= TCP receive data is running ... =>\n");

	uint32_t 	num_bytes	= 0	;
	fd_set 		sock_fdset		;

	struct timeval timeout_recv	;
	timeout_recv.tv_sec = 0		;
	timeout_recv.tv_usec= 100	;

	while(1)
	{
		if(Flags_Struct.TCP_Struct.Connected == UP)
		{
//			tmp_fd=0;

			FD_ZERO(&sock_fdset);
			FD_SET(TCP_Param_Struct.tcp_client_sockfd, &sock_fdset);

			ret = select(TCP_Param_Struct.tcp_client_sockfd+1,&sock_fdset,NULL,NULL,&timeout_recv);

		    if(ret == -1)
		    {
		    	Err_Disp("TCP_Rece_Data - select", ret);
		    }
		    else if (ret == 0)
		    {
		    	timeout_recv.tv_sec	= 0		;
		    	timeout_recv.tv_usec= 100	;
		    }
		    else
		    {
		    	if(FD_ISSET(TCP_Param_Struct.tcp_client_sockfd,&sock_fdset))
		    	{
					memset(TCP_Param_Struct.RX_Buffer, 0, sizeof(TCP_Param_Struct.RX_Buffer));
//					TCP_Param_Struct.RX_Size = 0;
					num_bytes = 0;

					num_bytes = recv(TCP_Param_Struct.tcp_client_sockfd, TCP_Param_Struct.RX_Buffer, sizeof(TCP_Param_Struct.RX_Buffer), 0);

					if(num_bytes < 0)
					{
						Err_Disp("TCP_Rece_Data - read", num_bytes);
						Flags_Struct.TCP_Struct.Error_Send_Receive = UP;
						break;
					}
					else if(num_bytes > 0)
					{
						printf("<= Received Data: %s , num: %d =>\n", TCP_Param_Struct.RX_Buffer, num_bytes);
					}
		    	}
		    }
		}
		usleep(500);
	}
}

/**************************************************************/
/********** TCP Client Checking Connection Function ***********/
/**************************************************************/
int TCP_Client_Check (void)
{
	while(1)
	{
		if(Flags_Struct.TCP_Struct.Connected == UP && Flags_Struct.TCP_Struct.Error_Send_Receive == UP)
		{
			Close_Socket(&TCP_Param_Struct.tcp_client_sockfd);
			TCP_Connection();
			printf("<= TCP Connection Reconnected! =>\n");
		}
		sleep(1);
	}
}

/**********************************************/
/********** Closing Socket Function ***********/
/**********************************************/
void Close_Socket(int* TCP_Client_SockFD)
{
	int sh=shutdown(TCP_Client_SockFD, SHUT_RDWR);
	int cl=close(TCP_Client_SockFD);

	TCP_Client_SockFD = 0;

	Flags_Struct.TCP_Struct.Connected 			= DOWN;
	Flags_Struct.TCP_Struct.Error_Send_Receive 	= DOWN;
	Flags_Struct.TCP_Struct.TCP_Ready_To_Start 	= UP;

	printf("<= Socket is completely closed =>\n");
}

/**************************************************************/
/********** Reading Data through Main UART Function ***********/
/**************************************************************/
void UART_Main_Read (void)
{
	int iRet;
	fd_set fdset;
	struct timeval timeout = {0, 1000};	// timeout 1s , 0us

	while (UART_Struct.Main_UART_Struct.fd_uart >=0)
	{
		//Create a file descriptor set and add the UART file descriptor to it
		FD_ZERO(&fdset);
		FD_SET(UART_Struct.Main_UART_Struct.fd_uart, &fdset);

		//Set timeout to zero for non-blocking behavior
		timeout.tv_sec  = 0;
		timeout.tv_usec = 10000;

		//Use select to check for available data to read from UART
		iRet = select(UART_Struct.Main_UART_Struct.fd_uart + 1, &fdset, NULL, NULL, &timeout);
		if (-1 == iRet)
		{//Select Error
			printf("<= failed to select =>\n");
			break;
		}
		else if (0 == iRet)
		{// no data in Rx buffer
//			printf("<= no data =>\n");

		}
		else
		{// data is in Rx data
			if (FD_ISSET(UART_Struct.Main_UART_Struct.fd_uart, &fdset))
			{
				do
				{
					memset(UART_Struct.Main_UART_Struct.RX_buffer, 0x0, sizeof(UART_Struct.Main_UART_Struct.RX_buffer));
					iRet = Ql_UART_Read(UART_Struct.Main_UART_Struct.fd_uart, UART_Struct.Main_UART_Struct.RX_buffer, UART_READ_BUF_SIZE);
					sleep(1);
					if  (DB_CS5 == dcb.databit)
		            {
			            printf("> read(uart)=%d:%d, %d, %d, %d, %d, %d, %d, %d, %d\n",
			            		iRet, UART_Struct.Main_UART_Struct.RX_buffer[0],UART_Struct.Main_UART_Struct.RX_buffer[1],UART_Struct.Main_UART_Struct.RX_buffer[2],UART_Struct.Main_UART_Struct.RX_buffer[3],UART_Struct.Main_UART_Struct.RX_buffer[4],UART_Struct.Main_UART_Struct.RX_buffer[5],UART_Struct.Main_UART_Struct.RX_buffer[6],UART_Struct.Main_UART_Struct.RX_buffer[7],UART_Struct.Main_UART_Struct.RX_buffer[8]);
		            }
		            else
		            	printf("|=> read(uart)=size:%d - data:%s\n", iRet, UART_Struct.Main_UART_Struct.RX_buffer);
				} while (UART_READ_BUF_SIZE == iRet);
			}
		}
	}
}

/************************************************************/
/********** Wring Data through Main UART Function ***********/
/************************************************************/
uint32_t UART_Main_Write (char *TX_Buff)
{
	UART_Struct.Main_UART_Struct.TX_size = 0;
	memset(UART_Struct.Main_UART_Struct.TX_buffer, 0, sizeof(UART_Struct.Main_UART_Struct.TX_buffer));
	memcpy(UART_Struct.Main_UART_Struct.TX_buffer, TX_Buff, sizeof(TX_Buff));

	while(1)
	{
		UART_ret = Ql_UART_Write(UART_Struct.Main_UART_Struct.fd_uart, TX_Buff+UART_Struct.Main_UART_Struct.TX_size, strlen(TX_Buff)-UART_Struct.Main_UART_Struct.TX_size);
//		printf("<= UART - ret:%d , TX_Len:%d =>\r\n", UART_ret, strlen(TX_Buff)-UART_Struct.Main_UART_Struct.TX_size);

		if(UART_ret == (strlen(TX_Buff)-UART_Struct.Main_UART_Struct.TX_size))
		{
			printf("<= UART Send done - sent:%d , remn:%d =>\r\n", UART_ret, strlen(TX_Buff)-UART_Struct.Main_UART_Struct.TX_size);
			break;
		}
		else if(UART_ret > 0)
		{
			UART_Struct.Main_UART_Struct.TX_size += UART_ret;
			printf("<= UART Sending ... - sent:%d , remn:%d=>\r\n", UART_ret, strlen(TX_Buff)-UART_Struct.Main_UART_Struct.TX_size);
		}
		else
		{
			Err_Disp("Ql_UART_Write", UART_ret);
			break;
		}

		usleep(500);
	}

	return UART_ret;
}

/***********************************************/
/********** Initialize I2C Protocols ***********/
/***********************************************/
void I2C_Init (void)
{
	I2C_Struct.fd_i2c = Ql_I2C_Init(I2C_DEV);

	if(I2C_Struct.fd_i2c<0)
		Err_Disp("Ql_I2C_Init", I2C_Struct.fd_i2c);
	else
		printf("<= I2C Init SUCCESS - ret = %d =>\n", I2C_Struct.fd_i2c);
}

/***********************************************/
/********** DS1307 Set Time Function ***********/
/***********************************************/
int DS1307_Set_Time (uint8_t year, uint8_t month, uint8_t date, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, bool H_12)
{
	uint8_t time[7]={0,0,0,0,0,0,0};

	time[6] = ((year/10)<<4)	| (year%10)						;
	time[5] = ((month/10)<<4)	| (month%10)					;
	time[4] = ((date/10)<<4)	| (date%10)						;
	time[3] = (day & 0x07)										;
	time[2] = ((hour/10)<<4)	| (hour%10)		| (H_12*0x64)	;
	time[1] = ((minute/10)<<4)	| (minute%10)					;
	time[0] = ((second/10)<<4)	| (second%10)					;

	return Ql_I2C_Write(I2C_Struct.fd_i2c, DS1307_I2C_SLAVE_ADDR, 0, time, 7);
}

/***********************************************/
/********** DS1307 Get Time Function ***********/
/***********************************************/
void DS1307_Get_Time (DS1307_I2C_STRUCT_TYPEDEF* DS1307_Time)
{
	uint8_t time[7];

    Ql_I2C_Read(I2C_Struct.fd_i2c, DS1307_I2C_SLAVE_ADDR, 0, time, 7);

    DS1307_Time->year 	= (((time[6]>>4)&0x0F)*10) + (time[6]&0x0F)	;
    DS1307_Time->month 	= (((time[5]>>4)&0x01)*10) + (time[5]&0x0F)	;
    DS1307_Time->date 	= (((time[4]>>4)&0x03)*10) + (time[4]&0x0F)	;
    DS1307_Time->day 	= (time[3]&0x07)							;
    DS1307_Time->H_12 	= (time[2]&0x64)							;
    DS1307_Time->hour 	= (((time[2]>>4)&0x03)*10) + (time[2]&0x0F)	;
    DS1307_Time->minute = (((time[1]>>4)&0x07)*10) + (time[1]&0x0F)	;
    DS1307_Time->second = (((time[0]>>4)&0x07)*10) + (time[0]&0x0F)	;
}

/***************************************************************/
/********** Turning DLMS To Gateway Frame Function ***********/
/***************************************************************/
uint8_t DLMS2GW_Frame_Convertor (uint8_t* DLMS2GW_Prtcl_Frame, uint8_t Logical_Address, uint8_t Client_Address, uint8_t Control_Byte, uint16_t GW_DST_Add)
{
	uint8_t	APDU[65536]			;
	uint8_t 	Add_Len 		= 0	;
	uint8_t	last_byte 		= 0	;
	uint8_t 	APDU_start_add	= 0	;
	uint8_t 	dst_size 		= 0	;
	uint16_t 	phy_add 		= 0	;
	uint16_t 	HCS 			= 0	;
	uint16_t 	FCS 			= 0	;
	uint16_t 	APDU_size 		= 0	;
	uint16_t 	MAC_frame_Size 	= 0	;
	uint16_t 	dst_add_log 	= 0	;
	uint16_t 	dst_add_phy 	= 0	;
	uint16_t 	Frame_Format	= 0	;

	if(DLMS2GW_Prtcl_Frame[0] == 0 && DLMS2GW_Prtcl_Frame[1] == 1) 			//Checking version 0x0001
	{
		if( DLMS2GW_Prtcl_Frame[2] == 0 && DLMS2GW_Prtcl_Frame[3] == 1)		//Checking source Address 0x0001
		{
			if((DLMS2GW_Prtcl_Frame[4] == ((uint8_t) (GW_DST_Add >> 8))) && (DLMS2GW_Prtcl_Frame[5] == ((uint8_t) (GW_DST_Add & 0x00FF))))	//Checking destination address - gprs meter's address
			{
				if(DLMS2GW_Prtcl_Frame[8] == 0xE6)					//Checking header - must be 0xE6
				{
					if(DLMS2GW_Prtcl_Frame[9] == 0x00)				//Checking Network ID - shall be 0x00
					{
						memset(APDU, 0, sizeof(APDU));

						Add_Len = DLMS2GW_Prtcl_Frame[10];			//Reading Physical Address length from 10th array of received format

						APDU_start_add = 11 + DLMS2GW_Prtcl_Frame[10];											//Calculating start point to read APDU information
						APDU_size = (((uint16_t) (DLMS2GW_Prtcl_Frame[6])) << 8) | DLMS2GW_Prtcl_Frame[7];		//Reading APDU bytes size from received frame
						memcpy(APDU, DLMS2GW_Prtcl_Frame + APDU_start_add, APDU_size);

						for(int i=11; i<11+Add_Len; i++)								//Turning physical address from ASCII to decimal
						{
							phy_add += (DLMS2GW_Prtcl_Frame[i]-0x30)*pow(10,10+Add_Len-i);
						}

						MAC_frame_Size = 10;	//Min MAC frame size

						if(0 <= phy_add && phy_add <= 127)								//Max physical address (1-byte): 0b 0111 1111 = 127 => 0b 1111 1111
						{
							dst_size = 2;
							MAC_frame_Size += 2;
						}
						else if(128 <= phy_add && phy_add <= 16383) 					//Max physical address(2-byte): 0b 0011 1111 1111 1111 = 16383 => 0b 1111 1110 1111 1111
						{
							dst_size = 4;
							MAC_frame_Size += 4;
						}
						else
						{
							printf("Physical Address must be between 0 and 16383.\n");
							return 9;
						}

						MAC_frame_Size += APDU_size;

						uint8_t MAC_frame[MAC_frame_Size];
						memset(MAC_frame, 0, sizeof(MAC_frame));

						MAC_frame[0] 				= 0x7E;		//Start flag
						MAC_frame[MAC_frame_Size-1] = 0x7E;		//End flag

						Frame_Format = ((MAC_frame_Size - 2) & 0x07FF) | (0xA000)	;
						MAC_frame[1] = (uint8_t) (Frame_Format >> 8)				;
						MAC_frame[2] = (uint8_t) (Frame_Format & 0xFF)				;

						if(0 <= Logical_Address && Logical_Address <= 16383)
						{
							dst_add_log = ((Logical_Address << 2) & (0xFE00)) | ((Logical_Address << 1) & (0x00FE));
						}
						else
						{
							printf("Logical Address must be between 0 and 16383.\n");
							return 8;
						}

						dst_add_phy = ((phy_add << 2) & (0xFE00)) | ((phy_add << 1) & (0x00FE)) | 0x1;

						if(dst_size == 4)			//Combining logical address with physical address and turning to source address
						{
							MAC_frame[3] = (uint8_t) (dst_add_log >> 8);
							MAC_frame[4] = (uint8_t) (dst_add_log & 0xFF);
							MAC_frame[5] = (uint8_t) (dst_add_phy >> 8);
							MAC_frame[6] = (uint8_t) (dst_add_phy & 0xFF);

							last_byte = 6;
						}
						else if(dst_size == 2)
						{
							MAC_frame[3] = (uint8_t) (dst_add_log & 0xFF);
							MAC_frame[4] = (uint8_t) (dst_add_phy & 0xFF);

							last_byte = 4;
						}

						if(Client_Address >= 0 && Client_Address <= 127)
						{
							MAC_frame[last_byte + 1] = ((Client_Address << 1) & (0xFE)) | 0x1;		//Turning client address to source address
						}
						else
						{
							printf("Client Address must be between 0 and 127.\n");
							return 7;
						}

						if(Control_Byte>=0 && Control_Byte<=255)
						{
							MAC_frame[last_byte + 2] = Control_Byte;
						}
						else
						{
							printf("Physical Address must be between 0 and 255.\n");
							return 6;
						}


						HCS = countCRC(MAC_frame, 1, last_byte+2);
						MAC_frame[last_byte + 3] = (uint8_t) (HCS >> 8);
						MAC_frame[last_byte + 4] = (uint8_t) (HCS & 0x00FF);

						for(int k=0; k<APDU_size; k++)
						{
							MAC_frame[last_byte + 5 + k] = APDU[k];
						}

						last_byte = last_byte + 4 + APDU_size;

						FCS = countCRC(&MAC_frame, 1, last_byte);
						MAC_frame[last_byte + 1] = (uint8_t) (FCS >> 8);
						MAC_frame[last_byte + 2] = (uint8_t) (FCS & 0x00FF);

						memset(Gate_Meter_Prtcl, 0, sizeof(Gate_Meter_Prtcl));
						memcpy(Gate_Meter_Prtcl, MAC_frame, sizeof(MAC_frame));
						Gate_Meter_size = MAC_frame_Size;
					}
					else
					{
						printf("Network ID must be 0x00.\n");
						return 5;
					}
				}
				else
				{
					printf("Header must be 0xE6.\n");
					return 4;
				}
			}
			else
			{
				printf("Destination Gateway Address must be %d.\n", GW_DST_Add);
				return 3;
			}
		}
		else
		{
			printf("Source Address must be 0x0001.\n");
			return 2;
		}
	}
	else
	{
		printf("Version must be 0x0001.\n");
		return 1;
	}
	return 0;
}

/**************************************************************/
/********** Turning Meter To Gateway Frame Function ***********/
/**************************************************************/
uint8_t Meter2GW_Frame_Convertor (uint8_t* Meter2GW_Prtcl_Frame, uint8_t Source_Address)
{
	uint8_t	last_byte			= 0;
	uint16_t	Phy_Add				= 0;
	uint16_t	DLMS_IP_Prtcl_Size	= 0;

	uint8_t 	HES_Frame[500];
	uint16_t 	HES_Frame_Size;

	HES_Frame_Size = 11;

	memset(HES_Frame, 0, sizeof(HES_Frame));


	if(Meter2GW_Prtcl_Frame[0] == 0x7E )
	{
		if((Meter2GW_Prtcl_Frame[1] & 0xF0) == 0xA0 )
		{
			HES_Frame[0] = 0;
			HES_Frame[1] = 1;				//Version

			HES_Frame[2] = 0;
			HES_Frame[3] = Source_Address;	//GPRS meter's address

			HES_Frame[4] = 0;
			HES_Frame[5] = 1;				//Destination Address must be 1

			HES_Frame[8] = 0xE7;			//Header
			HES_Frame[9] = 0;				//Network ID
			HES_Frame[10]= 0;

			DLMS_IP_Prtcl_Size = (((((uint16_t) (Meter2GW_Prtcl_Frame[1])) << 8) | ((uint16_t) (Meter2GW_Prtcl_Frame[2]))) & 0x07FF) + 2;

			for(int i=3; i<DLMS_IP_Prtcl_Size; i++)		//Finding end of destination address
			{
				if((Meter2GW_Prtcl_Frame[i] & 0x1) == 0x1)
				{
					last_byte = i;
					break;
				}
			}

			for(int i=last_byte+1; i<DLMS_IP_Prtcl_Size; i++)	//Finding end of source address
			{
				if((Meter2GW_Prtcl_Frame[i] & 0x1) == 0x1)
				{
					if(i-last_byte == 4)		//Detecting size of source address
					{
						Phy_Add = (uint16_t) ((Meter2GW_Prtcl_Frame[i] >> 1) | ((uint8_t) (Meter2GW_Prtcl_Frame[i-1] << 6)));
						Phy_Add = Phy_Add | ((uint16_t) ((Meter2GW_Prtcl_Frame[i-1] & 0xFC)<<6));
						printf("Phy_Add = %d\n", Phy_Add);

						uint16_t Phy_Add_Copy = Phy_Add;
						uint8_t Phy_Add_len = 0;
						uint8_t Phy_Add_buf = 0;

						do								//Counting the number of physical address digits
						{
							Phy_Add_Copy = Phy_Add_Copy/10;
							HES_Frame[10] ++;
						}
						while(Phy_Add_Copy > 0);

						Phy_Add_len = HES_Frame[10];
						Phy_Add_Copy = Phy_Add;

						HES_Frame_Size += Phy_Add_len;

						while(Phy_Add_len>0)			//Turning physical address to ASCII
						{
							Phy_Add_buf = Phy_Add_Copy / pow(10, Phy_Add_len-1);
							Phy_Add_Copy = Phy_Add_Copy % (uint32_t) (pow(10, Phy_Add_len-1));
							Phy_Add_len --;
							HES_Frame[10 + HES_Frame[10] - Phy_Add_len] = 0x30+Phy_Add_buf;
						}

					}
					else if(i-last_byte == 2)
					{
						Phy_Add = (Meter2GW_Prtcl_Frame[i] >> 1);

						uint16_t Phy_Add_Copy = Phy_Add;
						uint8_t Phy_Add_len = 0;
						uint8_t Phy_Add_buf = 0;

						do
						{
							Phy_Add_Copy = Phy_Add_Copy/10;
							HES_Frame[10] ++;
						}
						while(Phy_Add_Copy > 0);

						Phy_Add_len		= HES_Frame[10]	;
						Phy_Add_Copy 	= Phy_Add		;

						HES_Frame_Size += Phy_Add_len;

						while(Phy_Add_len>0)
						{
							Phy_Add_buf		= Phy_Add_Copy / pow(10, Phy_Add_len-1);
							Phy_Add_Copy 	= Phy_Add_Copy % (uint32_t) (pow(10, Phy_Add_len-1));
							Phy_Add_len --;
							HES_Frame[10 + HES_Frame[10] - Phy_Add_len] = 0x30+Phy_Add_buf;
						}
					}
					else
					{
						printf("Source Address is wrong. \n");
						return 5;
					}
					uint16_t HES_APDU_Byte = 10+HES_Frame[10]+1;

					last_byte = i;

					// i+1 = control byte

					uint16_t HCS_Frame = 0;
					uint16_t HCS_Cal 	= 0;
					HCS_Frame 	= ((uint16_t) (Meter2GW_Prtcl_Frame[last_byte+2]) << 8) | (Meter2GW_Prtcl_Frame[last_byte+3]);
					HCS_Cal 	= countCRC(Meter2GW_Prtcl_Frame, 1, last_byte + 1);
					last_byte 	= last_byte +3;

					uint16_t APDU_len 	= DLMS_IP_Prtcl_Size - last_byte -4;		//4 = len(start flag) + len(end flag)
					HES_Frame_Size		+= APDU_len;
					HES_Frame[6] 		= (uint8_t) (APDU_len >> 8);
					HES_Frame[7] 		= (uint8_t) (APDU_len & 0x00FF);

					if(HCS_Frame == HCS_Cal)
					{
						for(int j = last_byte+1; j < DLMS_IP_Prtcl_Size-3; j++)
						{
							HES_Frame[HES_APDU_Byte] = Meter2GW_Prtcl_Frame[j];
							HES_APDU_Byte ++;
						}
						last_byte = DLMS_IP_Prtcl_Size - 4;

						uint16_t FCS_Frame = 0;
						uint16_t FCS_Cal 	= 0;
						FCS_Frame 	= ((uint16_t) (Meter2GW_Prtcl_Frame[last_byte + 1]) << 8) | (Meter2GW_Prtcl_Frame[last_byte + 2]);
						FCS_Cal 	= countCRC(Meter2GW_Prtcl_Frame, 1, last_byte);

						if(FCS_Frame == FCS_Cal)
						{
							memset(Gate_HES_Prtcl, 0, sizeof(Gate_HES_Prtcl));
							memcpy(Gate_HES_Prtcl, HES_Frame, sizeof(HES_Frame));
							Gate_HES_size = HES_Frame_Size;
						}
						else
						{
							printf("FCS is wrong. \n");
							return 5;
						}
					}
					else
					{
						printf("HCS is wrong. \n");
						return 4;
					}

					break;
				}
			}

			if(Meter2GW_Prtcl_Frame[DLMS_IP_Prtcl_Size-1] != 0x7E)
			{
				printf("End flag is wrong. \n");
				return 3;
			}
		}
		else
		{
			printf("Frame format field is wrong.-0x%x \n", Meter2GW_Prtcl_Frame[1]);
			return 2;
		}
	}
	else
	{
		printf("Start flag is wrong. \n");
		return 1;
	}
	return 0;
}

/***********************************************/
/********** Calculating CRC Function ***********/
/***********************************************/
static uint16_t countCRC(char* Buff, uint32_t index, uint32_t count)
{
   uint16_t tmp;
   uint16_t FCS16 = 0xFFFF;
   uint16_t pos;
   for (pos = 0; pos < count; ++pos)
   {

	   FCS16 = (FCS16 >> 8) ^ FCS16Table[(FCS16 ^ ((unsigned char*)Buff)[index + pos]) & 0xFF];
   }
   FCS16 = ~FCS16;
   //CRC is in big endian byte order.
   tmp = FCS16;
   FCS16 = tmp >> 8;
   FCS16 |= tmp << 8;
   return FCS16;
}

/***********************************************/
/********** HAJIAN - startServers ***********/
/***********************************************/
int startServers(int port, int trace)
{
    int ret;

    connection lnWrapper , lniec , rs485;

   //Initialize DLMS settings.
    svr_init(&lnWrapper.settings, 1, DLMS_INTERFACE_TYPE_WRAPPER, WRAPPER_BUFFER_SIZE, PDU_BUFFER_SIZE, ln47frameBuff, WRAPPER_BUFFER_SIZE, ln47pduBuff, PDU_BUFFER_SIZE);
    // //We have several server that are using same objects. Just copy them.
    unsigned char KEK[16] = { 0x31,0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31 };
    BB_ATTACH(lnWrapper.settings.base.kek, KEK, sizeof(KEK));
    svr_InitObjects(&lnWrapper.settings);

    DLMS_INTERFACE_TYPE interfaceType;
    if(lnWrapper.settings.localPortSetup->defaultMode==0) interfaceType=DLMS_INTERFACE_TYPE_HDLC_WITH_MODE_E;
    else interfaceType=DLMS_INTERFACE_TYPE_HDLC;

    //Initialize DLMS settings.
    svr_init(&lniec.settings, 1, interfaceType, HDLC_BUFFER_SIZE, PDU_BUFFER_SIZE, lnframeBuff, HDLC_HEADER_SIZE + HDLC_BUFFER_SIZE, lnpduBuff, PDU_BUFFER_SIZE);

    svr_InitObjects(&lniec.settings);

    //Start server
    if ((ret = svr_start(&lnWrapper, port)) != 0)
    {
        return ret;
    }

    //Start server
    if ((ret = svr_start_Serial(&lniec, "/dev/ttyUSB0")) != 0)
    {
        return ret;
    }

    lnWrapper.trace =lniec.trace =  trace;

    uint32_t lastMonitor = 0;
    while (1)
    {
        //Monitor values only once/second.
        if (time_current() - lastMonitor > 1)
        {
            lastMonitor = time_current();
            if ((ret = svr_monitorAll(&lnWrapper.settings)) != 0)
            {
                printf("lnWrapper monitor failed.\r\n");
            }
        }

    }
    con_close(&lnWrapper);
    return 0;
}

/************************************/
/********** Main Function ***********/
/************************************/
int main(int argc, char* argv[])
{
    int  port = 4060;
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
    startServers(port, GX_TRACE_LEVEL_INFO);
    return 0;
}


