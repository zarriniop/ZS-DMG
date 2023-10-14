/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2019
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   ql_gpio.h 
 *
 * Project:
 * --------
 *   OpenLinux
 *
 * Description:
 * ------------
 *   GPIO API defines.
 *
 *
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * WHO            WHEN                WHAT
 *----------------------------------------------------------------------------
 * Carola.Zhang   16/07/2019		  Create.
 ****************************************************************************/

#ifndef __QL_GPIO_H__
#define __QL_GPIO_H__

typedef enum{
    /*Invalid*/  PINNAME_BEGIN = -1,
	/*PIN-1*/    PINNAME_GPIO1 = 1,
	/*PIN-2*/    PINNAME_GPIO2 = 2,
	/*PIN-3*/    PINNAME_GPIO3 = 3,
	/*PIN-4*/    PINNAME_GPIO4 = 4,
	/*PIN-5*/    PINNAME_GPIO5 = 5,
    /*PIN-6*/    PINNAME_NET_STATUS = 6,
    /*PIN-11*/   PINNAME_DBG_RXD = 11,
    /*PIN-12*/   PINNAME_DBG_TXD = 12,
    /*PIN-13*/   PINNAME_USIM_PRESENCE = 13,
    /*PIN-23*/   PINNAME_SD_INT_DET = 23,
    /*PIN-24*/   PINNAME_PCM_IN = 24,
    /*PIN-25*/   PINNAME_PCM_OUT = 25,
    /*PIN-26*/   PINNAME_PCM_SYNC = 26,
    /*PIN-27*/   PINNAME_PCM_CLK = 27,
    /*PIN-28*/   PINNAME_SDC2_DATA3 = 28,
    /*PIN-29*/   PINNAME_SDC2_DATA2 = 29,
    /*PIN-30*/   PINNAME_SDC2_DATA1 = 30,
    /*PIN-31*/   PINNAME_SDC2_DATA0 = 31,
    /*PIN-32*/   PINNAME_SDC2_CLK = 32,
    /*PIN-33*/   PINNAME_SDC2_CMD = 33,
    /*PIN-37*/   PINNAME_SPI_CS_N = 37,
    /*PIN-38*/   PINNAME_SPI_MOSI = 38,
    /*PIN-39*/   PINNAME_SPI_MISO = 39,
    /*PIN-40*/   PINNAME_SPI_CLK = 40,
    /*PIN-41*/   PINNAME_I2C_SCL = 41,
    /*PIN-42*/   PINNAME_I2C_SDA = 42,
    /*PIN-62*/   PINNAME_GPIO6 = 62,
    /*PIN-63*/   PINNAME_DCD = 63,
    /*PIN-66*/   PINNAME_DTR = 66,
    /*PIN-64*/   PINNAME_MAIN_CTS = 64,
    /*PIN-65*/   PINNAME_MAIN_RTS = 65,
    /*PIN-67*/   PINNAME_MAIN_TXD = 67,
    /*PIN-68*/   PINNAME_MAIN_RXD = 68,
    /*PIN-73*/   PINNAME_RMII_RXD1 = 73,
    /*PIN-74*/   PINNAME_RMII_RXCL = 74,
    /*PIN-75*/   PINNAME_RMII_CLK = 75,
    /*PIN-76*/   PINNAME_RMII_RXD0 = 76,
    /*PIN-77*/   PINNAME_RMII_TXD0 = 77,
    /*PIN-78*/   PINNAME_RMII_TXD1 = 78,
    /*PIN-79*/   PINNAME_RMII_RXD2 = 79,
    /*PIN-80*/   PINNAME_RMII_TXD2 = 80,
    /*PIN-81*/   PINNAME_RMII_TX_CTRL = 81,
    /*PIN-82*/   PINNAME_RMII_RXD3 = 82,
    /*PIN-83*/   PINNAME_RMII_TXCL = 83,
    /*PIN-84*/   PINNAME_RMII_TXD3 = 84,
    /*PIN-118*/  PINNAME_WLAN_SLP_CLK = 118,
    /*PIN-119*/  PINNAME_RMII_RST = 119,
    /*PIN-120*/  PINNAME_RMII_INT = 120,
    /*PIN-121*/  PINNAME_RMII_MDIO = 121,
    /*PIN-122*/  PINNAME_RMII_MDC = 122,
    /*PIN-127*/  PINNAME_WLAN_PER_EN = 127,
    /*PIN-135*/  PINNAME_WLAN_WAKE = 135,
    /*PIN-136*/  PINNAME_WLAN_EN = 136,
    /*PIN-139*/  PINNAME_GPIO8 = 139,
    PINNAME_END
}Enum_PinName;

