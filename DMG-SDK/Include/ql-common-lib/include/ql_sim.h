/**  
  @file
  ql_sim.h

  @brief
  This file provides the definitions for sim, and declares the 
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
21/08/2019  Nebula.li      create
=============================================================================*/

#ifndef __QL_SIM_H__
#define __QL_SIM_H__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char UINT8;
typedef unsigned short UINT16;
//typedef unsigned long UINT32;
typedef unsigned int UINT;

typedef enum 
{
	QL_SIM_SUCCESS,
	QL_SIM_GENERIC_FAILURE,
 	QL_SIM_RADIO_NOT_AVAILABLE,
    QL_SIM_INCORRECT_PASSWORD,
    QL_SIM_CARD_NO_INSERTED,
    QL_SIM_CARD_BUSY,
    QL_SIM_CARD_BLOCKED,
}QL_SIM_ERROR_CODE;


#define QL_SIM_MCC_LEN      4   /**  Length of the MCC. */
#define QL_SIM_MNC_MAX      4   /**  Maximum length of the MNC. */
#define QL_SIM_PLMN_NUM_MAX 24  /**  Maximum number of PLMN data sets. */
typedef struct 
{
    uint8_t mcc[QL_SIM_MCC_LEN];  /**< MCC value in ASCII characters.*/
    uint8_t mnc[QL_SIM_MNC_MAX];  /**< MNC value in ASCII characters.*/
}QL_SIM_PLMN_INFO;  
typedef struct 
{
    uint32_t            preferred_operator_list_num;                    /**< Must be set to the number of elements in preferred_operator_list. */
    QL_SIM_PLMN_INFO    preferred_operator_list[QL_SIM_PLMN_NUM_MAX];   /**< Preferred operator list. */
}QL_SIM_PREFERRED_OPERATOR_LIST;    /* Message */

#define QL_SIM_PIN_LEN_MAX  16   /**  Maximum length of PIN data. */ 
#define QUEC_CI_SIM_MAX_CMD_DATA_SIZE  261

typedef struct 
{
    uint8_t                     pin_value[QL_SIM_PIN_LEN_MAX];  /*  Value of the PIN */
}QL_SIM_VERIFY_PIN_INFO;  

typedef struct 
{
    uint8_t                     old_pin_value[QL_SIM_PIN_LEN_MAX];  /**< Value of the old PIN as a sequence of ASCII characters. */
    uint8_t                     new_pin_value[QL_SIM_PIN_LEN_MAX];  /**< Value of the new PIN as a sequence of ASCII characters. */
}QL_SIM_CHANGE_PIN_INFO;  

typedef struct 
{
    uint8_t                     puk_value[QL_SIM_PIN_LEN_MAX];      /**< Value of the PUK as a sequence of ASCII characters. */
    uint8_t                     new_pin_value[QL_SIM_PIN_LEN_MAX];  /**< Value of the new PIN as a sequence of ASCII characters. */
}QL_SIM_UNBLOCK_PIN_INFO;  

typedef enum 
{
    QL_SIM_STAT_NOT_INSERTED,
    QL_SIM_STAT_READY,
    QL_SIM_STAT_SIM_PIN,
    QL_SIM_STAT_SIM_PUK,
    QL_SIM_STAT_PH_SIM_LOCK_PIN,
    QL_SIM_STAT_PH_SIM_LOCK_PUK,
    QL_SIM_STAT_PH_FSIM_PIN,
    QL_SIM_STAT_PH_FSIM_PUK,
    QL_SIM_STAT_SIM_PIN2,
    QL_SIM_STAT_SIM_PUK2,
    QL_SIM_STAT_PH_NET_PIN,
    QL_SIM_STAT_PH_NET_PUK,
    QL_SIM_STAT_PH_NET_SUB_PIN,
    QL_SIM_STAT_PH_NET_SUB_PUK,
    QL_SIM_STAT_PH_SP_PIN,
    QL_SIM_STAT_PH_SP_PUK,
    QL_SIM_STAT_PH_CORP_PIN,
    QL_SIM_STAT_PH_CORP_PUK,
    QL_SIM_STAT_BUSY,
    QL_SIM_STAT_BLOCKED,
    QL_SIM_STAT_UNKNOWN
}QL_SIM_CARD_STATE;    /**< Card state. */
typedef enum 
{
    QL_SIM_CARD_TYPE_UNKNOWN  = 0,    /**<  Unidentified card type.  */
    QL_SIM_CARD_TYPE_ICC      = 1,    /**<  Card of SIM or RUIM type.  */
    QL_SIM_CARD_TYPE_UICC     = 2,    /**<  Card of USIM or CSIM type.  */
}QL_SIM_CARD_TYPE;  

