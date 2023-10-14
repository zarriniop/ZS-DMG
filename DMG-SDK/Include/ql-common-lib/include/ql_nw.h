/**  
  @file
  ql_nw.h

  @brief
  This file provides the definitions for nw, and declares the 
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
14/08/2019  Nebula.li      create
=============================================================================*/

#ifndef __QL_NW_H__
#define __QL_NW_H__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define  QL_NW_IND_VOICE_REG_EVENT_FLAG  (1 << 0)  //VOICE注册事件
#define  QL_NW_IND_DATA_REG_EVENT_FLAG  (1 << 1)   //DATA注册事件
#define  QL_NW_IND_SIGNAL_STRENGTH_EVENT_FLAG  (1 << 2)  //信号强度事件
#define  QL_NW_IND_NITZ_TIME_UPDATE_EVENT_FLAG  (1 << 3)  //时间跟新事件
//START baron-2020.02.29: add for ims register state indication
#define  QL_NW_IND_IMS_NETWORK_STATE_CHANGED_FLAG    (1 << 4) //ims register state change
//END
#define CELL_MAX_NUM 20

typedef enum 
{
	QL_NW_SUCCESS,
	QL_NW_GENERIC_FAILURE,
 	QL_NW_RADIO_NOT_AVAILABLE,
}QL_NW_ERROR_CODE;

typedef enum {
  QL_NW_PREF_NET_TYPE_GSM_WCDMA  = 0, 
  QL_NW_PREF_NET_TYPE_GSM_ONLY  = 1, 
  QL_NW_PREF_NET_TYPE_WCDMA  = 2, 
  QL_NW_PREF_NET_TYPE_GSM_WCDMA_AUTO  = 3, //根据 PRL自动选择GSM、WCDMA
  QL_NW_PREF_NET_TYPE_LTE_GSM_WCDMA  = 9, //自动选择LTE、GSM、WCDMA
  QL_NW_PREF_NET_TYPE_LTE_ONLY  = 11,
  QL_NW_PREF_NET_TYPE_LTE_WCDMA  = 12,
  QL_NW_PREF_NET_TYPE_LTE_GSM_GSM_PREF = 13, //优先GSM
  QL_NW_PREF_NET_TYPE_LTE_GSM_LTE_PREF = 14, //优先LTE
  QL_NW_PREF_NET_TYPE_LTE_GSM = 15,
  QL_NW_PREF_NET_TYPE_LTE_WCDMA_WCDMA_PREF = 16,
  QL_NW_PREF_NET_TYPE_LTE_WCDMA_LTE_PREF = 17,
  QL_NW_PREF_NET_TYPE_LTE_GSM_WCDMA_GSM_PREF   = 19,
  QL_NW_PREF_NET_TYPE_LTE_GSM_WCDMA_WCDMA_PREF = 20,
  QL_NW_PREF_NET_TYPE_LTE_GSM_WCDMA_LTE_PREF   = 21,
} QL_NW_PREFERRED_NETWORK_TYPE;

typedef struct 
{
  QL_NW_PREFERRED_NETWORK_TYPE  preferred_nw_mode;  //首选的网络制式，见QL_NW_PREFERRED_NETWORK_TYPE
  int  roaming_pref;      //漫游开关。 0 关； 1 开
}QL_NW_CONFIG_INFO_T;

typedef struct 
{
  char        nitz_time[32];  //格式：format: YY/MM/DD HH:MM:SS '+/-'TZ daylight,   18/09/19 07:40:18 +32 00.
  unsigned long  abs_time;   //0表示不可用
  unsigned char      leap_sec;   //0表示不可用
}QL_NW_NITZ_TIME_INFO_T;

typedef struct
{
  char long_eons[128];
  char short_eons[128];
  char mcc[4];
  char mnc[4];
}QL_NW_OPERATOR_INFO_T;