/****************************************************************************
 * Error Code Definition
 ***************************************************************************/
enum {
        RES_OK = 0,
        RES_BAD_PARAMETER  = -1,     ///< Parameter is invalid.
        RES_IO_NOT_SUPPORT = -2,
        RES_IO_ERROR = -3,
        RES_NOT_IMPLEMENTED = -4
};

typedef enum{
    PINDIRECTION_IN  = 0,
    PINDIRECTION_OUT = 1
}Enum_PinDirection;

typedef enum{
    PINLEVEL_LOW  = 0,
    PINLEVEL_HIGH = 1
}Enum_PinLevel;

typedef enum{
    PINPULLSEL_DISABLE  = (0<<13),    // Disable pull selection
    PINPULLSEL_PULLDOWN = (5<<13),    // Pull-down 
    PINPULLSEL_PULLUP   = (6<<13)     // Pull-up 
}Enum_PinPullSel;

/****************************************************************************
 * GPIO Config Items
 ***************************************************************************/
typedef struct{
    Enum_PinName           pin_name;
    Enum_PinDirection      pinDirection;  
    Enum_PinLevel          pinLevel;
    Enum_PinPullSel       pinPullSel;
}ST_GPIOConfig;

//------------------------------------------------------------------------------
/**
 * The type of GPIO Edge Sensivity.
 */
//------------------------------------------------------------------------------
typedef enum {
    EINT_SENSE_NONE,       // pin is input, but no an interrupt pin.
    EINT_SENSE_RISING,
    EINT_SENSE_FALLING,
    EINT_SENSE_BOTH
}Enum_EintType;

/*****************************************************************
* Function:     Ql_GPIO_Init 
* 
* Description:
*               This function enables the GPIO function of the specified pin,
*               and initialize the configurations, including direction,
*               level and pull selection.
*
* Parameters:
*               pin_name:
*                   Pin name, one value of Enum_PinName.
*               dir:
*                   The initial direction of GPIO, one value of Enum_PinDirection.
*               level:
*                   The initial level of GPIO, one value of Enum_PinLevel. 
*               pull_sel:
*                   Pull selection, one value of Enum_PinPullSel.
* Return:        
*               RES_OK, this function succeeds.
*               RES_IO_NOT_SUPPORT, the input GPIO is invalid. 
*               RES_IO_ERR, the function failed
*               other place. For example this GPIO has been using as EINT.
*****************************************************************/
int Ql_GPIO_Init(Enum_PinName       pin_name, 
                 Enum_PinDirection  dir, 
                 Enum_PinLevel      level, 
                 Enum_PinPullSel    pull_sel
                 );

/*****************************************************************
* Function:     Ql_GPIO_Base_Init 
* 
* Description:
*               This function enables the GPIO function of the specified pin.
*
* Parameters:
*               pin_name:
*                   Pin name, one value of Enum_PinName.
*            
* Return:        
*               RES_OK, this function succeeds.
*               RES_IO_NOT_SUPPORT, the input GPIO is invalid. 
*               RES_IO_ERR, the function failed
*****************************************************************/
int Ql_GPIO_Base_Init(Enum_PinName pin_name );

/*****************************************************************
* Function:     Ql_GPIO_SetLevel 
* 
* Description:
*               This function sets the level of the specified GPIO.
*
* Parameters:
*               pin_name:
*                   Pin name, one value of Enum_PinName.
*               level:
*                   The initial level of GPIO, one value of Enum_PinLevel. 
* Return:        
*               RES_OK, this function succeeds.
*               RES_IO_NOT_SUPPORT, the input GPIO is invalid. 
*               RES_IO_ERR, the function failed
*               other place. For example this GPIO has been using as EINT.
*****************************************************************/
int Ql_GPIO_SetLevel(Enum_PinName pin_name, Enum_PinLevel level);

/*****************************************************************
* Function:     Ql_GPIO_GetLevel 
* 
* Description:
*               This function gets the level of the specified GPIO.
*
* Parameters:
*               pin_name:
*                   Pin name, one value of Enum_PinName.
* Return:        
*               The level value of the specified GPIO, which is 
*               nonnegative integer.
*               RES_IO_NOT_SUPPORT, the input GPIO is invalid.
*****************************************************************/
int Ql_GPIO_GetLevel(Enum_PinName pin_name);

