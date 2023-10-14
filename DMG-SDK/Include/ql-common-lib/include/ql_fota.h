/**  
  @file
  ql_fota.h

  @brief
  This file provides the definitions for device, and declares the 
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
08/11/2019  baron.qian      create
=============================================================================*/
#ifndef __QL_FOTA_H__
#define __QL_FOTA_H__

#ifdef __cplusplus
extern "C" {
#endif

enum {
    QUEC_FOTA_OK,           /*OK*/
    QUEC_FOTA_FAILED        /*failed*/
}QL_FOTA_STATUS;

/*callback function define, used to get upgrade state and rate of process*/
typedef int(*fota_callback)(int state, int percent);

/*******************************************************************************
* @brief fota initialize
  @param 
    
    fota_callback: callback function
  @return 
        if success return 0, else return -1
 *******************************************************************************/
int ql_fota_init(fota_callback cb);

/*******************************************************************************
* @brief write firmware package, the firmware package is written in segments.
         and The result of the write is output by calling the callback function.
         the firmware package size must less than 32MB
  @param 
           fname: firmware package file
    segment_size: the length of once write, recommending 3*1024*1024 bytes
  @return 
        if success return 0, else return -1
 *******************************************************************************/
int ql_fota_fw_write(char *fname, int segment_size);


/*******************************************************************************
* @brief download firmware by url, and write firmware package, the firmware 
         package is written in segments. The result of the write is output by 
         calling the callback function. the firmware package size must less than 
         32MB
  @param 
                 url: [IN] the address of download firmware package file, the url   
                           support http or https protocol.
        segment_size: [IN] the length of once write, recommending 3*1024*1024 bytes
        conn_timeout: [IN] timeout to connect to the server, if set 0 that means
                           switch to the default build-in connection timeout(300s)
    download_timeout: [IN] timeout for download the firmware file. if set 0 that means
                           it never time out
  @return 
        if success return 0, else return -1
 *******************************************************************************/
int ql_fota_fw_write_by_url(char *url, int segment_size, int conn_timeout, int download_timeout);


/*******************************************************************************
* @brief reboot system and clear env
  @param 
   is_reboot: if set 1, after fota success, reboot system
  @return 
        if success return 0, else return -1
 *******************************************************************************/
int ql_fota_done(int is_reboot);

#ifdef __cplusplus
}
#endif
#endif //__QL_FOTA_H__
