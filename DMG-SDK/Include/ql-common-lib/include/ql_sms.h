/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Quectel Co., Ltd. 2016
 *
 *****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   ql_sms.h
 *
 * Project:
 * --------
 *   OpenLinux
 *
 * Description:
 * ------------
 *   API definition for SMS.
 *
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * WHO            WHEN                WHAT
 *----------------------------------------------------------------------------
 * Pony.ma       19/09/2019           Create
 ****************************************************************************/

#ifndef __QL_SMS_H__
#define __QL_SMS_H__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
#define MAX_ADDR_LEN            24
#define FALSE                   0
#define TRUE                    1

#define QL_SMS_MIN_LEN          1       
#define MAX_LONGSMS_SEGMENT     8
#define MAX_SMS_LENGTH          (160*4)
#define SMS_BUF_LEN             (MAX_SMS_LENGTH + 32)
#define MAX_LONGSMS_LENGTH      (160*4*MAX_LONGSMS_SEGMENT)



typedef unsigned char           UINT8;
typedef unsigned short          UINT16;
typedef unsigned int            UINT32;

typedef enum{
	QL_SMS_SUCCESS,
	QL_SMS_GENERIC_FAILURE,
	QL_SMS_PRAR_ERROR,
	QL_GET_RESPONSE_ERROR,
}QL_SMS_ERROR_CODE;

typedef enum 
{
	QL_SMS_RECV_EVENT   = 0,    /**< SMS come event.  */
}QL_SMS_NFY_MSG_ID; 

typedef enum 
{
	QL_SMS_MODE_GSM = 0,    //GSM
    QL_SMS_MODE_UCS2        //UCS2
}QL_SMS_CODE_MODE;

typedef struct
{
	char number[32];
	char body[280]; // 姣忔潯鏀跺埌鐨勭煭淇￠暱搴︽渶澶т负280
	char time[32];
	int  body_len;
    QL_SMS_CODE_MODE  code_mode;
	char *pdu_str;
}QL_SMS_STATUS_INFO;

typedef struct
{
	int  item;
	char *name;
}ql_sms_item;

typedef void (*QL_SMS_StatusIndMsgHandlerFunc_t)   
	(    
	 QL_SMS_NFY_MSG_ID     msg_id,
	 void                  *pv_data,
	 int                   pv_data_len,
	 void                  *contextPtr    
	);

typedef enum{
	QL_SMS_ClientInit = 0,
	QL_SMS_ClientRelease,
	//QL_SMS_SendSMS,
	QL_SMS_DeleteSMS,	
	QL_SMS_SearchSMSTextMessage,
	QL_SMS_SearchSMSPduMessage,
	QL_SMS_AddSMSEventHandler,
	QL_SMS_GetSMSCenterAddress,
	QL_SMS_SetSMSCenterAddress,
	QL_SMS_SetSaveLocation,
	QL_SMS_GetSaveLocation,
	QL_SMS_SetSMSNotSave,
	QL_SMS_SetSMSMsgMode,
	QL_SMS_GetSMSMsgMode,
	QL_SMS_SetSMSCodeMode,
	QL_SMS_GetSMSCodeMode,
	QL_SMS_SetSMSReceiveDealmode,
	QL_SMS_SendTextSMS,
	QL_SMS_SendPduSMS,
	QL_SMS_GetSimNum,

	QL_SMS_OptionMax,
}QL_SMS_OPTION;

typedef struct
{
	char mem[5];
	int  cur_num;  //current sms numbers
	int  max_nums; //Maximum number of messages that can be stored
}sms_mem_t;

typedef struct
{
	sms_mem_t mem1; //messages to be read and deleted from this memory storage.
	sms_mem_t mem2; //messages will be written and sent to this memory storage.
	sms_mem_t mem3; //received messages will be placed in this memory storage if routing to PC is not set.
}ql_sms_mem_info_t;

typedef struct recvmessage_struct{
	char num[32];
	char buf[280];
	int buflen;	
}recvmessage;

/* init sms client */
QL_SMS_ERROR_CODE ql_sms_init();
/* deinit sms clint */
QL_SMS_ERROR_CODE ql_sms_release();
/* send message api */
//QL_SMS_ERROR_CODE ql_sms_send_msg(uint8_t *destNum,uint8_t *data,size_t dataLen,int sms_type);
/* get smsc api */
QL_SMS_ERROR_CODE ql_sms_get_sms_center_address(uint8_t *sms_center_addree);
/* register sms event header api*/
QL_SMS_ERROR_CODE ql_sms_add_event_handler(QL_SMS_StatusIndMsgHandlerFunc_t handlerPtr, void* contextPtr);
/* set smsc api */
QL_SMS_ERROR_CODE ql_sms_set_sms_center_address(unsigned char *destNum);
/* delete message api */
QL_SMS_ERROR_CODE ql_sms_delete_msg(size_t index); 
/* set sms storage api */
QL_SMS_ERROR_CODE ql_sms_set_sms_pref_storage(size_t location);
/* set sms not storage api */
QL_SMS_ERROR_CODE ql_sms_set_sms_desave();
QL_SMS_ERROR_CODE ql_set_sms_msg_mode(int messagemode);
QL_SMS_ERROR_CODE ql_get_sms_msg_mode(int *messagemode);
QL_SMS_ERROR_CODE ql_set_sms_code_mode(char* codemode);
QL_SMS_ERROR_CODE ql_get_sms_code_mode(char *codemode);
QL_SMS_ERROR_CODE ql_sms_get_sms_pref_storage(ql_sms_mem_info_t *mem_info);
//QL_SMS_ERROR_CODE ql_search_sms_message(int index, recvmessage* payload);
QL_SMS_ERROR_CODE ql_search_sms_text_message(int index, recvmessage* payload);
QL_SMS_ERROR_CODE ql_search_sms_pdu_message(int index, recvmessage* payload);
QL_SMS_ERROR_CODE ql_set_sms_recive_dealmode(int dealmode);
QL_SMS_ERROR_CODE ql_sms_send_pdu_msg(uint8_t *phone_num,uint8_t *data,int sms_type);
QL_SMS_ERROR_CODE ql_sms_send_text_msg(uint8_t *phone_num,uint8_t *data,int sms_type);
QL_SMS_ERROR_CODE ql_sms_get_sim_num(uint8_t *sim_num);

#ifdef __cplusplus
}
#endif

#endif