typedef enum CI_SIM_PRIM
{
  CI_SIM_PRIM_EXECCMD_REQ = 1,						/**< \brief Requests to execute a SIM command \details   */
  CI_SIM_PRIM_EXECCMD_CNF,							/**< \brief Confirms the request to execute a SIM command \details   */
  CI_SIM_PRIM_DEVICE_IND,								/**< \brief Indicates that the current SIM status changed \details   */
  CI_SIM_PRIM_PERSONALIZEME_REQ,						/**< \brief Requests that ME personalization be activated, deactivated, disabled, or queried \details   */
  CI_SIM_PRIM_PERSONALIZEME_CNF,						/**< \brief Confirms the request to activate, deactivate, disable, or query ME personalization \details   */
  CI_SIM_PRIM_OPERCHV_REQ,							/**< \brief Requests that CHVs be verified, enabled, disabled, changed, unblocked, or queried \details   */
  CI_SIM_PRIM_OPERCHV_CNF,							/**< \brief Confirms the request to verify, enable, disable, change, unblock, or query CHVs \details   */
  CI_SIM_PRIM_DOWNLOADPROFILE_REQ,					/**< \brief Requests a download of the profile that shows ME capabilities relevant to SIM Application Toolkit functionality \details The functionality of this primitive is equivalent to using the SIM command TERMINAL PROFILE in the CI_SIM_PRIM_EXECCMD_REQ primitive.
  *  This primitive saves upper layer effort to build a header for the Terminal Profile SIM command.
  *  If the pProfile pointer is NULL for this request, the communications interface assumes that the application layer does not support
  *  SIM Application Toolkit operations.  */
  CI_SIM_PRIM_DOWNLOADPROFILE_CNF,					/**< \brief Confirms the request to download the profile that shows ME capabilities relevant to SIM Application Toolkit functionality  \details   */
  CI_SIM_PRIM_ENDATSESSION_IND,						/**< \brief  \details NOT SUPPORTED REMOVE FROM API  */
  CI_SIM_PRIM_PROACTIVE_CMD_IND,						/**< \brief Indicates a SIMAT proactive command  \details This primitive forwards the SIMAT proactive command with its original syntax. The primitive CI_SIM_PRIM_ENABLE_SIMAT_INDS_REQ enables and disables proactive command indication.  */
  CI_SIM_PRIM_PROACTIVE_CMD_RSP,						/**< \brief Responds to the SIMAT proactive command  \details The primitive forwards a SIMAT proactive command response from the entity, such as DISPLAY or TERMINAL, that received the proactive command.  */
  CI_SIM_PRIM_ENVELOPE_CMD_REQ,						/**< \brief Requests that an ENVELOPE command be executed \details   */
  CI_SIM_PRIM_ENVELOPE_CMD_CNF,						/**< \brief Confirms the request to execute an ENVELOPE command  \details   */
  CI_SIM_PRIM_GET_SUBSCRIBER_ID_REQ,					/**< \brief Requests the subscriber ID \details   */
  CI_SIM_PRIM_GET_SUBSCRIBER_ID_CNF,					/**< \brief Confirms the request for the subscriber ID  \details   */
  CI_SIM_PRIM_GET_PIN_STATE_REQ,						/**< \brief Requests the current PIN state \details   */
  CI_SIM_PRIM_GET_PIN_STATE_CNF,						/**< \brief Confirms the request for the current PIN state \details   */
  CI_SIM_PRIM_GET_TERMINALPROFILE_REQ,				/**< \brief Requests the SIMAT terminal profile  \details   */
  CI_SIM_PRIM_GET_TERMINALPROFILE_CNF,				/**< \brief Confirms the request and returns the SIMAT terminal profile \details   */
  CI_SIM_PRIM_ENABLE_SIMAT_INDS_REQ,					/**< \brief Requests that SIMAT related indications, such as the proactive SIM command indication and the SIMAT session ended indication, be enabled or disabled  \details   */
  CI_SIM_PRIM_ENABLE_SIMAT_INDS_CNF,					/**< \brief Confirms the request to enable or disable SIMAT related indications  \details   */
  CI_SIM_PRIM_LOCK_FACILITY_REQ,						/**< \brief Requests to lock, unlock, or query SIM-related ME  \details   */
  CI_SIM_PRIM_LOCK_FACILITY_CNF,						/**< \brief Confirms a request to lock, unlock, or query SIM-related ME  \details   */
  CI_SIM_PRIM_GET_FACILITY_CAP_REQ,					/**< \brief Requests the bitmask of supported SIM-related facility codes \details   */
  CI_SIM_PRIM_GET_FACILITY_CAP_CNF,					/**< \brief Confirms the request for the bitmask of supported SIM-related facility codes \details   */
  CI_SIM_PRIM_GET_SIMAT_NOTIFY_CAP_REQ,				/**< \brief Requests SIM Application Toolkit (SIMAT) notification capability information. \details   */
  CI_SIM_PRIM_GET_SIMAT_NOTIFY_CAP_CNF,				/**< \brief Confirms the request for SIM Application Toolkit (SIMAT) notification capability information \details   */
  CI_SIM_PRIM_GET_CALL_SETUP_ACK_IND,				/**< \brief Indicates that the SIM Application Toolkit (SIMAT) has initiated an outgoing CALL SETUP operation, and requests confirmation/acknowledgment from the mobile user \details The application returns the required acknowledgment in a CI_SIM_PRIM_GET_CALL_SETUP_ACK_RSP response.  */
  CI_SIM_PRIM_GET_CALL_SETUP_ACK_RSP,				/**< \brief Responds with an acknowledgment from the mobile user for an outgoing CALL SETUP indication  \details The mobile user may accept (allow) or reject (disallow) the SIMAT initiated CALL SETUP operation.
*     If the user allows the CALL SETUP, it proceeds. If the user disallows the CALL SETUP, it is aborted.
*     If the CALL SETUP is allowed to proceed, the MO call progression is managed by the normal call control procedures. See the
*     CI CC Service Group API definition for more information.  */

  /* service provider name */
  CI_SIM_PRIM_GET_SERVICE_PROVIDER_NAME_REQ,		/**< \brief Requests the service provider name, as stored on SIM or USIM \details The PIN status is not required to read this information.  */
  CI_SIM_PRIM_GET_SERVICE_PROVIDER_NAME_CNF,		/**< \brief Confirms the request to get the service provider name, as stored on SIM or USIM \details The service provider name is coded as 7-bit GSM characters, with the most-significant bit of each character set to zero.
  *  The service provider name pointer is NULL if the result code indicates an error.  */

  /* Message Waiting Information */
  CI_SIM_PRIM_GET_MESSAGE_WAITING_INFO_REQ,		/**< \brief Requests to get message waiting information stored on SIM or USIM \details The PIN status is required to read this information.  */
  CI_SIM_PRIM_GET_MESSAGE_WAITING_INFO_CNF,			/**< \brief Confirms the request to get message waiting information stored on SIM or USIM \details If the result code indicates an error, the message waiting status information is not useful.
  *  There is a difference between the message categories defined for 2G and 3G SIM storage. This is rationalized by the CCI implementation.  */
  CI_SIM_PRIM_SET_MESSAGE_WAITING_INFO_REQ,			/**< \brief Requests to set message waiting information on SIM or USIM  \details Requires PIN status to write this information. There is a difference between the message categories defined for 2G and 3G SIM storage. This is rationalized by the CCI implementation.  */
  CI_SIM_PRIM_SET_MESSAGE_WAITING_INFO_CNF,			/**< \brief Confirms a request to set the message waiting information on SIM or USIM \details   */

  /* SIM Service Table */
  CI_SIM_PRIM_GET_SIM_SERVICE_TABLE_REQ,				/**< \brief Requests to get the SIM Service Table from SIM or USIM  \details The PIN status is required to read this information. If CPHS features are not supported by the handset, this information is unavailable.  */
  CI_SIM_PRIM_GET_SIM_SERVICE_TABLE_CNF,				/**< \brief Confirms the request to get the SIM Service Table from SIM or USIM \details If CPHS features are not supported by the handset, this information is unavailable.  */

  /* CPHS Customer Service Profile */
  CI_SIM_PRIM_GET_CUSTOMER_SERVICE_PROFILE_REQ,	/**< \brief Requests to get the CPHS customer service profile from SIM or USIM \details The PIN status is required to read this information. If CPHS features are not supported by the handset, this information is unavailable. */
  CI_SIM_PRIM_GET_CUSTOMER_SERVICE_PROFILE_CNF,	/**< \brief Confirms the request and returns the CPHS customer service profile from SIM or USIM. \details If CPHS features are not supported by the handset, this information is unavailable.  */

  /* Display Alpha and Icon Identifiers */
  CI_SIM_PRIM_SIMAT_DISPLAY_INFO_IND,					/**< \brief Indicates to the application that text and optionally an icon should be displayed.
														 *  The text to be displayed results from a SAT transaction such as SS, SMS, USSD, SS, or send DTMF. \details   */

  /* Default Language */
  CI_SIM_PRIM_GET_DEFAULT_LANGUAGE_REQ,			/**< \brief Requests the default language stored on the SIM/USIM card \details   */
  CI_SIM_PRIM_GET_DEFAULT_LANGUAGE_CNF,				/**< \brief Confirms the request to get the default language stored on the SIM/USIM card and returns the first entry in the EF_LP file \details Extract from ETSI TS 102.221: "the language code is a pair of alphanumeric characters, as defined in ISO 639 [30].
  *  Each alphanumeric character shall be coded on one byte using the SMS default 7-bit coded alphabet as defined in TS 23.038
  *  ("Man-machine Interface (MMI) of the User Equipment", revision 3.4.0, Doc Number 3GPP TS 22.030)
  *  with bit 8 set to 0.". 'FF FF' means undefined default language.  */

  /* Generic SIM commands */
  CI_SIM_PRIM_GENERIC_CMD_REQ,						/**< \brief Requests to send a generic command to the SIM/USIM card
													 * \details The request reflects the structure of a SIM application protocol data unit (APDU),
													 * as defined in ETSI 102.221. The 'class of instruction' element is not controlled by the user, comm. use class 0x0 or 0xa depending on the command.
													 * Note that updating a file using this command only updates the file on the SIM; it does not trigger a REFRESH of the ME memory. */
  CI_SIM_PRIM_GENERIC_CMD_CNF,						/**< \brief Confirms a request to send a generic command to the SIM or USIM  \details  */

  /* Indication of card type, status and PIN state */
  CI_SIM_PRIM_CARD_IND,								/**< \brief Indicates that the current SIM/USIM status changed \details This indication is sent each time CI_SIM_PRIM_DEVICE_IND is sent.  */

  CI_SIM_PRIM_IS_EMERGENCY_NUMBER_REQ,				/**< \brief Requests to determine if the specified dial number is an emergency call code \details   */
  CI_SIM_PRIM_IS_EMERGENCY_NUMBER_CNF,				/**< \brief Confirms the request to determine if the specified number is an emergency call code \details If a SIM card is present, the EF_ECC SIM card file is searched for the specified number. If a SIM card is not present, a default table of possible emergency call codes is searched for the specified number, as per TS 22.101. */

  CI_SIM_PRIM_SIM_OWNED_IND,							/**< \brief Indicates whether the SIM is owned
														 * \details This indication is sent each time a SIM-OK notification is received
 * from the protocol stack and indicates that the SIM card can be accessed. SIM owned is TRUE if the IMSI did not change
 * since the last SIM-OK notification.  */
  CI_SIM_PRIM_SIM_CHANGED_IND,						/**< \brief Indicates whether the IMSI on the current SIM has changed
													 * \details  This indication is sent each time a SIM-OK notification is received
 * from the protocol stack. */
  CI_SIM_PRIM_DEVICE_STATUS_REQ,						/**< \brief Requests SIM status \details   */
  CI_SIM_PRIM_DEVICE_STATUS_CNF,						/**< \brief Confirms the request for the current SIM status \details   */
  CI_SIM_PRIM_READ_MEP_CODES_REQ,			/**< \brief   Requests the MEP codes for a specified category  \details This operation does not require a password.*/
  CI_SIM_PRIM_READ_MEP_CODES_CNF,			/**< \brief   Confirms the request and returns the MEP codes for the specified category \details */
  CI_SIM_PRIM_UDP_LOCK_REQ, 					/**< \brief   Requests an activate, deactivate, or query UDP lock  \details  An operation can be done on only one category at a time. A password is required for an unlock operation. */
  CI_SIM_PRIM_UDP_LOCK_CNF, 					/**< \brief   Confirms the UDP lock request \details */
  CI_SIM_PRIM_UDP_CHANGE_PASSWORD_REQ,  	/**< \brief   Requests to set a new password for a UDP lock \details */
  CI_SIM_PRIM_UDP_CHANGE_PASSWORD_CNF, 	/**< \brief   Confirms the request to set a new password for a UDP lock \details */
  CI_SIM_PRIM_UDP_ASL_REQ,					/**< \brief  Requests to manipulate the UDP authorized SIM list \details */
  CI_SIM_PRIM_UDP_ASL_CNF,					/**< \brief  Confirms the request to manipulate the UDP authorized SIM list  \details */
/* Michal Bukai - Virtual SIM support - START */
   CI_SIM_PRIM_SET_VSIM_REQ,                /**< \brief  Requests to enable virtual SIM
											 * \details Virtual SIM can be enabled if no SIM is inserted.
											 * An error is sent if the user tried to enable virtual SIM while a SIM is inserted.
											 * The application needs to reset the communication subsystem after receiving a confirmation. */
   CI_SIM_PRIM_SET_VSIM_CNF,                 /**< \brief Confirms setting virtual SIM
											  * \details Virtual SIM can be enabled if no SIM is inserted.
											 * An error is sent if the user tried to enable virtual SIM while a SIM is inserted.
											 * The application needs to reset the communication subsystem after receiving a confirmation. */
   CI_SIM_PRIM_GET_VSIM_REQ,                 /**< \brief Requests the current setting of the virtual SIM (enabled / disabled) \details*/
   CI_SIM_PRIM_GET_VSIM_CNF,                 /**< \brief Confirms the request and returns the current setting of the virtual SIM (enabled / disabled) \details*/
/* Michal Bukai - Virtual SIM support - END */
/*Michal Bukai - OTA support for AT&T - START*/
  CI_SIM_PRIM_CHECK_MMI_STATE_IND,	/**< \brief  \details NOT SUPPORTED REMOVE FROM API  */
  CI_SIM_PRIM_CHECK_MMI_STATE_RSP,	/**< \brief  \details NOT SUPPORTED REMOVE FROM API  */
/*Michal Bukai - OTA support for AT&T - END*/
/*Michal Bukai - BT SAP support - START*/
  CI_SIM_PRIM_BTSAP_CONNECT_REQ,					/**< \brief Requests to start a BT SAP session \details */
  CI_SIM_PRIM_BTSAP_CONNECT_CNF,					/**< \brief Confirms the request to start a BT SAP session \details */
  CI_SIM_PRIM_BTSAP_DISCONNECT_REQ, 				/**< \brief Requests to disconnect from a BT SAP session \details */
  CI_SIM_PRIM_BTSAP_DISCONNECT_CNF, 				/**< \brief Confirms the request to disconnect from a BT SAP session \details */
  CI_SIM_PRIM_BTSAP_TRANSFER_APDU_REQ,				/**< \brief Requests to transfer APDU to the SIM/USIM \details */
  CI_SIM_PRIM_BTSAP_TRANSFER_APDU_CNF,				/**< \brief Confirms the request to transfer APDU to the SIM/USIM and may return a response APDU from the SIM/USIM \details */
  CI_SIM_PRIM_BTSAP_TRANSFER_ATR_REQ,				/**< \brief Requests to get Answer To Reset data from SIM/USIM \details */
  CI_SIM_PRIM_BTSAP_TRANSFER_ATR_CNF,				/**< \brief Confirms the request to get Answer To Reset data from SIM/USIM \details */
  CI_SIM_PRIM_BTSAP_SIM_CONTROL_REQ,				/**< \brief Requests to control SIM/USIM status, this command can be used to power off, power on or reset the SIM/USIM \details */
  CI_SIM_PRIM_BTSAP_SIM_CONTROL_CNF,				/**< \brief Confirms the SIM control request \details */
  CI_SIM_PRIM_BTSAP_STATUS_IND,						/**< \brief indicates a change in the availably of the subscription module during BT SAP connection \details */
  CI_SIM_PRIM_BTSAP_STATUS_REQ,						/**< \brief Requests the subscription module availability status during BT SAP connection \details */
  CI_SIM_PRIM_BTSAP_STATUS_CNF,						/**< \brief Confirms the request and returns the status of the subscription module during BT SAP connection \details */
  CI_SIM_PRIM_BTSAP_SET_TRANSPORT_PROTOCOL_REQ,		/**< \brief Requests to set transport protocol \details */
  CI_SIM_PRIM_BTSAP_SET_TRANSPORT_PROTOCOL_CNF,		/**< \brief Confirms the request to set transport protocol \details */
/*Michal Bukai - BT SAP support - END*/
/*Michal Bukai - Add IMSI to MEP code group - START*/
  CI_SIM_PRIM_MEP_ADD_IMSI_REQ,	/**< \brief  Requests to add the current IMSI to MEP SIM /USIM code group
								* \details This operation requires a password.
								* This operation requires that SIM/USIM personalization is deactivated.*/
  CI_SIM_PRIM_MEP_ADD_IMSI_CNF,	/**< \brief Confirms the request to add the current IMSI to MEP SIM /USIM code group. \details   */
/*Michal Bukai - Add IMSI to MEP code group - END*/
/*Michal Bukai - SIM Logic CH - NFC\ISIM support - START*/
  CI_SIM_PRIM_OPEN_LOGICAL_CHANNEL_REQ,		/**< \brief  Requests to open a logical channel that will be used to access the UICC application identified by DFname.  \details The UICC will open a new logical channel; select the application identified by the DFname, and return a session ID that will be used to identify the new channel.*/
  CI_SIM_PRIM_OPEN_LOGICAL_CHANNEL_CNF,		/**< \brief  Confirms the request to open a logical channel and returns the session ID.  \details */
  CI_SIM_PRIM_CLOSE_LOGICAL_CHANNEL_REQ,	/**< \brief  Requests to close a logical channel.  \details */
  CI_SIM_PRIM_CLOSE_LOGICAL_CHANNEL_CNF,	/**< \brief  Confirms the request to close a logical channel.  \details */
/*Michal Bukai - SIM Logic CH - NFC\ISIM support support - END*/
/*Michal Bukai - additional SIMAT primitives - START*/
  CI_SIM_PRIM_SIMAT_CC_STATUS_IND,					/**< \brief Indicates the SIM Application Toolkit (SIMAT) call control status response 
													* \details If call control service in SIMAT is activated, all dialled digit strings, supplementary service control strings and USSD strings are passed to the UICC before the call setup request, 
													* the supplementary service operation or the USSD operation is sent to the network.
													* The SIMAT has the ability to allow, bar or modify the request. 
													* In addition SIMAT has the ability to replace the request by another operation, for instance call request may be replaced by SS or USSD operation.
													*/
  CI_SIM_PRIM_SIMAT_SEND_CALL_SETUP_RSP_IND,		/**< \brief Indicates the response sent to SIM Application Toolkit (SIMAT) after call setup. \details */
  CI_SIM_PRIM_SIMAT_SEND_SS_USSD_RSP_IND, 			/**< \brief Indicates the response sent to SIM Application Toolkit (SIMAT) after SS or USSD operation. \details */
  CI_SIM_PRIM_SIMAT_SM_CONTROL_STATUS_IND, 			/**< \brief Indicates the SIM Application Toolkit (SIMAT) short message control status response. 
													\details If SM control service in SIMAT is activated, all MO short messages are passed to the UICC before the short message is sent to the network.
													* The SIMAT has the ability to allow, bar or modify the destination address.
													*/
  CI_SIM_PRIM_SIMAT_SEND_SM_RSP_IND,				/**< \brief Indicates the response sent to SIM Application Toolkit (SIMAT) after SM operation. \details */
/*Michal Bukai - additional SIMAT primitives - END*/

/*Michal Bukai - RSAP support - START*/
  CI_SIM_PRIM_RSAP_CONN_REQ_IND,		/**< \brief  Request to connect to a remote SIM received from the protocol stack \details */
  CI_SIM_PRIM_RSAP_CONN_REQ_RSP,		/**< \Response to protocol stack request to connect to a remote SIM \details */
  CI_SIM_PRIM_RSAP_STAT_REQ,			/**< \brief  Request received from a SAP conversion module to update the RSAP card status. 
										* \details This request is actually an indication from a remote SAP conversion module indicating a status change in the remote connection or card status */
  CI_SIM_PRIM_RSAP_STAT_CNF,			/**< \brief  Confirms that the update of the remote card status was received by the protocol stack \details */
  CI_SIM_PRIM_RSAP_DISCONN_REQ_IND,		/**< \brief  Request to disconnect from a remote SIM received from the protocol stack \details */
  CI_SIM_PRIM_RSAP_DISCONN_REQ_RSP,		/**< \Response to protocol stack request to disconnect from a remote SIM \details */
  CI_SIM_PRIM_RSAP_GET_ATR_IND,			/**< \brief Request to get ATR from a remote SIM received from the protocol stack \details */
  CI_SIM_PRIM_RSAP_GET_ATR_RSP,			/**< \brief Response from a remote SIM with the ATR APDU \details */
  CI_SIM_PRIM_RSAP_GET_STATUS_REQ_IND,	/**< \brief  Request from the protocol stack to get the RSAP connection status. 
										* \details The request is answered by CI_SIM_PRIM_RSAP_CONN_STAT_REQ */

  CI_SIM_PRIM_RSAP_SET_TRAN_P_REQ_IND,	/**< \brief Request from the protocol stack to change the transport protocol of the remote SIM \details */
  CI_SIM_PRIM_RSAP_SET_TRAN_P_REQ_RSP,	/**< \brief Response to a protocol stack request to change the transport protocol of the remote SIM.
										* \details If the requested transport protocol is supported by the remote SIM and by the SAP conversion module, 
										* the requested transport protocol is selected and the remote SIM is reset.	If the requested transport protocol is not supported, 
										* SIM status is changed to CARD_NOT_ACC. The request is followed by CI_SIM_PRIM_RSAP_CONN_STAT_REQ, which indicates the new remote SIM status.*/
  CI_SIM_PRIM_RSAP_SIM_CONTROL_REQ_IND,	/**< \brief  Request from the protocol stack to control the remote SIM status. 
										* \details This command can be used to power off, power on, or reset the remote SIM */
  CI_SIM_PRIM_RSAP_SIM_CONTROL_REQ_RSP,	/**< \brief  Response to a protocol stack request to control the remote SIM status. 
										* \details The response is followed by CI_SIM_PRIM_RSAP_CONN_STAT_REQ, which indicates the new status of the remote SIM.*/
  CI_SIM_PRIM_RSAP_SIM_SELECT_REQ,		/**< \brief Request to select the local or remote SIM \details */
  CI_SIM_PRIM_RSAP_SIM_SELECT_CNF,		/**< \brief Confirms the request to select the local or remote SIM \details */
  CI_SIM_PRIM_RSAP_STATUS_IND,			/**< \brief Indicates that the current SIM/USIM status changed during RSAP connection \details */
  CI_SIM_PRIM_RSAP_TRANSFER_APDU_IND,	/**< \brief Request from the protocol stack to transfer APDU to the remote SIM \details */
  CI_SIM_PRIM_RSAP_TRANSFER_APDU_RSP,	/**< \brief Request from the protocol stack to transfer APDU to the remote SIM. 
										* \details A response APDU is returned if the transfer of APDU is successful.*/
  /*Michal Bukai - RSAP support - END*/

  CI_SIM_PRIM_DEVICE_RSP, 				/**< \brief Response to CI_SIM_PRIM_DEVICE_RSP.
										* \details This response is confirms that the CI_SIM_PRIM_DEVICE_RSP has been received and handled. Specially the SIM clock stop level. This indicates the Comm that D2 can be enabled on SIM driver level.*/
//ICC ID feature
  CI_SIM_PRIM_ICCID_IND, 				/**< \brief Indicates the content of the EF-ICCID file. This indication is sent at init. The EF-ICCID can be accessed even if PIN is required.*/
  CI_SIM_PRIM_GET_ICCID_REQ, 			/**< \brief Request to get the content of the EF-ICCID file. Can be sent if ICC is ready.
										* \details Can be sent when the SIM state is CI_SIM_ST_READY or CI_SIM_ST_INSERTED.	Otherwise the request will fail.*/

  CI_SIM_PRIM_GET_ICCID_CNF,			/**< \brief Confirmation with the ICC id to the request to get the ICC Id.*/
//ICC ID feature
  CI_SIM_PRIM_EAP_AUTHENTICATION_REQ,	/**< \brief Requests to exchange EAP packets with the UICC. */
  CI_SIM_PRIM_EAP_AUTHENTICATION_CNF,	/**< \brief Confirms the EAP authentication request and returns the authentication response */

  CI_SIM_EAP_RETRIEVE_PARAMETERS_REQ, /**< \brief Requests to retrieve EAP parameters from the UICC. */
  CI_SIM_EAP_RETRIEVE_PARAMETERS_CNF, /**< \brief Confirms the request to retrieve EAP parameters and returns the contents of the
										* \elementary file corresponding to requested parameter.*/

  CI_SIM_PRIM_GET_NUM_UICC_APPLICATIONS_REQ,/**< \brief Requests to get number of applications available on the UICC. */
  CI_SIM_PRIM_GET_NUM_UICC_APPLICATIONS_CNF,/**< \brief Confirms the request to get number of applications available on the UICC. */

  CI_SIM_PRIM_GET_UICC_APPLICATIONS_INFO_REQ, /**< \brief Requests to get list of applications available on the UICC. */
  CI_SIM_PRIM_GET_UICC_APPLICATIONS_INFO_CNF, /**< \brief Confirms the request to get list of applications available on the UICC. */
  /* ADD NEW COMMON PRIMITIVES HERE, BEFORE 'CI_SIM_PRIM_LAST_COMMON_PRIM' */
  /*2013.12.11, added by Xili for CQ00051618, begin*/
  CI_SIM_PRIM_ISIM_AID_IND,                         /**< \brief Indicates the content of the ISIM Aid. This indication is sent at init if it had one. */
  /*2013.12.11, added by Xili for CQ00051618, end*/

  /* Add by jungle for CQ00057999 on 2014-04-02 Begin */
  CI_SIM_PRIM_APP_PIN_REQ,
  CI_SIM_PRIM_APP_PIN_CNF,
  /* Add by jungle for CQ00057999 on 2014-04-02 End*/

  /*2014.05.08, added by Xili for CQ00060947, begin*/
  CI_SIM_PRIM_ADMIN_DATA_IND,  
  /*2014.05.08, added by Xili for CQ00060947, end*/

  /*2015.03.19, mod by Xili for adding ECC list indication, CQ00088196 begin*/
  CI_SIM_PRIM_ECC_LIST_IND,  
  /*2015.03.19, mod by Xili for adding ECC list indication, CQ00088196 end*/

  /* Mod by jungle for CQ00089692 on 2015-04-08 Begin */
  CI_SIM_PRIM_EXEC_LARGE_CMD_CNF,
  CI_SIM_PRIM_GENERIC_LARGE_CMD_CNF,
  /* Mod by jungle for CQ00089692 on 2015-04-08 End */

  CI_SIM_PRIM_UPDATE_COUNT_REQ,
  CI_SIM_PRIM_UPDATE_COUNT_CNF,
  
  /* END OF COMMON PRIMITIVES LIST */
  CI_SIM_PRIM_LAST_COMMON_PRIM

  /* The customer specific extension primitives are added starting from
  * CI_SIM_PRIM_firstCustPrim = CI_SIM_PRIM_LAST_COMMON_PRIM as the first identifier.
  * The actual primitive names and IDs are defined in the associated
  * 'ci_sim_cust_xxx.h' file.
  */

  /* DO NOT ADD ANY MORE PRIMITIVES HERE */

} _QuecCiSimPrim;

