/**  
  @file
  ql_spi.h

  @brief
  This file provides the definitions for spi driver, and declares the 
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
2019/08/20  Juson          create
=============================================================================*/
#ifndef __QL_SPI_H__
#define __QL_SPI_H__

#define SPI_CPHA		0x01
#define SPI_CPOL		0x02

typedef enum
{
	SPIMODE0  =   (0|0),
	SPIMODE1  =   (0|SPI_CPHA),
	SPIMODE2  =   (SPI_CPOL|0),
	SPIMODE3  =   (SPI_CPOL|SPI_CPHA),
}SPI_MODE;

typedef enum
{
	S_6_5M  =   6500000,
	S_13M 	=   13000000,
	S_26M  	=   26000000,
	S_52M  	=   50000000,
}SPI_SPEED;

/**
 * Function:  Ql_SPI_Init
 * Description: spi init
 * Parameters:  dev_name---device name
 *              mode---spi mode
 *              bits---spi per word
 *              speed---spi transfer clock
 * Return: spi fd
 **/ 
int Ql_SPI_Init(const char *dev_name, SPI_MODE mode, unsigned char bits, SPI_SPEED speed);

/**
 * Function: Ql_SPI_DeInit
 * Description: spi deinit
 * Parameters: fd---spi fd
 * Return:
 */ 
int Ql_SPI_DeInit(int fd);

/**
 * Function: Ql_SPI_Write_Read
 * Description: spi write read function
 * Parameters: fd---spi fd
 *              w_buf---write buffer
 *              r_buf---read buffer
 *              len---spi transfer length
 * Return: 0---transfer success
 *          other---failed
 **/
int Ql_SPI_Write_Read(int fd, unsigned char *w_buf, unsigned char *r_buf, unsigned int len);

#endif/* __QL_SPI_H__ */