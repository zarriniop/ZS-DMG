/**  
  @file
  ql_voice.h

  @brief
  This file provides the definitions for voice, and declares the 
  API functions.

*/
/*============================================================================
  Copyright (c) 2017 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
 =============================================================================*/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.


WHEN        WHO            WHAT, WHERE, WHY
----------  ------------   ----------------------------------------------------
14/11/2019  gunner.yang      create
06/01/2020  pony.made        add net_state
18/02/2020  gale.gao       Add API: imsd get, wait status get, voice unhold;
=============================================================================*/

#ifndef __QL_VOICE_H__
#define __QL_VOICE_H__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
	QL_VOICE_SUCCESS,
	QL_VOICE_GENERIC_FAILURE,
 	QL_VOICE_RADIO_NOT_AVAILABLE,
}QL_VOICE_ERROR_CODE;

typedef enum{
    GSM = 0,
    GSM_COMPACT,
    GSM_wEGPRS,
    UTRAN,
    UTRAN_wHSDPA,
    UTRAN_wHSUPA,
    UTRAN_wHSDPA_HSUPA,
    E_UTRAN,
    UTRAN_HSPA_PLUS,
}QL_REG_STATE;

typedef enum{
    REASON_INVALID=-1,
    REASON_UNCONDITIONAL =0,
    REASON_DEVICE_BUSY =1,
    REASON_DEVICE_NO_REPLY =2,
    REASON_DEVICE_UNREACHABLE =3,
    REASON_DEVICE_ALL_FORWARDING =4,
    REASON_DEVICE_ALL_CONDITIONAL =5,
}QL_VOICE_CALL_FW_REASON;

typedef enum{
    MODE_DISABLE=0,
    MODE_ENABLE,
    MODE_QUERY_STATUS,
    MODE_REGISTRATION,
    MODE_ERASURE,
}QL_VOICE_CALL_FW_MODE;

typedef struct
{
    int     status;
    int     fw_class;
    char    num[20];
    int     type;
    char    subaddr[20];
    int     satype;
    int     time;
} ql_voice_call_fw_list_t;

typedef enum {
    QL_CALL_ACTIVE = 0,
    QL_CALL_HOLDING = 1,
    QL_CALL_DIALING = 2,    /* MO call only */
    QL_CALL_ALERTING = 3,   /* MO call only */
    QL_CALL_INCOMING = 4,   /* MT call only */
    QL_CALL_WAITING = 5,     /* MT call only */
    QL_CALL_OFFERING = 6,	  /*MT call offering (call setup)*/
    QL_CALL_DISCONNECTING = 7, /*call in disconnect procedure*/
    QL_CALL_DISCONNECTED = 8, /*call is disconnected*/
} QL_CallState;

typedef struct
{
    int idx;            /*call index, identification number*/
    int dir;            /*0: mobile originated(MO) call, 1: mobile terminated(MT) call*/
    int state;          /*define in QL_CallState*/
    char number[20];    /*string type phone number in format specified by <type>*/
    int type;           /*type of address octet in interger format(refer TS24.008 subclause 10.5.4.7)*/
}ql_voice_call_item;

typedef struct
{
    int count;
    ql_voice_call_item items[16];
}ql_voice_call_list_t;
/*
    callbackfunction prototype
*/
typedef void (*QL_VOICE_EventHandlerFunc_t)(QL_CallState voice_flag, char *number,void *contextPtr);

QL_VOICE_ERROR_CODE ql_voice_call_init ();
QL_VOICE_ERROR_CODE ql_voice_call_release();

/*
    Register voice call event.
     -handlerPtr    : user_cb
     -contextPtr    : user_cb_data
*/
QL_VOICE_ERROR_CODE ql_voice_call_event_register(QL_VOICE_EventHandlerFunc_t handlerPtr, void* contextPtr);


/*
    Phone call.
     -call_num   : dst phone number
*/
QL_VOICE_ERROR_CODE ql_voice_call_start(char* call_num);


/*
    Put through.
*/
QL_VOICE_ERROR_CODE ql_voice_call_answer();
QL_VOICE_ERROR_CODE ql_voice_auto_answer(int seconds);


/*
    Hang up.
*/
QL_VOICE_ERROR_CODE ql_voice_call_end();


/* dfmf call */
QL_VOICE_ERROR_CODE ql_voice_call_dtmf(char *dtmf,int duration);

/* get voice net state
 * Just use in call to judge Volte or CS-vioce 
 * regstate: 
 * 		7	-> Volte call
 * 		else	-> CS call
*/
QL_VOICE_ERROR_CODE ql_voice_call_get_regstate(int* regstate);

/* set imsd value
    1: open votle
    0: close votle
 */
QL_VOICE_ERROR_CODE ql_voice_call_set_imsd(int imsd);

/* get imsd value 
   1: open volte
   0: close volte
 */
QL_VOICE_ERROR_CODE ql_voice_call_get_imsd(int *imsd);

/* set call wait
    0: close
    1: open  */
QL_VOICE_ERROR_CODE ql_voice_call_set_wait(int wait);

/* get call wait status
    0: close
    1: open  */
QL_VOICE_ERROR_CODE ql_voice_call_get_wait(int *wait);

/* hold the voice */
QL_VOICE_ERROR_CODE ql_voice_call_hold();

/* unhold the voice */
QL_VOICE_ERROR_CODE ql_voice_call_unhold();

QL_VOICE_ERROR_CODE ql_voice_call_fw_set(QL_VOICE_CALL_FW_REASON reason, QL_VOICE_CALL_FW_MODE mode, char *num);

QL_VOICE_ERROR_CODE ql_voice_call_fw_query(QL_VOICE_CALL_FW_REASON reason, ql_voice_call_fw_list_t *resp_data);

/**
 * get the current call list, same as AT+CLCC
*/
QL_VOICE_ERROR_CODE ql_voice_call_get_current_list(ql_voice_call_list_t *resp_data);
#ifdef __cplusplus
}
#endif

#endif
