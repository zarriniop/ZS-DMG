/**  
  @file
  ql_dev.h

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
11/10/2019  Nebula.li      create
=============================================================================*/

#ifndef __QL_DEV_H__
#define __QL_DEV_H__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
	QL_DEV_SUCCESS,
	QL_DEV_GENERIC_FAILURE,
 	QL_DEV_RADIO_NOT_AVAILABLE,
}QL_DEV_ERROR_CODE;

typedef enum 
{
	QL_DEV_MODEM_MIN_FUN,
	QL_DEV_MODEM_FULL_FUN,
	QL_DEV_MODEM_DISABLE_RECEIVE_RF_CIRCUITS = 3,
	QL_DEV_MODEM_DISABLE_TRANSMIT_AND_RECEIVE_RF_CIRCUITS,
	QL_DEV_MODEM_DISABLE_SIM,
	QL_DEV_MODEM_TURN_OFF_FULL_SECONDARY_RECEIVE,
}QL_DEV_MODEM_FUNCTION;

QL_DEV_ERROR_CODE ql_dev_init ();
QL_DEV_ERROR_CODE ql_dev_release();

/*
    Get the IMEI string.
     -imei      : input buffer
*/
QL_DEV_ERROR_CODE ql_dev_get_imei(char* imei);


/*
    Get the FirmwareVersion string.
     -version   : input buffer
*/
QL_DEV_ERROR_CODE ql_dev_get_firmware_version(char* version);


/*
    Get the DeviceModel string.
     -model     : input buffer
*/
QL_DEV_ERROR_CODE ql_dev_get_model(char* model);


/*
    Get the SN string.
     -sn       : input buffer
*/
QL_DEV_ERROR_CODE ql_dev_get_sn(char* sn);

/*
    Set the modem function.
     -function       :[in]   modem function
     -rst    :[in]    if it is to reset modem before setting modem function. (0: not to reset; 1: reset)
*/
QL_DEV_ERROR_CODE ql_dev_set_modem_fun(QL_DEV_MODEM_FUNCTION function,int rst);

/*
    Get the modem function.
     -function    :[out]  modem function
*/
QL_DEV_ERROR_CODE ql_dev_get_modem_fun(int *function);

#ifdef __cplusplus
}
#endif

#endif
