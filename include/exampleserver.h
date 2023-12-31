//
// --------------------------------------------------------------------------
//  Gurux Ltd
//
//
//
// Filename:        $HeadURL:  $
//
// Version:         $Revision:  $,
//                  $Date:  $
//                  $Author: $
//
// Copyright (c) Gurux Ltd
//
//---------------------------------------------------------------------------

#pragma once


#include "../../development/include/server.h"

#include "connection.h"


#define  SETTINGS_PATH 	"/root/Settings.txt"

#define SHAHAB_OLD_VERSION	"1"
#define SHAHAB_NEW_VERSION	"2"

typedef struct
{
	char SerialNumber[10];
	char ProductYear[5];
	char manufactureID[5];
	char IP[17];
	char PORT[7];
	char ListenPORT[7];
	char APN[22];
	char AuthKey[16];
	char BroadEncKey[16];
	char UniEncKey[16];
	char KEK[16];
	char LLSPass[10];
	char MDM[5];
}SETTINGS;
SETTINGS Settings;






typedef struct
{
	unsigned char		protocolVersion;

	uint32_t			dedicatedKey_capacity;
	uint32_t			dedicatedKey_position;
	uint32_t			dedicatedKey_size;
	unsigned char		dedicatedKey_data[100];

	unsigned char		sourceSystemTitle[8];

	DLMS_COMMAND		transaction_command;
	uint32_t			transaction_capacity;
	uint32_t			transaction_position;
	uint32_t			transaction_size;
	unsigned char		transaction_data[2000];

	uint32_t 				blockIndex;
	DLMS_CONNECTION_STATE 	connected;
	DLMS_AUTHENTICATION 	authentication;
	unsigned char 			isAuthenticationRequired;
	DLMS_SECURITY 			cipher_security;

	uint32_t			CtoS_capacity;
	uint32_t			CtoS_position;
	uint32_t			CtoS_size;
	unsigned char		CtoS_data[100];

	uint32_t			StoC_capacity;
	uint32_t			StoC_position;
	uint32_t			StoC_size;
	unsigned char		StoC_data[100];

	unsigned char 		senderFrame;
	unsigned char 		receiverFrame;

	uint32_t 			serverAddress;
	uint16_t			clientAddress;
	uint32_t 			dataReceived;
	uint32_t 			frameReceived;
	DLMS_CONFORMANCE 	negotiatedConformance;
	uint16_t 			maxPduSize;
}SvrSettings;












void Read_Settings(SETTINGS *settings);


char DATAFILE[FILENAME_MAX];
char IMAGEFILE[FILENAME_MAX];
char TRACEFILE[FILENAME_MAX];
void println(char* desc, gxByteBuffer* data);

int TCP_start(connection *con);

int IEC_start(connection *con, char *file);

int rs485_start(connection *con, char *file);
    
void srv_Save(dlmsServerSettings *settings);
void srv_Load(dlmsServerSettings *settings);


int svr_InitObjects(dlmsServerSettings *settings);


/**
* Check is data sent to this server.
*
* @param serverAddress
*            Server address.
* @param clientAddress
*            Client address.
* @return True, if data is sent to this server.
*/
unsigned char svr_isTarget(dlmsSettings *settings, unsigned long int serverAddress, unsigned long clientAddress);

/**
* Get attribute access level.
*/
DLMS_ACCESS_MODE svr_getAttributeAccess(
    dlmsSettings *settings,
    gxObject *obj,
    unsigned char index);

/**
* Get method access level.
*/
extern DLMS_METHOD_ACCESS_MODE svr_getMethodAccess(
    dlmsSettings *settings,
    gxObject *obj,
    unsigned char index);

/**
* called when client makes connection to the server.
*/
int svr_connected(
    dlmsServerSettings *settings);

/**
    * Client has try to made invalid connection. Password is incorrect.
    *
    * @param connectionInfo
    *            Connection information.
    */
int svr_invalidConnection(dlmsServerSettings *settings);

/**
* called when client clses connection to the server.
*/
int svr_disconnected(
    dlmsServerSettings *settings);

/**
    * Read selected item(s).
    *
    * @param args
    *            Handled read requests.
    */
void svr_preRead(
    dlmsSettings* settings,
    gxValueEventCollection* args);

/**
    * Write selected item(s).
    *
    * @param args
    *            Handled write requests.
    */
void svr_preWrite(
    dlmsSettings* settings,
    gxValueEventCollection* args);

/**
     * Action is occurred.
     *
     * @param args
     *            Handled action requests.
     */
void svr_preAction(
    dlmsSettings* settings,
    gxValueEventCollection* args);

/**
* Read selected item(s).
*
* @param args
*            Handled read requests.
*/
void svr_postRead(
    dlmsSettings* settings,
    gxValueEventCollection* args);

/**
* Write selected item(s).
*
* @param args
*            Handled write requests.
*/
void svr_postWrite(
    dlmsSettings* settings,
    gxValueEventCollection* args);

/**
* Action is occurred.
*
* @param args
*            Handled action requests.
*/
void svr_postAction(
    dlmsSettings* settings,
    gxValueEventCollection* args);

/**
    * Check whether the authentication and password are correct.
    *
    * @param authentication
    *            Authentication level.
    * @param password
    *            Password.
    * @return Source diagnostic.
    */
DLMS_SOURCE_DIAGNOSTIC svr_validateAuthentication(
    dlmsServerSettings* settings,
    DLMS_AUTHENTICATION authentication,
    gxByteBuffer* password);

/**
     * Find object.
     *
     * @param objectType
     *            Object type.
     * @param sn
     *            Short Name. In Logical name referencing this is not used.
     * @param ln
     *            Logical Name. In Short Name referencing this is not used.
     * @return Found object or NULL if object is not found.
     */
int svr_findObject(
    dlmsSettings* settings,
    DLMS_OBJECT_TYPE objectType,
    int sn,
    unsigned char* ln,
    gxValueEventArg *e);

void svr_preGet(dlmsSettings* settings,
    gxValueEventCollection* args);

void svr_postGet(dlmsSettings* settings,
    gxValueEventCollection* args);


/**
* This is reserved for future use.
*
* @param args
*            Handled data type requests.
*/
void svr_getDataType(
    dlmsSettings* settings,
    gxValueEventCollection* args);
