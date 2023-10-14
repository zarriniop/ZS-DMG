/**  
  @file
  DSI_ConnectManager.h

  @brief
  This file provides the definitions for quectel log api, and declares the 
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

#ifndef _QL_LOG_H_
#define _QL_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	QL_LOG_MIN = -1,
	QL_LOG_EMERG_LEVEL=0,     /* system is unusable */
	QL_LOG_INIT_LEVEL=0,      /* log init level */
	QL_LOG_ALERT_LEVEL=1,     /* action must be taken immediately */
	QL_LOG_CRIT_LEVEL=2,      /* critical conditions */
	QL_LOG_ERR_LEVEL=3,      /* error conditions */
	QL_LOG_WARN_LEVEL=4,   /* warning conditions */
	QL_LOG_NOTICE_LEVEL=5,    /* normal but significant condition */
	QL_LOG_INFO_LEVEL=6,      /* informational */
	QL_LOG_DEBUG_LEVEL=7,     /* debug-level messages */
	QL_LOG_MAX
} QL_LOG_PRIORITY_E;

#define QL_LOG_PRORIFY_VERIFY(prio) ((prio)>QL_LOG_MIN && (prio)<QL_LOG_MAX)

typedef enum {
    QL_LOG_ID_MIN = -1,
   	QL_LOG_ID_MAIN = 0,
    QL_LOG_ID_RADIO = 1,
    QL_LOG_ID_EVENTS = 2,
    QL_LOG_ID_SYSTEM = 3,
    QL_LOG_ID_KMSG = 4,
	QL_LOG_ID_STDOUT=5,
    QL_LOG_ID_MAX
} QL_LOG_ID_E;

#define QL_LOG_ID_VERIFY(log_id)    ((log_id)>QL_LOG_ID_MIN && (log_id)<QL_LOG_ID_MAX)

#define LOG_BUF_SIZE 2048

#define QL_LOG_OPEN() ql_log_open()
#define QL_LOG_CLOSE() ql_log_close()
#define QL_LOG_SETPOS(log_id) ql_log_setpos(log_id)

#define QL_LOG_DEBUG(fmt, ...) ql_log_print(QL_LOG_DEBUG_LEVEL,__FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define QL_LOG_INFO(fmt, ...) ql_log_print(QL_LOG_INFO_LEVEL,__FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define QL_LOG_WARN(fmt, ...) ql_log_print(QL_LOG_WARN_LEVEL,__FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define QL_LOG_ERROR(fmt, ...) ql_log_print(QL_LOG_ERR_LEVEL,__FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)


extern void ql_log_open();
extern void ql_log_close();
extern int ql_log_print(QL_LOG_PRIORITY_E prio,const char *func, int line,  const char *fmt, ...);




#ifdef __cplusplus
}
#endif

#endif