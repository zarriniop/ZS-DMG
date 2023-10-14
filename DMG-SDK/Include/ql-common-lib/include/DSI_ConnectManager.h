/**  
  @file
  DSI_ConnectManager.h

  @brief
  This file provides the definitions for dsi, and declares the 
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
24/07/2019  Nebula.li      create
=============================================================================*/

#ifndef __DSI_CONNECTMANAGER_H__
#define __DSI_CONNECTMANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define QL_WAN_SUCCESS                      (0)
#define QL_WAN_ERROR                        (-1)
#define QL_INTERFACE_NAME_LEN               16
#define QL_IPV4_MAX_ADDR_LEN                32
#define QL_IPV6_MAX_ADDR_LEN                128


/**
 * used to check network status, when timeout, start data call again.
*/
#define QL_WAN_NETWORK_STATUS_MAX_TIMEOUT   (60)


/**
 * DSI event define
 */ 
typedef enum
{
  CONNECT_DISCON = 0x00,
  CONNECT_CONSUCCESS,
  CONNECT_CONNING,
  CONNECT_REDIAL,
  CONNECT_DIAL_IMMEDIATELY,
  CONNECT_ROAM,
  CONNECT_WAIT_NWCHANGE,
  CONNECT_GET_GLOBALIP_ERR,
  CONNECT_DEACTIVATED,
  CONNECT_WAIT_PS_ATTACH,

  FAIL_BASE_INDEX=0x10,
  SIM_NOT_READY=FAIL_BASE_INDEX+0x01,
  NW_NOT_READY=FAIL_BASE_INDEX+0x02,
  PDP_OPERATOR_BARRED =FAIL_BASE_INDEX+0x08,               /* no retry */
  PDP_INSUFFICIENT_RESOURCES =FAIL_BASE_INDEX+0x1A,
  PDP_MISSING_UKNOWN_APN = FAIL_BASE_INDEX+0x1B,            /* no retry */
  PDP_UNKNOWN_PDP_ADDRESS_TYPE = FAIL_BASE_INDEX+0x1C,      /* no retry */
  PDP_USER_AUTHENTICATION = FAIL_BASE_INDEX+0x1D,           /* no retry */
  PDP_ACTIVATION_REJECT_GGSN = FAIL_BASE_INDEX+0x1E,        /* no retry */
  PDP_ACTIVATION_REJECT_UNSPECIFIED = FAIL_BASE_INDEX+0x1F,
  PDP_SERVICE_OPTION_NOT_SUPPORTED = FAIL_BASE_INDEX+0x20,  /* no retry */
  PDP_SERVICE_OPTION_NOT_SUBSCRIBED = FAIL_BASE_INDEX+0x21, /* no retry */
  PDP_SERVICE_OPTION_OUT_OF_ORDER = FAIL_BASE_INDEX+0x22,
  PDP_NSAPI_IN_USE = FAIL_BASE_INDEX+0x23,                  /* no retry */
  PDP_REGULAR_DEACTIVATION = FAIL_BASE_INDEX+0x24,          /* possibly restart radio,
                                                    based on framework config */
  PDP_ONLY_IPV4_ALLOWED = FAIL_BASE_INDEX+0x32,             /* no retry */
  PDP_ONLY_IPV6_ALLOWED = FAIL_BASE_INDEX+0x33,             /* no retry */
  PDP_ONLY_SINGLE_BEARER_ALLOWED = FAIL_BASE_INDEX+0x34,
  PDP_PROTOCOL_ERRORS   = FAIL_BASE_INDEX+0x6F,             /* no retry */

  /* Not mentioned in the specification */
  PDP_VOICE_REGISTRATION_FAIL = -1,
  PDP_DATA_REGISTRATION_FAIL = -2,

  /* reasons for data call drop - network/modem disconnect */
  PDP_SIGNAL_LOST = -3,
  PDP_PREF_RADIO_TECH_CHANGED = -4,/* preferred technology has changed, should retry
                                            with parameters appropriate for new technology */
  PDP_RADIO_POWER_OFF = -5,        /* data call was disconnected because radio was resetting,
                                            powered off - no retry */
  PDP_TETHERED_CALL_ACTIVE = -6,   /* data call was disconnected by modem because tethered
                                            mode was up on same APN/data profile - no retry until
                                            tethered call is off */
	PDP_CID_EXIST_FAIL = -7,		  /* cid is exist already*/

  PDP_NO_CCINET=-10,
  PDP_DHCP_SERVER_ERR_PIPE=-11,

  PDP_ERROR_UNSPECIFIED = 0xffff,  /* retry silently */
  QL_DSI_EVT_MAX
} QL_DSI_NET_EVT_T;

#define SIM_REMOVED SIM_NOT_READY

typedef struct
{
	char ip[QL_IPV4_MAX_ADDR_LEN];
	char pri_dns[QL_IPV4_MAX_ADDR_LEN];
	char sec_dns[QL_IPV4_MAX_ADDR_LEN];
  char name[QL_INTERFACE_NAME_LEN];
  char gateway[QL_IPV4_MAX_ADDR_LEN];
}v4_address_status;

