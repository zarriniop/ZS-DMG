/**  
  @file
  ql_oe.h

  @brief
  Include related header file for quectel common api.

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
03/08/2019  Juson.zhang    add spi and uart head file
03/08/2019  Carola.zhang   add i2c and gpio head file
=============================================================================*/

#ifndef _QL_OE_H_
#define _QL_OE_H_

#include "DSI_ConnectManager.h"
#include "ql_log.h"

//-------------------------peripheral head file-------------------//
#include "ql_uart.h"
#include "ql_spi.h"
#include "ql_gpio.h"
#include "ql_i2c.h"
#include "ql_sms.h"

#endif