typedef struct 
{
    uint8_t                         pin1_num_retries;       /**<   Number of PIN 1 retries. */
    uint8_t                         puk1_num_retries;       /**<   Number of PUK 1 retries. */
    uint8_t                         pin2_num_retries;       /**<   Number of PIN 2 retries. */
    uint8_t                         puk2_num_retries;       /**<   Number of PUK 2 retries. */
}QL_SIM_CARD_PIN_INFO;  
 
typedef struct 
{
    QL_SIM_CARD_TYPE      card_type; // SIM card type
    QL_SIM_CARD_STATE     card_state;  //SIM card state
    QL_SIM_CARD_PIN_INFO  card_pin_info; // PIN info
}QL_SIM_CARD_STATUS_INFO;  


#define QL_SIM_DATA_LEN_MAX     255
typedef struct 
{
    uint32_t    data_len;                        /**< Must be set to the number of elements in data. */
    uint8_t     data[QL_SIM_DATA_LEN_MAX];       /**< Data retrieved from the card. */
}QL_SIM_CARD_FILE_DATA;  
 
typedef enum 
{
    QL_SIM_FILE_TYPE_UNKNOWN      = 0,/**<  Unknown file type  */
    QL_SIM_FILE_TYPE_TRANSPARENT  = 1,/**< File structure consisting of a sequence of bytes.  */
    QL_SIM_FILE_TYPE_CYCLIC       = 2,/**< File structure consisting of a sequence of records, each containing the same fixed size in 
                                                     chronological order. Once all the records have been used, the oldest data is overwritten.  */
    QL_SIM_FILE_TYPE_LINEAR_FIXED  = 3,    /**< File structure consisting of a sequence of records, each containing the same fixed size.  */
}QL_SIM_FILE_TYPE;

