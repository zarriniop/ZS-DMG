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
 *   ql_adc.h 
 *
 * Project:
 * --------
 *   OpenLinux
 *
 * Description:
 * ------------
 *   ADC API defines.
 *
 *
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * WHO            WHEN                WHAT
 *----------------------------------------------------------------------------
 * Carola.Zhang   20/11/2019		  Create.
 ****************************************************************************/

#ifndef __QL_ADC_H__
#define __QL_ADC_H__

typedef enum {
    QADC_NONE = -1,
    ADC0 = 0,
    ADC1 = 1,
    QADC_END
}Enum_QADC;


int ql_adc_show(Enum_QADC qadc);


#endif