//START baron-2020.02.29: add for ims register state
typedef struct __volte_state
{
    int reg_state;    /*0: not registered, 1: registered*/
}VOLTE_STATE;
//END

typedef enum 
{
  QL_NW_ACCESS_TECH_GSM   = 0,
  QL_NW_ACCESS_TECH_GSM_COMPACT    = 1,
  QL_NW_ACCESS_TECH_UTRAN      = 2,
  QL_NW_ACCESS_TECH_GSM_wEGPRS    = 3,
  QL_NW_ACCESS_TECH_UTRAN_wHSDPA   = 4,
  QL_NW_ACCESS_TECH_UTRAN_wHSUPA     = 5,
  QL_NW_ACCESS_TECH_UTRAN_wHSDPA_HSUPA    = 6,
  QL_NW_ACCESS_TECH_E_UTRAN    = 7,
  QL_NW_ACCESS_TECH_UTRAN_HSPAP   = 8,
  QL_NW_ACCESS_TECH_E_UTRAN_CA   = 9,
  QL_NW_ACCESS_TECH_NONE = 10,
}QL_NW_ACCESS_TECHNOLOGY;

typedef struct 
{
  int status; //0：未知的网络；1：可用的网络；2：当前的网络；3：禁止使用的网络
  QL_NW_OPERATOR_INFO_T operator_name; //运营商信息，见QL_NW_OPERATOR_INFO_T。
  QL_NW_ACCESS_TECHNOLOGY  act;  //注网制式，见QL_NW_RADIO_TECH_TYPE_T。
}QL_NW_SCAN_ENTRY_INFO_T;

typedef struct 
{
  int  entry_len; //可用网络的个数
  QL_NW_SCAN_ENTRY_INFO_T entry[40]; //可用网络的列表，见QL_NW_SCAN_ENTRY_INFO_T。
}QL_NW_SCAN_RESULT_LIST_INFO_T; 


typedef enum {
    QL_NW_RADIO_TECH_UNKNOWN = 0,
    QL_NW_RADIO_TECH_GPRS = 1,
    QL_NW_RADIO_TECH_EDGE = 2,
    QL_NW_RADIO_TECH_UMTS = 3,
    QL_NW_RADIO_TECH_IS95A = 4,
    QL_NW_RADIO_TECH_IS95B = 5,
    QL_NW_RADIO_TECH_1xRTT =  6,
    QL_NW_RADIO_TECH_HSDPA = 9,
    QL_NW_RADIO_TECH_HSUPA = 10,
    QL_NW_RADIO_TECH_HSPA = 11,
    QL_NW_RADIO_TECH_EHRPD = 13,
    QL_NW_RADIO_TECH_LTE = 14,
    QL_NW_RADIO_TECH_HSPAP = 15, // HSPA+
    QL_NW_RADIO_TECH_GSM = 16, 
    QL_NW_RADIO_TECH_TD_SCDMA = 17,
    QL_NW_RADIO_TECH_IWLAN = 18,
    QL_NW_RADIO_TECH_LTEP = 19,
    QL_NW_RADIO_TECH_DC_HSPA = 20
} QL_NW_RADIO_TECHNOLOGY;

typedef enum {
  QL_NW_REG_STATE_NOT_REGISTERED=0,  //not registered, MT is not currently searching an operator to register to
  QL_NW_REG_STATE_HOME_NETWORK=1,  //registered, home network
  QL_NW_REG_STATE_TRYING_ATTACH_OR_SEARCHING=2, //not registered, but MT is currently trying to attach or searching an operator to register to
  QL_NW_REG_STATE_DENIED=3, //registration denied
  QL_NW_REG_STATE_UNKNOWN=4, //unknown
  QL_NW_REG_STATE_ROAMING=5, //registered, roaming
  QL_NW_REG_STATE_HOME_NETWORK_SMS_ONLY=6, //registered for “SMS only”, home network (not applicable)
  QL_NW_REG_STATE_ROAMING_SMS_ONLY=7,// registered for “SMS only”, roaming (not applicable)
  QL_NW_REG_STATE_ATTACHED_EMERGENCY_ONLY=8, //attached for emergency bearer services only 
  QL_NW_REG_STATE_HOME_NETWORK_CSFB_NOT_PREFERRED=9, //registered for “CSFB not preferred”, home network (not applicable)
  QL_NW_REG_STATE_ROAMING_CSFB_NOT_PREFERRED=10,//registered for “CSFB not preferred”, roaming (not applicable)
  QL_NW_REG_STATE_EMERGENCY_ONLY=11,//emergency bearer services only
}QL_NW_REG_STATE;