typedef enum 
{
    QL_SIM_FILE_ACCESS_TYPE_ALWAYS      =0,
    QL_SIM_FILE_ACCESS_TYPE_CHV1        =1,
    QL_SIM_FILE_ACCESS_TYPE_CHV2        =2,
    QL_SIM_FILE_ACCESS_TYPE_ADM         =3,
}QL_SIM_FILE_ACCESS_TYPE;

typedef enum 
{
    QL_SIM_FILE_STATUS_INVALID      =0,
    QL_SIM_FILE_STATUS_EFFECTIVE    =1,
}QL_SIM_FILE_STATUS;

typedef struct 
{
    QL_SIM_FILE_ACCESS_TYPE read_access;
    QL_SIM_FILE_ACCESS_TYPE update_access;
}QL_SIM_FILE_ACCESS_INFO;

typedef struct 
{
    unsigned int                id;
    QL_SIM_FILE_TYPE            type;    /**<   File type: */
    QL_SIM_FILE_ACCESS_INFO     access;    /**<   File access conditions: */
    QL_SIM_FILE_STATUS          status;    /**<   File status: */
    unsigned int                size;      /**<   Size of transparent files.*/
    unsigned int                record_len;    /**<   Size of each cyclic or linear fixed file record.*/
    unsigned int                record_count;   /**<   Number of cyclic or linear fixed file records.*/
}QL_SIM_FILE_INFO;  