typedef struct
{
	int state; //dial status
	v4_address_status addr; //IPv4 address information
}v4_info;

typedef struct
{
	char ip[QL_IPV6_MAX_ADDR_LEN];
	char pri_dns[QL_IPV6_MAX_ADDR_LEN];
	char sec_dns[QL_IPV6_MAX_ADDR_LEN];
  char name[QL_INTERFACE_NAME_LEN];
  char gateway[QL_IPV6_MAX_ADDR_LEN];
}v6_address_status;

typedef struct
{
	int state; //dial status
	v6_address_status addr; //IPv6 address information
}v6_info;

typedef struct
{
	int profile_idx;
	int ip_type;
	v4_info v4;
	v6_info v6;
}ql_data_call_info;

/**
 * DSI auth pref define
*/
typedef enum
{
  QL_DSI_AUTH_PREF_NULL = 0,
  QL_DSI_AUTH_PREF_ONLY_PAP,
  QL_DSI_AUTH_PREF_ONLY_CHAP,
  //QL_DSI_AUTH_PREF_BOTH_PAP_CHAP
} QL_DSI_AUTH_PREF_T;

/** 
 * callback function used in  ql_wan_start, when data call status changed, 
 * Use callback functions for notifications, see enum QL_DSI_NET_EVT_T
*/
typedef void (*nw_status_cb)(int status);


typedef void (*ex_conn_status_cb)(int cid,int iptype,int status,int cause);



/*-----------------------------------------------------------------------------------------------*/
/**
  @brief data call init function, must call first.
  @return if success return QL_WAN_SUCCESS, else return QL_WAN_ERROR
  */
/*-----------------------------------------------------------------------------------------------*/
int ql_wan_init();

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief data call release function, must call finally.
  @return if success return QL_WAN_SUCCESS, else return QL_WAN_ERROR
  */
/*-----------------------------------------------------------------------------------------------*/
int ql_wan_release();


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief data call auto connect set, include auto status and interval(seconds).
  @param[in] auto_status  When it is 0, the network cannot be automatically reconnected 
                                after disconnection; when it is 1, it can be automatically reconnected
  @param[in] interval_sec data call status check interval
                                the max value is QL_WAN_NETWORK_STATUS_MAX_TIMEOUT
  @return if success return QL_WAN_SUCCESS, else return QL_WAN_ERROR
  */
/*-----------------------------------------------------------------------------------------------*/
int ql_wan_set_autoconnect(int auto_status, int interval_sec);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief start data call, must call ql_wan_init first.
  @param[in] nw_cb this is a callback function, when data call status changed, Use callback 
                   functions for notifications
  @return if success return QL_WAN_SUCCESS, else return QL_WAN_ERROR
  */
/*-----------------------------------------------------------------------------------------------*/
int ql_wan_start(int profile_idx, int op, nw_status_cb nw_cb);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief start data call, must call ql_wan_init first.
  @param[in] nw_cb this is a callback function, when data call status changed, Use callback 
                   functions for notifications
  @return if success return QL_WAN_SUCCESS, else return QL_WAN_ERROR
  */
/*-----------------------------------------------------------------------------------------------*/
int ql_wan_start_ex(int profile_idx, int op, ex_conn_status_cb nw_cb);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief data call stop function.
  @return if success return QL_WAN_SUCCESS, else return QL_WAN_ERROR
  */
/*-----------------------------------------------------------------------------------------------*/
int ql_wan_stop(int profile_idx);


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief function to set apn.
  @param[in] apn        apn name want to set
  @param[in] username   user name of apn
  @param[in] password   password of apn
  @param[in] auth       auth type of apn, see enum QL_DSI_AUTH_PREF_T
  @return if success return QL_WAN_SUCCESS, else return QL_WAN_ERROR
  */
/*-----------------------------------------------------------------------------------------------*/
int ql_wan_setapn(int profile_idx, int ip_type, const char *apn, const char *userName, const char *password, QL_DSI_AUTH_PREF_T auth);
/*-----------------------------------------------------------------------------------------------*/
/**
  @brief function to get first apn infomation.
  @param[out] apn       apn name to return
  @param[in ] apnLen    apn name buffer size
  @param[out] userName  username of apn to return
  @param[in ] userLen   username buffer size
  @param[out] password  password to return
  @param[in ] pwdLen    password buffer size 
  @return if success return QL_WAN_SUCCESS, else return QL_WAN_ERROR
  */
/*-----------------------------------------------------------------------------------------------*/
int ql_wan_getapn(int profile_idx, int *ip_type, char *apn, int apnLen, char *userName, int userLen, char *password, int pwdLen);
int ql_get_data_call_info(int profile_idx, ql_data_call_info *info);

#ifdef __cplusplus
}
#endif

#endif