typedef struct
{
	QL_NW_REG_STATE state; //注册状态
	int lac;
	int cid;
	QL_NW_RADIO_TECHNOLOGY  rat;
	int rejectCause;  //注册被拒绝的原因
	int psc; //Primary Scrambling Code
} QL_NW_COMMON_REG_STATUS_INFO_T;

typedef struct
{
	QL_NW_COMMON_REG_STATUS_INFO_T voice_reg;  //VOICE注册信息，见QL_NW_COMMON_REG_STATUS_INFO_T
	QL_NW_COMMON_REG_STATUS_INFO_T data_reg;   //DATA注册信息。
}QL_NW_REG_STATUS_INFO_T;

typedef struct 
{
  int   nw_selection_mode; //0 自动；1手动
  char  mcc[4];
  char  mnc[4];
  QL_NW_ACCESS_TECHNOLOGY  act;  //selection 制式，见QL_NW_ACCESS_TECHNOLOGY。
}QL_NW_SELECTION_INFO_T;

typedef struct {
  int rssi;  
  int bitErrorRate;   
  int rscp;		
	int ecio;			
} QL_NW_GW_SIGNAL_STRENGTH_INFO_T;

typedef struct {
  int rssi;  
  int rsrp;   
  int rsrq;		
	int rssnr;	
	int cqi;		
} QL_NW_LTE_SIGNAL_STRENGTH_INFO_T;

typedef struct {
    QL_NW_GW_SIGNAL_STRENGTH_INFO_T   GW_SignalStrength;
    QL_NW_LTE_SIGNAL_STRENGTH_INFO_T  LTE_SignalStrength;
}QL_NW_SIGNAL_STRENGTH_INFO_T;

/* CSQ: just get rssi and ber */
typedef struct {
  int rssi;
  int bitErrorRate;
} QL_NW_CSQ_SIGNAL_STRENGTH_INFO_T;

typedef struct
{
    int flag;       /**Cell type, 0:present,1:neighbor.*/
    int cid;        /**Cell ID, (0 indicates information is not represent).*/
    int mcc;        /**This field should be ignored when cid is not present*/
    int mnc;        /**This field should be ignored when cid is not present*/
    int lac;        /**Location area code.(This field should be ignord when cid is not present). */
    int arfcn;      /**Absolute RF channel number. */
    char bsic;       /**Base station identity code. (0 indicates information is not present). */
}QL_MCM_NW_GSM_CELL_INFO_T;

typedef struct
{
    int flag;          /**Cell type, 0:present,1:inter,2:intra.*/
    int cid;           /**Cell ID, (0 indicates information is not represent).*/
    int lcid;          /**UTRAN Cell ID (0 indicates information is not present). */
    int mcc;           /**This field should be ignored when cid is not present*/
    int mnc;           /**This field should be ignored when cid is not present*/
    int lac;           /**Location area code. (This field should be ignored when cid is not present). */
    int uarfcn;        /**UTRA absolute RF channel number. */
    int psc;           /**Primary scrambling code. */
}QL_MCM_NW_UMTS_CELL_INFO_T;