typedef struct 
{
    int sw1;
    int sw2;
}QL_SIM_FILE_OPERATION_RET;

typedef struct 
{ // when write, if phonenum is empty, it means to delete this item specified by index  
    int          index;  // the record index in phone book
    uint8_t      username[32];   //  username
    uint8_t      phonenum[24];   //  Phone number, it can include '+'*/
}QL_SIM_PHONE_BOOK_RECORD_INFO;

typedef struct
{
    int total;      //total number of locations
    int used;       //used number of locations
}QL_SIM_PHONEBOOK_STATE;

#define QL_SIM_PHONE_BOOK_RECORDS_MAX_COUNT 20
typedef struct 
{
    int record_count;  //the count of record
    QL_SIM_PHONE_BOOK_RECORD_INFO record[QL_SIM_PHONE_BOOK_RECORDS_MAX_COUNT]; // the list of record
}QL_SIM_PHONE_BOOK_RECORDS_INFO;

typedef enum
{
    QL_SIM_PHONE_BOOK_STORAGE_DC,
    QL_SIM_PHONE_BOOK_STORAGE_EN,
    QL_SIM_PHONE_BOOK_STORAGE_FD,
    QL_SIM_PHONE_BOOK_STORAGE_LD,
    QL_SIM_PHONE_BOOK_STORAGE_MC,
    QL_SIM_PHONE_BOOK_STORAGE_ME,
    QL_SIM_PHONE_BOOK_STORAGE_MT,
    QL_SIM_PHONE_BOOK_STORAGE_ON,
    QL_SIM_PHONE_BOOK_STORAGE_RC,
    QL_SIM_PHONE_BOOK_STORAGE_SM,
    QL_SIM_PHONE_BOOK_STORAGE_AP,
    QL_SIM_PHONE_BOOK_STORAGE_MBDN,
    QL_SIM_PHONE_BOOK_STORAGE_MN,
    QL_SIM_PHONE_BOOK_STORAGE_SDN,
    QL_SIM_PHONE_BOOK_STORAGE_ICI,
    QL_SIM_PHONE_BOOK_STORAGE_OCI,
}QL_SIM_PHONE_BOOK_STORAGE;



