/**
  @file
  ql_atc.h
  @brief
  send at command api

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
06/07/2022  Wayen.xu      create
=============================================================================*/

#ifndef QL_ATC_H
#define QL_ATC_H

/*******************************************************
* @method: ql_atc_init
* @Description: initialize environment variables.                
* @param: void
* @return: 
         success: return 0
         error: return !0
********************************************************/
int ql_atc_init();

/*******************************************************
* @method: ql_atc_release
* @Description: release environment variables.                
* @param: void
* @return: 
         success: return 0
         error: return !0
********************************************************/
int ql_atc_release();

/*******************************************************
* @method: ql_atc_send
* @Description: send at command(synchronous)
                use this api should first call init api and finish call release api                
* @param:
         cmd: at command
         resp: at return value
         len: response value length
* @return: 
         success: return 0
         error: return !0
********************************************************/
int ql_atc_send(char* cmd, char* resp, int resp_len);


#endif
