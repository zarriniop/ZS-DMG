
/**  
  @file
  ql_common.h

  @brief
  This file provides the definitions for common functions, and declares the 
  common functions.

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
23/08/2019  Nebula.li      create
=============================================================================*/

#ifndef __QL_COMMON_H__
#define __QL_COMMON_H__

#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <libubox/ustream.h>
#include <libubox/blobmsg_json.h>
#include <libubox/kvlist.h>
#include "libubus.h"
#include "uci_blob.h"
#include "libubox/uloop.h"
#include "rilutil.h"

#ifdef __cplusplus
extern "C" {
#endif

int ql_start_main_thread();
int ql_exit_main_thread();
int ql_ril_sync_request(unsigned int requestid, const void * data, int len,void* priv,void * cb,int timeout);
int ql_ril_async_request(unsigned int requestid, const void * data, int len,void* priv,void * cb);
int ql_ubus_sync_request(char *obj_name, char *method,struct blob_buf *blob_buf,void* priv,void *cb,int timeout);
int ql_ubus_async_request(char *obj_name, char *method,struct blob_buf *blob_buf,void* priv,void *cb);
int ql_ubus_subscribe_reg(char *subscriber_name, struct ubus_subscriber *event_obj);
int ql_ubus_subscribe_remove(char *subscriber_name, struct ubus_subscriber *event_obj);

int atcmd_get_line_num(char * result, int result_len);
char * atcmd_get_line(char * result, int result_len,int index);
int atcmd_get_para_num(char * line);
void atcmd_get_para_value_str(char * line,int index, char *para_value);
void ql_set_response(struct kvlist *result_list,int reqid, int resp_type, int ret, void *data, int data_len);
int ql_get_response(struct kvlist *result_list,int reqid, int * resp_type, void *data);
int kvlist_get_len(struct kvlist *kv, const void *data);

int utf8_to_usc2(unsigned char *utf8_data,int utf8_data_len, unsigned int *usc2_data);
int usc2_to_utf8(unsigned int *usc2_data, int usc2_data_len, unsigned char *utf8_data);

int set_config_option(const char *package, const char *section, const char *option, const char *value);
char *get_config_option(char *package, char *section, char *option);

#ifdef __cplusplus
}
#endif

#endif