typedef enum 
{
    QL_SIM_CARD_STATUS_UPDATE_EVENT   = 0,    /**< Card status update event.  */
    
}QL_SIM_NFY_MSG_ID;  

typedef void (*QL_SIM_CardStatusIndMsgHandlerFunc_t)   
(    
    QL_SIM_NFY_MSG_ID     msg_id,
    void                  *pv_data,
    int                   pv_data_len,
    void                  *contextPtr    
);


QL_SIM_ERROR_CODE ql_sim_init();

QL_SIM_ERROR_CODE ql_sim_release();

/* Add callback function if anything changed specified by the mask in QL_LOC_Set_Indications*/
QL_SIM_ERROR_CODE ql_sim_add_event_handler(QL_SIM_CardStatusIndMsgHandlerFunc_t handlerPtr, void* contextPtr);

/*===========================================================================

  FUNCTION:  ql_sim_get_imsi

===========================================================================*/
/*
    @brief
    Function sends a command to the modem to retrieve the IMSI (for 3GPP)
    or IMSI_M (for 3GPP2) from the SIM in ASCII form

    @return
    void
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_get_imsi
(
    uint8_t   *imsi,      ///< [OUT] IMSI buffer
    size_t    imsiLen     ///< [IN] IMSI buffer length
);


/*===========================================================================

  FUNCTION:  ql_sim_get_iccid

===========================================================================*/
/*
    @brief
    Function sends a command to the modem to retrieve the ICCID from
    SIM in ASCII form

    @return
    QL_SIM_ERROR_CODE
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_get_iccid
(
    uint8_t    *iccid,    ///< [OUT] ICCID
    size_t     iccidLen   ///< [IN] ICCID buffer length
);


/*===========================================================================

  FUNCTION:  QL_SIM_GetPhoql_sim_get_phonenumberneNumber

===========================================================================*/
/*
    @brief
    Function sends a command to the modem to retrieve the device phone
    number from MSISDN (for 3GPP) or MDN (for 3GPP2) from the SIM in
    ASCII form

    @return
    QL_SIM_ERROR_CODE
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_get_phonenumber
(
    uint8_t   *phone_num, ///< [OUT] phone number
    size_t    phoneLen    ///< [IN] phone number buffer length
);

/*===========================================================================

  FUNCTION:  ql_sim_get_operator_plmn_list

===========================================================================*/
/*
    @brief
    Function sends multiple record based read commands to the modem
    to retrieve a list of operator preffered PLMN.

    @note
    Function should only be called for SIM/USIM

    @return
    QL_SIM_ERROR_CODE
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_get_operator_plmn_list
(
    QL_SIM_PREFERRED_OPERATOR_LIST      *pt_info    ///< [OUT] Preferred operator list
);

/*===========================================================================

  FUNCTION:  ql_sim_verify_pin

===========================================================================*/
/*
    @brief
    Function sends a command to the modem to verify either PIN1 or PIN2

    @return
    QL_SIM_ERROR_CODE
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_verify_pin
(
    QL_SIM_VERIFY_PIN_INFO      *pt_info   // [IN] PIN/PIN2
);


/*===========================================================================

  FUNCTION:  ql_sim_change_pin

===========================================================================*/
/*
    @brief
    Function sends a command to the modem to change the value of
    either PIN

    @return
    QL_SIM_ERROR_CODE
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_change_pin
(
    QL_SIM_CHANGE_PIN_INFO      *pt_info   // [IN] PIN 
);


/*===========================================================================

  FUNCTION:  ql_sim_unblock_pin

===========================================================================*/
/*
    @brief
    Function sends a command to the modem to unblock a PIN1 or PIN2 that
    has been blocked

    @return
    QL_SIM_ERROR_CODE
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_unblock_pin
(
    QL_SIM_UNBLOCK_PIN_INFO     *pt_info   // [IN] input PUK/PUK2 and new PIN/PIN2
);

/*===========================================================================

  FUNCTION:  ql_sim_enable_pin

===========================================================================*/
/*
    @brief
    Function sends a command to the modem to enable PIN

    @return
    QL_SIM_ERROR_CODE
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_enable_pin
(
    QL_SIM_VERIFY_PIN_INFO      *pt_info   // [IN] PIN
);

/*===========================================================================

  FUNCTION:  ql_sim_disable_pin

===========================================================================*/
/*
    @brief
    Function sends a command to the modem to disable PIN

    @return
    QL_SIM_ERROR_CODE
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_disable_pin
(
    QL_SIM_VERIFY_PIN_INFO     *pt_info   // [IN] PIN 
);


/*===========================================================================

  FUNCTION:  ql_sim_get_card_status

===========================================================================*/
/*
    @brief
    Function get sim card status informations

    @return
    QL_SIM_ERROR_CODE
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_get_card_status
(
    QL_SIM_CARD_STATUS_INFO     *pt_info   ///< [OUT] sim card status info output
);



/*===========================================================================

  FUNCTION:  ql_sim_get_file_info

===========================================================================*/
/*
    @brief
    Function sends a command to the modem to retrieve the file info

    @return
    QL_SIM_ERROR_CODE
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_get_file_info
(
    unsigned int fileid,
    QL_SIM_FILE_INFO  *file_info,     // [OUT] output file info
    QL_SIM_FILE_OPERATION_RET *operation_ret // [OUT] return the operation status code 
);

/*===========================================================================

  FUNCTION:  ql_sim_read_file

===========================================================================*/
/*
    @brief
    Function get the data of a EF file.

    @return
    QL_SIM_ERROR_CODE
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_read_file
(
    unsigned int fileid,  // EF file id
    unsigned int index,   // the index of record for a record EF file, the offest of date for a transparent EF file.
    unsigned int data_max_len, // the max length of output data.
    QL_SIM_CARD_FILE_DATA  *file_data,     // [OUT] File data
    QL_SIM_FILE_OPERATION_RET *operation_ret // [OUT] return the operation status code 
);

/*===========================================================================

  FUNCTION:  ql_sim_write_file

===========================================================================*/
/*
    @brief
    Function write data to a EF file.

    @return
    QL_SIM_ERROR_CODE
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_write_file
(
    unsigned int fileid, // EF file id
    int index, // the index of record for a record EF file, the offest of date for a transparent EF file.
    QL_SIM_CARD_FILE_DATA  *file_data,     ///< [IN] File data of the specified one by pt_info
    QL_SIM_FILE_OPERATION_RET *operation_ret // [OUT] return the operation status code
);


/*===========================================================================

  FUNCTION:  ql_sim_read_phonebook_record

===========================================================================*/
/*
    @brief
    Function will get the phone book record.

    @return
    QL_SIM_ERROR_CODE
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_read_phonebook_record
(
    QL_SIM_PHONE_BOOK_STORAGE storage, //IN the storage position of the phone book
    int start_index, // the start index of records, 0: read record by username.
    int end_index, // the end index of records
    uint8_t *username, // the username in a record, it is valid when start_index is 0
    QL_SIM_PHONE_BOOK_RECORDS_INFO   *records  // the list of records 
);


/*===========================================================================

  FUNCTION:  ql_sim_write_phonebook_record

===========================================================================*/
/*
    @brief
    Function will write  a record to a phone book.

    @return
    QL_SIM_ERROR_CODE
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_write_phonebook_record
(
    QL_SIM_PHONE_BOOK_STORAGE storage, //IN the storage position of the phone book
    QL_SIM_PHONE_BOOK_RECORD_INFO   *record //IN  a record
);


/*===========================================================================*/
/*
    @brief
    Function will return the state of currently selected phonebook

    @return
    QL_SIM_ERROR_CODE
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_get_phonebook_state(QL_SIM_PHONEBOOK_STATE *state);


/*===========================================================================*/
/*
    @brief
    Function will request to select the local or remote SIM

    @return
    QL_SIM_ERROR_CODE
*/
/*=========================================================================*/
QL_SIM_ERROR_CODE ql_sim_select(UINT service, UINT is_remote);


#ifdef __cplusplus
}
#endif

#endif 