typedef struct
{
    int flag;          /**Cell type, 0:present,1:inter,2:intra.*/
    int cid;           /**<Cell ID, (0 indicates information is not represent).*/
    int mcc;           /**This field should be ignored when cid is not present*/
    int mnc;           /**This field should be ignored when cid is not present*/
    int tac;           /**Tracing area code (This field should be ignored when cid is not present). */
    int pci;           /**Physical cell ID. Range: 0 to 503. */
    int earfcn;        /**E-UTRA absolute radio frequency channel number of the cell. RANGE: 0 TO 65535. */
}QL_MCM_NW_LTE_CELL_INFO_T;

/** Gets cell information. */
typedef struct 
{
    int                 gsm_info_valid;                         /**< Must be set to TRUE if gsm_info is being passed. */
    int                 gsm_info_num;                           /**< Must be set to the number of elements in entry*/
    QL_MCM_NW_GSM_CELL_INFO_T    gsm_info[CELL_MAX_NUM];  /**<   GSM cell information (Serving and neighbor. */  
    int                 umts_info_valid;                        /**< Must be set to TRUE if umts_info is being passed. */
    int                 umts_info_num;                          /**< Must be set to the number of elements in entry*/
    QL_MCM_NW_UMTS_CELL_INFO_T   umts_info[CELL_MAX_NUM];/**<   UMTS cell information (Serving and neighbor). */
    int                 lte_info_valid;                         /**< Must be set to TRUE if lte_info is being passed. */
    int                 lte_info_num;                           /**< Must be set to the number of elements in entry*/
    QL_MCM_NW_LTE_CELL_INFO_T    lte_info[CELL_MAX_NUM];  /**<   LTE cell information (Serving and neighbor). */
}QL_NW_CELL_INFO_T;


typedef void (*QL_NW_EventHandlerFunc_t)(unsigned int ind_flag, void *ind_msg_buf, unsigned int ind_msg_len, void *contextPtr);

QL_NW_ERROR_CODE ql_nw_init ();
QL_NW_ERROR_CODE ql_nw_release();
QL_NW_ERROR_CODE ql_nw_set_config(QL_NW_CONFIG_INFO_T  *pt_info);
QL_NW_ERROR_CODE ql_nw_get_config(QL_NW_CONFIG_INFO_T  *pt_info);
QL_NW_ERROR_CODE ql_nw_get_nitz_time_info(QL_NW_NITZ_TIME_INFO_T *pt_info);
QL_NW_ERROR_CODE ql_nw_event_register(unsigned int  bitmask);
QL_NW_ERROR_CODE ql_nw_get_operator_name(QL_NW_OPERATOR_INFO_T  *pt_info);
QL_NW_ERROR_CODE ql_nw_perform_scan(QL_NW_SCAN_RESULT_LIST_INFO_T   *pt_info);
QL_NW_ERROR_CODE ql_nw_get_reg_status(QL_NW_REG_STATUS_INFO_T  *pt_info);
QL_NW_ERROR_CODE ql_nw_set_selection(QL_NW_SELECTION_INFO_T  *pt_info);
QL_NW_ERROR_CODE ql_nw_get_selection(QL_NW_SELECTION_INFO_T  *pt_info);
QL_NW_ERROR_CODE ql_nw_get_signal_strength(QL_NW_SIGNAL_STRENGTH_INFO_T    *pt_info);
QL_NW_ERROR_CODE ql_nw_get_cell_info(QL_NW_CELL_INFO_T  *pt_info);
QL_NW_ERROR_CODE ql_nw_add_event_handler(QL_NW_EventHandlerFunc_t handlerPtr, void* contextPtr);
QL_NW_ERROR_CODE ql_nw_get_volte_state(VOLTE_STATE *state);
/* CSQ: just get rssi and ber */
QL_NW_ERROR_CODE ql_nw_csq_get_signal_strength(QL_NW_CSQ_SIGNAL_STRENGTH_INFO_T    *pt_info);
#ifdef __cplusplus
}
#endif

#endif