/*****************************************************************
* Function:     Ql_GPIO_SetDirection 
* 
* Description:
*               This function sets the direction of the specified GPIO.
*
* Parameters:
*               pin_name:
*                   Pin name, one value of Enum_PinName.
*               dir:
*                   The initial direction of GPIO, one value of Enum_PinDirection.
* Return:        
*               RES_OK, this function succeeds.
*               RES_IO_NOT_SUPPORT, the input GPIO is invalid. 
*               RES_IO_ERR, the function failed
*               other place. For example this GPIO has been using as EINT.
*****************************************************************/
int Ql_GPIO_SetDirection(Enum_PinName pin_name, Enum_PinDirection dir);

/*****************************************************************
* Function:     Ql_GPIO_GetDirection 
* 
* Description:
*               This function gets the direction of the specified GPIO.
*
* Parameters:
*               pin_name:
*                   Pin name, one value of Enum_PinName.
* Return:        
*               0  INPUT
*               1  OUTPUT
*               RES_IO_NOT_SUPPORT, the input GPIO is invalid. 
*               other place. For example this GPIO has been using as EINT.
*****************************************************************/
int Ql_GPIO_GetDirection(Enum_PinName pin_name);

/*****************************************************************
* Function:     Ql_GPIO_SetPullSelection 
* 
* Description:
*               This function sets the pull selection of the specified GPIO.
*
* Parameters:
*               pin_name:
*                   Pin name, one value of Enum_PinName.
*               Enum_PinPullSel:
*                   Pull selection, one value of Enum_PinPullSel.
* Return:        
*               RES_OK, this function succeeds.
*               RES_IO_NOT_SUPPORT, the input GPIO is invalid. 
*               RES_IO_ERR, the function failed
*               other place. For example this GPIO has been using as EINT.
*****************************************************************/
int Ql_GPIO_SetPullSelection(Enum_PinName pin_name, Enum_PinPullSel pull_sel);

/*****************************************************************
* Function:     ql_gpio_get_pull_selection 
* 
* Description:
*               This function gets the pull selection of the specified GPIO.
*
* Parameters:
*               pin_name:
*                   Pin name, one value of Enum_PinName.
* Return:        
*               0<<13   no pull
*               5<<13   pull down
*               6<<13   pull up
*****************************************************************/
int Ql_GPIO_GetPullSelection(Enum_PinName pin_name);

/*****************************************************************
* Function:     Ql_GPIO_Uninit 
* 
* Description:
*               This function releases the specified GPIO that was 
*               initialized by calling Ql_GPIO_Init() previously.
*               After releasing, the GPIO can be used for other purpose.
* Parameters:
*               pin_name:
*                   Pin name, one value of Enum_PinName.
* Return:        
*               RES_OK, this function succeeds.
*               RES_IO_NOT_SUPPORT, the input GPIO is invalid. 
*               RES_IO_ERR, the function failed
*               other place. For example this GPIO has been using as EINT.
*****************************************************************/
int Ql_GPIO_Uninit(Enum_PinName pin_name);

//------------------------------------------------------------------------------
/*
* Description:
*     Definition for EINT callback function.
* 
* Parameters:
*     PinName:
*         EINT pin name, one value of Enum_PinName.
*
*     level:
*         The EINT level value, one value of Enum_PinLevel. 
*         0 or 1
*/
//------------------------------------------------------------------------------
typedef void (*Ql_EINT_Callback)(Enum_PinName eint_pin_name, int level);

//------------------------------------------------------------------------------
/*
* Function:     Ql_EINT_Enable 
* 
* Description:
*               Set the interrupt sense mode, and enable interrupt. 
*
* Parameters:
*               eint_pin_name:
*                   EINT pin name, one value of Enum_PinName that has 
*                   the interrupt function.
*
*               eint_type:
*                   Interrupt type, level-triggered or edge-triggered.
*                   Now, only edge-triggered interrupt is supported.
*
*               eint_callback:
*                   call back function
*
* Return:        
*               RES_OK, this function succeeds.
*               else failed to execute the function. 
*/
//------------------------------------------------------------------------------
int Ql_EINT_Enable(Enum_PinName eint_pin_name, Enum_EintType eint_type, Ql_EINT_Callback eint_callback);


//------------------------------------------------------------------------------
/*
* Function:     Ql_EINT_Disable 
* 
* Description:
*               Disable the interrupt sense. 
*
* Parameters:
*               eint_pin_name:
*                   EINT pin name, one value of Enum_PinName that has 
*                   the interrupt function.
*
* Return:        
*               RES_OK, this function succeeds.
*               else failed to execute the function. 
*/
//------------------------------------------------------------------------------
int Ql_EINT_Disable(Enum_PinName eint_pin_name);

#endif  // __QL_GPIO_H__
