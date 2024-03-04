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

#include <stdio.h>
#include <stdlib.h> // malloc and free needs this or error is generated.


#include <stdio.h>
#include <pthread.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h> //Add support for sockets
#include <unistd.h>     //Add support for sockets
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include "../include/connection.h"
#include "../../development/include/converters.h"
#include "../../development/include/helpers.h"
#include "../../development/include/date.h"

#include "../include/exampleserver.h"
#include "../../development/include/cosem.h"
#include "../../development/include/gxkey.h"
#include "../../development/include/gxobjects.h"
#include "../../development/include/gxget.h"
#include "../../development/include/gxset.h"
// Add this if you want to send notify messages.
#include "../../development/include/notify.h"
// Add support for serialization.
#include "../../development/include/gxserializer.h"

#include "../../development/include/gxmem.h"

GX_TRACE_LEVEL trace = GX_TRACE_LEVEL_OFF;

const static char *FLAG_ID = "ZSS";
// Serialization version is increased every time when structure of serialized data is changed.
const static uint16_t SERIALIZATION_VERSION = 2;

// Space for client challenge.
static unsigned char C2S_CHALLENGE[64];
// Space for server challenge.
static unsigned char S2C_CHALLENGE[64];
// Allocate space for read list.
static gxValueEventArg events[10];

unsigned char testMode = 1;
int socket1 = -1;
uint32_t SERIAL_NUMBER = 1234;


//Define all of Client
#define PUBLIC_CLIENT 16
#define MANAGEMENT_CLIENT 1



// TODO: Allocate space where profile generic row values are serialized.
#define PDU_MAX_PROFILE_GENERIC_COLUMN_SIZE 100
#define HDLC_HEADER_SIZE 17
#define HDLC_BUFFER_SIZE 128
#define PDU_BUFFER_SIZE 512
#define WRAPPER_BUFFER_SIZE 8 + PDU_BUFFER_SIZE
// Buffer where frames are saved.
static unsigned char frameBuff[HDLC_BUFFER_SIZE + HDLC_HEADER_SIZE];
// Buffer where PDUs are saved.
static unsigned char pduBuff[PDU_BUFFER_SIZE];
static unsigned char replyFrame[HDLC_BUFFER_SIZE + HDLC_HEADER_SIZE];
// Define server system title.
static unsigned char SERVER_SYSTEM_TITLE[8] = {0};
time_t imageActionStartTime = 0;
gxImageActivateInfo IMAGE_ACTIVATE_INFO[1];
static gxByteBuffer reply;

extern connection lnWrapper, lniec;

uint32_t time_current(void)
{
    // Get current time somewhere.
    return (uint32_t)time(NULL);
}

uint32_t time_elapsed(void)
{
    return (uint32_t)clock() / (CLOCKS_PER_SEC / 1000);
}

// In this example we wait 5 seconds before image is verified or activated.
time_t imageActionStartTime;

static gxClock clock1;
//gxClock clock1;
static gxIecHdlcSetup hdlc;
static gxIecHdlcSetup hdlcelectricalrs485port;

static gxData securityreceiveframecounterbroadcastkey;

static gxData ldn;
static gxData eventCode;
static gxData unixTime;
static gxData invocationCounter;
static gxData activefirmwareid2;
static gxData activefirmwaresignature2;
//static gxData deviceid1, deviceid2, deviceid3, deviceid4, deviceid5, deviceid6, deviceid7;
static gxData deviceid1, deviceid2, deviceid3, deviceid4, deviceid5, deviceid7;
gxData deviceid6;
static gxData errorregister;
static gxData unreadlogfilesstatusregister;
static gxData alarmregister1;
static gxData alarmfilter1;
static gxData alarmregister2;
static gxData alarmfilter2;
static gxData eventparameter;
static gxData eventobjectfrauddetectionlog;
static gxData eventobjectcommunicationlog;
//static gxData gprskeepalivetimeinterval;
gxData gprskeepalivetimeinterval;
static gxData localauthenticationprotection;
gxData imei;

static gxAssociationLogicalName associationNone;
static gxAssociationLogicalName associationLow;
static gxAssociationLogicalName associationHigh;
static gxAssociationLogicalName associationHighGMac;

static gxRegister activePowerL1;
static gxRegister clocktimeshiftlimit;

static gxScriptTable scriptTableGlobalMeterReset;
static gxScriptTable scriptTableDisconnectControl;
static gxScriptTable scriptTableActivateTestMode;
static gxScriptTable scriptTableActivateNormalMode;
static gxProfileGeneric eventLog;
static gxActionSchedule actionScheduleDisconnectOpen;
static gxActionSchedule actionScheduleDisconnectClose;
static gxActionSchedule imagetransferactivationSchedule;

gxPushSetup pushSetup;
static gxDisconnectControl disconnectControl;
static gxProfileGeneric loadProfile;
static gxProfileGeneric standardeventlog;
static gxProfileGeneric frauddetectionlog;
static gxProfileGeneric communicationlog;

static gxSapAssignment sapAssignment;
// Security Setup High is for High authentication.
static gxSecuritySetup securitySetupHigh;
gxSecuritySetup securitySetupManagementClient;

// Security Setup HighGMac is for GMac authentication.
static gxSecuritySetup securitySetupHighGMac;

gxImageTransfer imageTransfer;
gxAutoConnect autoConnect;
gxActivityCalendar activityCalendar;
gxLocalPortSetup localPortSetup;
gxRegisterMonitor registerMonitor;
gxAutoAnswer autoAnswer;
gxModemConfiguration modemConfiguration;
gxTcpUdpSetup udpSetup;
gxIp4Setup ip4Setup;

gxPppSetup pppSetup;
gxpppSetupLcpOption lcp[9];
gxpppSetupIPCPOption ipcp[6];

gxScriptAction script[9];
gxScript arr[7];

gxGPRSSetup gprsSetup;
static unsigned char APN[15];
gxScriptTable tarifficationScriptTable;
gxScriptTable pushscripttable;
gxScriptTable predefinedscriptsimageactivation;

gxRegisterActivation registerActivation;
gxCompactData compactData;

dlmsVARIANT gprs_kat_dlmsVar[3];
variantArray gprs_kat_VarArr;

dlmsVARIANT local_auth_dlmsVar[2];
variantArray local_auth_VarArr;

static gxObject *ALL_OBJECTS[] = {
    //these objects at the faham list
    BASE(securitySetupManagementClient),
    BASE(securityreceiveframecounterbroadcastkey),
    BASE(invocationCounter),
    BASE(ldn),
    BASE(deviceid1),
    BASE(deviceid2),
    BASE(deviceid3),
    BASE(deviceid4),
    BASE(deviceid5),
    BASE(deviceid6),
    BASE(deviceid7),
    BASE(clock1),
    BASE(clocktimeshiftlimit),
    BASE(imageTransfer),
    BASE(imagetransferactivationSchedule),
    BASE(predefinedscriptsimageactivation),
    BASE(activefirmwareid2),
    BASE(activefirmwaresignature2),
    BASE(hdlc),
    BASE(localPortSetup),
    BASE(hdlcelectricalrs485port),
    BASE(udpSetup),
    BASE(ip4Setup),
    BASE(pppSetup),
    BASE(errorregister),
    BASE(unreadlogfilesstatusregister),
    BASE(alarmregister1),
    BASE(alarmfilter1),
    BASE(alarmregister2),
    BASE(alarmfilter2),
    BASE(eventCode),
    BASE(eventparameter),
    BASE(standardeventlog),
    BASE(eventobjectfrauddetectionlog),
    BASE(frauddetectionlog),
    BASE(eventobjectcommunicationlog),
    BASE(communicationlog),
    BASE(pushSetup),
    BASE(pushscripttable),
    BASE(autoConnect),
    BASE(gprsSetup),
    BASE(modemConfiguration),
    BASE(autoAnswer),
    BASE(gprskeepalivetimeinterval),
    BASE(localauthenticationprotection),
    BASE(imei),
    //these objects not at the faham list. if deleted = ERROR
    BASE(associationNone),
    BASE(associationLow),
    BASE(associationHigh),
    BASE(associationHighGMac),
    BASE(securitySetupHigh),
    BASE(securitySetupHighGMac),
    BASE(sapAssignment),
    BASE(activePowerL1),
    BASE(scriptTableGlobalMeterReset),
    BASE(scriptTableActivateTestMode),
    BASE(scriptTableActivateNormalMode),
    BASE(loadProfile),
    BASE(eventLog),
    BASE(disconnectControl),
    BASE(actionScheduleDisconnectOpen),
    BASE(actionScheduleDisconnectClose),
    BASE(unixTime),
    BASE(activityCalendar),
    BASE(registerMonitor),
    BASE(tarifficationScriptTable),
    BASE(registerActivation),
};



gxObject *ASSOCIATION_NONE_OBJECTS[] = {
    BASE(securityreceiveframecounterbroadcastkey),
    BASE(invocationCounter),
    BASE(ldn),
    BASE(deviceid7),
};



gxObject *ASSOCIATION_HighGMAC_OBJECTS[] = {
    BASE(securitySetupManagementClient),
    BASE(securityreceiveframecounterbroadcastkey),
    BASE(invocationCounter),
    BASE(ldn),
    BASE(deviceid1),
    BASE(deviceid2),
    BASE(deviceid3),
    BASE(deviceid4),
    BASE(deviceid5),
    BASE(deviceid6),
    BASE(deviceid7),
    BASE(clock1),
    BASE(clocktimeshiftlimit),
    BASE(imageTransfer),
    BASE(imagetransferactivationSchedule),
    BASE(predefinedscriptsimageactivation),
    BASE(activefirmwareid2),
    BASE(activefirmwaresignature2),
    BASE(hdlc),
    BASE(localPortSetup),
    BASE(hdlcelectricalrs485port),
    BASE(udpSetup),
    BASE(ip4Setup),
    BASE(pppSetup),
    BASE(errorregister),
    BASE(unreadlogfilesstatusregister),
    BASE(alarmregister1),
    BASE(alarmfilter1),
    BASE(alarmregister2),
    BASE(alarmfilter2),
    BASE(eventCode),
    BASE(eventparameter),
    BASE(standardeventlog),
    BASE(eventobjectfrauddetectionlog),
    BASE(frauddetectionlog),
    BASE(eventobjectcommunicationlog),
    BASE(communicationlog),
    BASE(pushSetup),
    BASE(pushscripttable),
    BASE(autoConnect),
    BASE(gprsSetup),
    BASE(modemConfiguration),
    BASE(autoAnswer),
    BASE(gprskeepalivetimeinterval),
    BASE(localauthenticationprotection),
    BASE(imei),
};


////////////////////////////////////////////////////
// Define what is serialized to decrease EEPROM usage.
gxSerializerIgnore NON_SERIALIZED_OBJECTS[] = {
    // Nothing is saved when authentication is not used.
    IGNORE_ATTRIBUTE(BASE(associationNone), GET_ATTRIBUTE_ALL()),
    // Only password is saved for low and high authentication.
    IGNORE_ATTRIBUTE(BASE(associationLow), GET_ATTRIBUTE_EXCEPT(7)),
    IGNORE_ATTRIBUTE(BASE(associationHigh), GET_ATTRIBUTE_EXCEPT(7)),
    // Only scaler and unit are saved for all register objects.
    IGNORE_ATTRIBUTE_BY_TYPE(DLMS_OBJECT_TYPE_REGISTER, GET_ATTRIBUTE(2)),
    // Objects are not load because they are created statically.
    IGNORE_ATTRIBUTE_BY_TYPE(DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, GET_ATTRIBUTE(2))};

static uint32_t executeTime = 0;

static uint16_t activePowerL1Value = 0;

typedef enum
{
    // Meter is powered.
    GURUX_EVENT_CODES_POWER_UP = 0x1,
    // User has change the time.
    GURUX_EVENT_CODES_TIME_CHANGE = 0x2,
    // DST status is changed.
    GURUX_EVENT_CODES_DST = 0x4,
    // Push message is sent.
    GURUX_EVENT_CODES_PUSH = 0x8,
    // Meter makes auto connect.
    GURUX_EVENT_CODES_AUTO_CONNECT = 0x10,
    // User has change the password.
    GURUX_EVENT_CODES_PASSWORD_CHANGED = 0x20,
    // Wrong password tried 3 times.
    GURUX_EVENT_CODES_WRONG_PASSWORD = 0x40,
    // Disconnect control state is changed.
    GURUX_EVENT_CODES_OUTPUT_RELAY_STATE = 0x80,
    // User has reset the meter.
    GURUX_EVENT_CODES_GLOBAL_METER_RESET = 0x100
} GURUX_EVENT_CODES;

/////////////////////////////////////////////////////////////////////////////
// Save security settings to the EEPROM.        keepAliveTime keep;

//
// Only updated value is saved. This is done because write to EEPROM is slow
// and there is a limit how many times value can be written to the EEPROM.
/////////////////////////////////////////////////////////////////////////////
int saveSecurity(
    dlmsSettings *settings)
{
    int ret = 0;
    const char *fileName = "/usr/bin/ZS-DMG/security.raw";
    // Save keys to own block in EEPROM.
#if _MSC_VER > 1400
    FILE *f = NULL;
    fopen_s(&f, fileName, "wb");
#else
    FILE *f = fopen(fileName, "wb");
#endif
    gxByteBuffer bb;
    bb_init(&bb);
    bb_capacity(&bb, 256);
    if (f != NULL)
    {
        if ((ret = bb_set(&bb, settings->cipher.blockCipherKey.data, 16)) == 0 &&
            (ret = bb_set(&bb, settings->cipher.authenticationKey.data, 16)) == 0 &&
            (ret = bb_set(&bb, settings->kek.data, 16)) == 0 &&
            // Save server IV.
            (ret = bb_setUInt32(&bb, settings->cipher.invocationCounter)) == 0 &&
            // Save last client IV.
            (ret = bb_setUInt32(&bb, securitySetupHighGMac.minimumInvocationCounter)) == 0)
        {
            fwrite(bb.data, 1, bb.size, f);
        }
        bb_clear(&bb);
        fclose(f);
    }
    else
    {
        printf("%s\r\n", "Failed to open keys file.");
    }
    return ret;
}

/////////////////////////////////////////////////////////////////////////////
// Save data to the EEPROM.
//
// Only updated value is saved. This is done because write to EEPROM is slow
// and there is a limit how many times value can be written to the EEPROM.
/////////////////////////////////////////////////////////////////////////////
int saveSettings()
{
    int ret = 0;
    const char *fileName = "/usr/bin/ZS-DMG/settings.raw";
    // Save keys to own block in EEPROM.
#if _MSC_VER > 1400
    FILE *f = NULL;
    fopen_s(&f, fileName, "wb");
#else
    FILE *f = fopen(fileName, "wb");
#endif
    if (f != NULL)
    {
        gxSerializerSettings serializerSettings;
        ser_init(&serializerSettings);
        serializerSettings.stream = f;
        serializerSettings.ignoredAttributes = NON_SERIALIZED_OBJECTS;
        serializerSettings.count = sizeof(NON_SERIALIZED_OBJECTS) / sizeof(NON_SERIALIZED_OBJECTS[0]);
        ret = ser_saveObjects(&serializerSettings, ALL_OBJECTS, sizeof(ALL_OBJECTS) / sizeof(ALL_OBJECTS[0]));
        printf("[INFO]-[exampleserver.c]-[saveSettings]-[ser_saveObjects - ret:%d]\n", ret);
        fclose(f);
    }
    else
    {
        printf("%s\r\n", "Failed to open settings file.");
    }
    return ret;
}

// Allocate profile generic buffer.
void allocateProfileGenericBuffer(const char *fileName, uint32_t size)
{
    uint32_t pos;
    FILE *f = NULL;
#if _MSC_VER > 1400
    fopen_s(&f, fileName, "ab");
#else
    f = fopen(fileName, "ab");
#endif
    if (f != NULL)
    {
        fseek(f, 0, SEEK_END);
        if (ftell(f) == 0)
        {
            for (pos = 0; pos != size; ++pos)
            {
                if (fputc(0x00, f) != 0)
                {
                    printf("Error Writing to %s\n", fileName);
                    break;
                }
            }
        }
        fclose(f);
    }
}

int getProfileGenericFileName(gxProfileGeneric *pg, char *fileName)
{
    int ret = hlp_getLogicalNameToString(pg->base.logicalName, fileName);
    strcat(fileName, ".raw");
    return ret;
}

// Returns profile generic buffer column sizes.
int getProfileGenericBufferColumnSizes(
    dlmsSettings *settings,
    gxProfileGeneric *pg,
    DLMS_DATA_TYPE *dataTypes,
    uint8_t *columnSizes,
    uint16_t *rowSize)
{
    int ret = 0;
    uint8_t pos;
    gxKey *it;
    gxValueEventArg e;
    ve_init(&e);
    *rowSize = 0;
    uint16_t size;
    unsigned char type;
    // Loop capture columns and get values.
    for (pos = 0; pos != pg->captureObjects.size; ++pos)
    {
        if ((ret = arr_getByIndex(&pg->captureObjects, (uint16_t)pos, (void **)&it)) != 0)
        {
            break;
        }
        // Date time is saved in EPOCH to save space.
        if (((gxObject *)it->key)->objectType == DLMS_OBJECT_TYPE_CLOCK && ((gxTarget *)it->value)->attributeIndex == 2)
        {
            type = DLMS_DATA_TYPE_UINT32;
            size = 4;
        }
        else
        {
            e.target = (gxObject *)it->key;
            e.index = ((gxTarget *)it->value)->attributeIndex;
            if ((ret = cosem_getValue(settings, &e)) != 0)
            {
                break;
            }
            if (bb_size(e.value.byteArr) != 0)
            {
                if ((ret = bb_getUInt8(e.value.byteArr, &type)) != 0)
                {
                    break;
                }
                size = bb_available(e.value.byteArr);
            }
            else
            {
                type = DLMS_DATA_TYPE_NONE;
                size = 0;
            }
        }
        if (dataTypes != NULL)
        {
            dataTypes[pos] = type;
        }
        if (columnSizes != NULL)
        {
            columnSizes[pos] = (uint8_t)size;
        }
        *rowSize += (uint16_t)size;
        ve_clear(&e);
    }
    ve_clear(&e);
    return ret;
}

// Get max row count for allocated buffer.
uint16_t getProfileGenericBufferMaxRowCount(
    dlmsSettings *settings,
    gxProfileGeneric *pg)
{
    uint16_t count = 0;
    char fileName[30];
    // Allocate space for load profile buffer.
    getProfileGenericFileName(pg, fileName);
    uint16_t rowSize = 0;
    FILE *f = NULL;
#if _MSC_VER > 1400
    fopen_s(&f, fileName, "r+b");
#else
    f = fopen(fileName, "r+b");
#endif
    if (f == NULL)
    {
        // Allocate space for the profile generic buffer.
        allocateProfileGenericBuffer(fileName, 1024);
#if _MSC_VER > 1400
        fopen_s(&f, fileName, "r+b");
#else
        f = fopen(fileName, "r+b");
#endif
    }
    if (f != NULL)
    {
        getProfileGenericBufferColumnSizes(settings, pg, NULL, NULL, &rowSize);
        if (rowSize != 0)
        {
            fseek(f, 0L, SEEK_END);
            count = (uint16_t)ftell(f);
            // Decrease current index and total amount of the entries.
            count -= 4;
            count /= rowSize;
        }
        fclose(f);
    }
    return count;
}

// Get current row count for allocated buffer.
uint16_t getProfileGenericBufferEntriesInUse(gxProfileGeneric *pg)
{
    uint16_t index = 0;
    int ret = 0;
    char fileName[30];
    getProfileGenericFileName(pg, fileName);
    FILE *f = NULL;
#if _MSC_VER > 1400
    fopen_s(&f, fileName, "r+b");
#else
    f = fopen(fileName, "r+b");
#endif
    if (f != NULL)
    {
        uint16_t dataSize = 0;
        // Load current entry index from the begin of the data.
        unsigned char pduBuff[2];
        gxByteBuffer pdu;
        bb_attach(&pdu, pduBuff, 0, sizeof(pduBuff));
        if (fread(pdu.data, 1, 2, f) == 2)
        {
            pdu.size = 2;
            bb_getUInt16(&pdu, &index);
            fseek(f, 0, SEEK_SET);
            bb_empty(&pdu);
        }
        fclose(f);
    }
    return index;
}

int captureProfileGeneric(
    dlmsSettings *settings,
    gxProfileGeneric *pg)
{
    unsigned char pos;
    gxKey *it;
    int ret = 0;
    char fileName[30];
    getProfileGenericFileName(pg, fileName);
    unsigned char pduBuff[PDU_MAX_PROFILE_GENERIC_COLUMN_SIZE];
    gxByteBuffer pdu;
    bb_attach(&pdu, pduBuff, 0, sizeof(pduBuff));
    gxValueEventArg e;
    ve_init(&e);
    FILE *f = NULL;
#if _MSC_VER > 1400
    fopen_s(&f, fileName, "r+b");
#else
    f = fopen(fileName, "r+b");
#endif
    if (f != NULL)
    {
        uint16_t dataSize = 0;
        uint8_t columnSizes[10];
        DLMS_DATA_TYPE dataTypes[10];
        // Load current entry index from the begin of the data.
        uint16_t index = 0;
        if (fread(pdu.data, 1, 2, f) == 2)
        {
            pdu.size = 2;
            bb_getUInt16(&pdu, &index);
            fseek(f, 0, SEEK_SET);
            bb_empty(&pdu);
        }
        // Current index in ring buffer.
        if (pg->profileEntries != 0)
        {
            bb_setUInt16(&pdu, (1 + index) % (pg->profileEntries));
        }

        // Update how many entries is used until buffer is full.
        if (ret == 0 && pg->entriesInUse != pg->profileEntries)
        {
            // Total amount of the entries.
            ++pg->entriesInUse;
        }
        bb_setUInt16(&pdu, (uint16_t)pg->entriesInUse);
        // Update values to the EEPROM.
        fwrite(pdu.data, 1, 4, f);
        getProfileGenericBufferColumnSizes(settings, pg, dataTypes, columnSizes, &dataSize);
        if (index != 0 && pg->profileEntries != 0)
        {
            fseek(f, 4 + ((index % pg->profileEntries) * dataSize), SEEK_SET);
        }
        // Loop capture columns and get values.
        for (pos = 0; pos != pg->captureObjects.size; ++pos)
        {
            if ((ret = arr_getByIndex(&pg->captureObjects, pos, (void **)&it)) != 0)
            {
                break;
            }
            bb_clear(&pdu);
            // Date time is saved in EPOCH to save space.
            if ((((gxObject *)it->key)->objectType == DLMS_OBJECT_TYPE_CLOCK || ((gxObject *)it->key) == BASE(unixTime)) &&
                ((gxTarget *)it->value)->attributeIndex == 2)
            {
                e.value.ulVal = time_current();
                e.value.vt = DLMS_DATA_TYPE_UINT32;
                fwrite(&e.value.bVal, 4, 1, f);
            }
            else
            {
                e.target = (gxObject *)it->key;
                e.index = ((gxTarget *)it->value)->attributeIndex;
                e.value.byteArr = &pdu;
                e.value.vt = DLMS_DATA_TYPE_OCTET_STRING;
                if ((ret = cosem_getValue(settings, &e)) != 0)
                {
                    break;
                }
                // Data type is not serialized. For that reason first byte is ignored.
                fwrite(&e.value.byteArr->data[1], e.value.byteArr->size - 1, 1, f);
            }
        }
        fclose(f);
        if (ret != 0)
        {
            // Total amount of the entries.
            --pg->entriesInUse;
        }
    }
    // Append data.
    return ret;
}

void updateState(
    dlmsSettings *settings,
    uint16_t value)
{
    GX_UINT16(eventCode.value) = value;
    captureProfileGeneric(settings, &eventLog);
}
///////////////////////////////////////////////////////////////////////
// Write trace to the serial port.
//
// This can be used for debugging.
///////////////////////////////////////////////////////////////////////
void GXTRACE(const char *str, const char *data)
{
    // Send trace to the serial port in test mode.
    if (testMode)
    {
        if (data == NULL)
        {
            printf("%s\r\n", str);
        }
        else
        {
            printf("%s %s\r\n", str, data);
        }
    }
}

///////////////////////////////////////////////////////////////////////
// Write trace to the serial port.
//
// This can be used for debugging.
///////////////////////////////////////////////////////////////////////
void GXTRACE_INT(const char *str, int32_t value)
{
    char data[10];
    sprintf(data, " %ld", value);
    GXTRACE(str, data);
}

///////////////////////////////////////////////////////////////////////
// Write trace to the serial port.
//
// This can be used for debugging.
///////////////////////////////////////////////////////////////////////
void GXTRACE_LN(const char *str, uint16_t type, unsigned char *ln)
{
    char buff[30];
    sprintf(buff, "%d %d.%d.%d.%d.%d.%d", type, ln[0], ln[1], ln[2], ln[3], ln[4], ln[5]);
    GXTRACE(str, buff);
}

// Returns current time.
// If you are not using operating system you have to implement this by yourself.
// Reason for this is that all compilers's or HWs don't support time at all.
void time_now(
    gxtime *value, unsigned char meterTime)
{
	extern DS1307_I2C_STRUCT_TYPEDEF DS1307_Str;

	DS1307_Get_Time(&DS1307_Str);

	time_initUnix(value, (unsigned long)time(NULL));

    // If date time is wanted in meter time.
    if (meterTime)
    {
        clock_utcToMeterTime(&clock1, value);
    }

    uint16_t 	j_y;
    uint8_t 	j_m;
    uint8_t 	j_d;
	uint16_t  	g_y=((uint16_t) DS1307_Str.year) + 2000;
	uint8_t  	g_m=DS1307_Str.month;
	uint8_t  	g_d=DS1307_Str.date;
    M2Sh (&j_y, &j_m, &j_d, g_y, g_m, g_d);

	value->value.tm_year= j_y - 1900;
	value->value.tm_mon = j_m - 1;
	value->value.tm_mday= j_d;
	value->value.tm_hour= DS1307_Str.hour;
	value->value.tm_min = DS1307_Str.minute;
	value->value.tm_sec = DS1307_Str.second;

    value->deviation 	= 0;
    value->extraInfo 	= DLMS_DATE_TIME_EXTRA_INFO_NONE;
	value->skip 		= DATETIME_SKIPS_NONE;
	value->status 		= DLMS_CLOCK_STATUS_OK;

//	printf("Time now =========================> Y.M.D - HH:MM:SS = %d.%d.%d - %d:%d:%d\n",
//			value->value.tm_year,
//			value->value.tm_mon,
//			value->value.tm_mday,
//			value->value.tm_hour,
//			value->value.tm_min,
//			value->value.tm_sec);
}

void println(char *desc, gxByteBuffer *data)
{
    if (data != NULL)
    {
        char *str = bb_toHexString(data);
        printf("%s: %s\r\n", desc, str);
        free(str);
    }
}

///////////////////////////////////////////////////////////////////////
// This method adds example Logical Name Association object.
///////////////////////////////////////////////////////////////////////
int addAssociationNone()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 40, 0, 1, 255};
    if ((ret = INIT_OBJECT(associationNone, DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, ln)) == 0)
    {
        // All objects are shown also without authentication.
        OA_ATTACH(associationNone.objectList, ASSOCIATION_NONE_OBJECTS);
        // Uncomment this if you want to show only part of the objects without authentication.
        // OA_ATTACH(associationNone.objectList, NONE_OBJECTS);
        associationNone.authenticationMechanismName.mechanismId = DLMS_AUTHENTICATION_NONE;
        associationNone.clientSAP = 0x10;
        // Max PDU is half of PDU size. This is for demonstration purposes only.
        associationNone.xDLMSContextInfo.maxSendPduSize = associationNone.xDLMSContextInfo.maxReceivePduSize = PDU_BUFFER_SIZE / 2;
        associationNone.xDLMSContextInfo.conformance = (DLMS_CONFORMANCE)(DLMS_CONFORMANCE_GET | DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_GET_OR_READ);
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// This method adds example Logical Name Association object.
///////////////////////////////////////////////////////////////////////
int addAssociationLow()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 40, 0, 2, 255};
    if ((ret = INIT_OBJECT(associationLow, DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, ln)) == 0)
    {
        // Only Logical Device Name is add to this Association View.
        OA_ATTACH(associationLow.objectList, ASSOCIATION_NONE_OBJECTS);
        associationLow.authenticationMechanismName.mechanismId = DLMS_AUTHENTICATION_LOW;
        associationLow.clientSAP = 0x11;
        associationLow.xDLMSContextInfo.maxSendPduSize = associationLow.xDLMSContextInfo.maxReceivePduSize = PDU_BUFFER_SIZE;
        associationLow.xDLMSContextInfo.conformance = (DLMS_CONFORMANCE)(DLMS_CONFORMANCE_GENERAL_PROTECTION |
                                                                              DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_SET_OR_WRITE |
                                                                              DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_GET_OR_READ |
                                                                              DLMS_CONFORMANCE_SET |
                                                                              DLMS_CONFORMANCE_SELECTIVE_ACCESS |
                                                                              DLMS_CONFORMANCE_ACTION |
                                                                              DLMS_CONFORMANCE_MULTIPLE_REFERENCES |
																			  DLMS_CONFORMANCE_DATA_NOTIFICATION |
                                                                              DLMS_CONFORMANCE_GET);
        bb_addString(&associationLow.secret, Settings.LLSPass);
        associationLow.securitySetup = NULL;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// This method adds example Logical Name Association object for High authentication.
//  UA in Indian standard.
///////////////////////////////////////////////////////////////////////
int addAssociationHigh()
{
    int ret;
    // Dedicated key.
    static unsigned char CYPHERING_INFO[20] = {0};
    const unsigned char ln[6] = {0, 0, 40, 0, 3, 255};
    if ((ret = INIT_OBJECT(associationHigh, DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, ln)) == 0)
    {
        associationHigh.authenticationMechanismName.mechanismId = DLMS_AUTHENTICATION_HIGH;
        OA_ATTACH(associationHigh.objectList, ASSOCIATION_NONE_OBJECTS);
        BB_ATTACH(associationHigh.xDLMSContextInfo.cypheringInfo, CYPHERING_INFO, 0);
        associationHigh.clientSAP = 0x12;
        associationHigh.xDLMSContextInfo.maxSendPduSize = associationHigh.xDLMSContextInfo.maxReceivePduSize = PDU_BUFFER_SIZE;
        associationHigh.xDLMSContextInfo.conformance = (DLMS_CONFORMANCE)(DLMS_CONFORMANCE_GENERAL_PROTECTION |
                                                                              DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_SET_OR_WRITE |
                                                                              DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_GET_OR_READ |
                                                                              DLMS_CONFORMANCE_SET |
                                                                              DLMS_CONFORMANCE_SELECTIVE_ACCESS |
                                                                              DLMS_CONFORMANCE_ACTION |
                                                                              DLMS_CONFORMANCE_MULTIPLE_REFERENCES |
																			  DLMS_CONFORMANCE_DATA_NOTIFICATION |
                                                                              DLMS_CONFORMANCE_GET);
        bb_addString(&associationHigh.secret, "Gurux");
#ifndef DLMS_IGNORE_OBJECT_POINTERS
        associationHigh.securitySetup = &securitySetupHigh;
#else
        memcpy(associationHigh.securitySetupReference, securitySetupHigh.base.logicalName, 6);
#endif // DLMS_IGNORE_OBJECT_POINTERS
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// This method adds example Logical Name Association object for GMAC High authentication.
//  UA in Indian standard.
///////////////////////////////////////////////////////////////////////
int addAssociationHighGMac()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 40, 0, 4, 255};
    if ((ret = INIT_OBJECT(associationHighGMac, DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, ln)) == 0)
    {
        associationHighGMac.authenticationMechanismName.mechanismId = DLMS_AUTHENTICATION_HIGH_GMAC;
        OA_ATTACH(associationHighGMac.objectList, ASSOCIATION_HighGMAC_OBJECTS);

        associationHighGMac.clientSAP = 0x1;
        associationHighGMac.xDLMSContextInfo.maxSendPduSize = associationHighGMac.xDLMSContextInfo.maxReceivePduSize = PDU_BUFFER_SIZE;
        associationHighGMac.xDLMSContextInfo.conformance = (DLMS_CONFORMANCE)(DLMS_CONFORMANCE_GENERAL_PROTECTION |
                                                                              DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_SET_OR_WRITE |
                                                                              DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_GET_OR_READ |
                                                                              DLMS_CONFORMANCE_SET |
                                                                              DLMS_CONFORMANCE_SELECTIVE_ACCESS |
                                                                              DLMS_CONFORMANCE_ACTION |
                                                                              DLMS_CONFORMANCE_MULTIPLE_REFERENCES |
																			  DLMS_CONFORMANCE_DATA_NOTIFICATION |

//																			  DLMS_CONFORMANCE_GENERAL_BLOCK_TRANSFER |
//																			  DLMS_CONFORMANCE_ATTRIBUTE_0_SUPPORTED_WITH_SET |
//																			  DLMS_CONFORMANCE_PRIORITY_MGMT_SUPPORTED |
//																			  DLMS_CONFORMANCE_ATTRIBUTE_0_SUPPORTED_WITH_GET |
//																			  DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_ACTION |
//																			  DLMS_CONFORMANCE_EVENT_NOTIFICATION |




                                                                              DLMS_CONFORMANCE_GET);
        // GMAC authentication don't need password.
#ifndef DLMS_IGNORE_OBJECT_POINTERS
        associationHighGMac.securitySetup = &securitySetupHighGMac;
#else
        memcpy(associationHighGMac.securitySetupReference, securitySetupHigh.base.logicalName, 6);
#endif // DLMS_IGNORE_OBJECT_POINTERS
    }
    return ret;
}



///////////////////////////////////////////////////////////////////////
// This method adds security setup object for High authentication.
///////////////////////////////////////////////////////////////////////
int addSecuritySetupHigh()
{
    int ret;
    // Define client system title.
    static unsigned char CLIENT_SYSTEM_TITLE[8] = {0};
    const unsigned char ln[6] = {0, 0, 43, 0, 1, 255};
    if ((ret = INIT_OBJECT(securitySetupHigh, DLMS_OBJECT_TYPE_SECURITY_SETUP, ln)) == 0)
    {
        BB_ATTACH(securitySetupHigh.serverSystemTitle, SERVER_SYSTEM_TITLE, 8);
        BB_ATTACH(securitySetupHigh.clientSystemTitle, CLIENT_SYSTEM_TITLE, 8);
        securitySetupHigh.securityPolicy = DLMS_SECURITY_POLICY_NOTHING;
        securitySetupHigh.securitySuite = DLMS_SECURITY_SUITE_V0;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// This method adds security setup object for High GMAC authentication.
///////////////////////////////////////////////////////////////////////
int addSecuritySetupHighGMac()
{
    int ret;
    // Define client system title.
    static unsigned char CLIENT_SYSTEM_TITLE[8] = {0};
    static unsigned char SERVER_SYSTEM_TITLE[8] = {'Z','S','S',0x31, 0,0,0,0};
    const unsigned char ln[6] = {0, 0, 43, 0, 2, 255};
    if ((ret = INIT_OBJECT(securitySetupHighGMac, DLMS_OBJECT_TYPE_SECURITY_SETUP, ln)) == 0)
    {

        unsigned char hexBytes[4];
        for (int i = 0; i < 4; i++)
        {
            hexBytes[i] = (SERIAL_NUMBER >> (i * 8)) & 0xFF;
        }
        hexBytes[3] |= 0x40;

        SERVER_SYSTEM_TITLE[4] = hexBytes[3] ;
        SERVER_SYSTEM_TITLE[4] |= 0x0;
        SERVER_SYSTEM_TITLE[5] = hexBytes[2] ;
        SERVER_SYSTEM_TITLE[5] |= 0x0;

        SERVER_SYSTEM_TITLE[6] = hexBytes[1] ;
        SERVER_SYSTEM_TITLE[6] |= 0x0;

        SERVER_SYSTEM_TITLE[7] = hexBytes[0] ;
        SERVER_SYSTEM_TITLE[7] |= 0x0;





        BB_ATTACH(securitySetupHighGMac.serverSystemTitle, SERVER_SYSTEM_TITLE, 8);
        BB_ATTACH(securitySetupHighGMac.clientSystemTitle, CLIENT_SYSTEM_TITLE, 8);
        // Only Authenticated encrypted connections are allowed.
        securitySetupHighGMac.securityPolicy = DLMS_SECURITY_POLICY_NOTHING;
        securitySetupHighGMac.securitySuite = DLMS_SECURITY_SUITE_V0;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// Add Security Setup Management Client
///////////////////////////////////////////////////////////////////////
int addSecuritySetupManagementClient()
{
    int ret;
    // Define client system title.
    static unsigned char CLIENT_SYSTEM_TITLE[8] = {0};
    static unsigned char SERVER_SYSTEM_TITLE[8] = {'Z','S','S',0x31, 0,0,0,0};

    const unsigned char ln[6] = {0, 0, 43, 0, 0, 255};
    if ((ret = INIT_OBJECT(securitySetupManagementClient, DLMS_OBJECT_TYPE_SECURITY_SETUP, ln)) == 0)
    {
//    	printf("SERIAL_NUMBER in addSecuritySetupManagementClient = %d\n",SERIAL_NUMBER);
        unsigned char hexBytes[4];
        for (int i = 0; i < 4; i++)
        {
            hexBytes[i] = (SERIAL_NUMBER >> (i * 8)) & 0xFF;
        }
        hexBytes[3] |= 0x40;

        SERVER_SYSTEM_TITLE[4] = hexBytes[3] ;
        SERVER_SYSTEM_TITLE[4] |= 0x0;
        SERVER_SYSTEM_TITLE[5] = hexBytes[2] ;
        SERVER_SYSTEM_TITLE[5] |= 0x0;

        SERVER_SYSTEM_TITLE[6] = hexBytes[1] ;
        SERVER_SYSTEM_TITLE[6] |= 0x0;

        SERVER_SYSTEM_TITLE[7] = hexBytes[0] ;
        SERVER_SYSTEM_TITLE[7] |= 0x0;


         BB_ATTACH(securitySetupManagementClient.serverSystemTitle, SERVER_SYSTEM_TITLE, 8);
//         BB_ATTACH(securitySetupManagementClient.clientSystemTitle, CLIENT_SYSTEM_TITLE, 8);
         securitySetupManagementClient.securityPolicy = DLMS_SECURITY_POLICY_NOTHING;
         securitySetupManagementClient.securitySuite = DLMS_SECURITY_SUITE_V0;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// Add Clock Time Shift Limit
///////////////////////////////////////////////////////////////////////
int addClockTimeShiftLimit()
{
    const unsigned char ln[6] = {1, 0, 0, 9, 11, 255};
    INIT_OBJECT(clocktimeshiftlimit, DLMS_OBJECT_TYPE_REGISTER, ln);
    // 10 ^ 3 =  1000
    clocktimeshiftlimit.value.vt = DLMS_DATA_TYPE_UINT32;
    clocktimeshiftlimit.value.ulVal = 60;

    clocktimeshiftlimit.scaler = 1;
    clocktimeshiftlimit.unit = 5;

    return 0;
}

///////////////////////////////////////////////////////////////////////
// This method adds example register object.
///////////////////////////////////////////////////////////////////////
int addRegisterObject()
{
    const unsigned char ln[6] = {1, 1, 21, 25, 0, 255};
    INIT_OBJECT(activePowerL1, DLMS_OBJECT_TYPE_REGISTER, ln);
    // 10 ^ 3 =  1000
    GX_UINT16_BYREF(activePowerL1.value, activePowerL1Value);
    activePowerL1.scaler = -2;
    activePowerL1.unit = 30;
    return 0;
}

uint16_t readActivePowerValue()
{
    return ++activePowerL1Value;
}

uint16_t readEventCode()
{
    return eventCode.value.uiVal;
}

///////////////////////////////////////////////////////////////////////
// Add tariffication script table object.
///////////////////////////////////////////////////////////////////////
int addTarifficationScriptTable()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 10, 0, 100, 255};
    if ((ret = INIT_OBJECT(tarifficationScriptTable, DLMS_OBJECT_TYPE_SCRIPT_TABLE, ln)) == 0)
    {
        gxScript *s1 = malloc(sizeof(gxScript));
        gxScript *s2 = (gxScript *)malloc(sizeof(gxScript));
        s1->id = 1;
        arr_init(&s1->actions);
        s2->id = 2;
        arr_init(&s2->actions);
        arr_push(&tarifficationScriptTable.scripts, s1);
        arr_push(&tarifficationScriptTable.scripts, s2);
        gxScriptAction *a = (gxScriptAction *)malloc(sizeof(gxScriptAction));
        arr_push(&s1->actions, a);
        a->type = DLMS_SCRIPT_ACTION_TYPE_EXECUTE;
        a->target = BASE(registerActivation);
        a->index = 1;
        var_init(&a->parameter);
        // Action data is Int8 zero.
        GX_INT8(a->parameter) = 0;

        a = (gxScriptAction *)malloc(sizeof(gxScriptAction));
        arr_push(&s2->actions, a);
        a->type = DLMS_SCRIPT_ACTION_TYPE_EXECUTE;
        a->target = BASE(registerActivation);
        a->index = 1;
        var_init(&a->parameter);
        // Action data is Int8 zero.
        GX_INT8(a->parameter) = 0;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// Add register activation.
///////////////////////////////////////////////////////////////////////
int addRegisterActivation()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 14, 0, 1, 255};
    if ((ret = INIT_OBJECT(registerActivation, DLMS_OBJECT_TYPE_REGISTER_ACTIVATION, ln)) == 0)
    {
        bb_init(&registerActivation.activeMask);
        bb_addString(&registerActivation.activeMask, "RATE1");
        oa_init(&registerActivation.registerAssignment);
        oa_push(&registerActivation.registerAssignment, BASE(activePowerL1));

        arr_init(&registerActivation.maskList);
        gxByteBuffer *name = malloc(sizeof(gxByteBuffer));
        bb_init(name);
        bb_addString(name, "RATE1");
        gxByteBuffer *indexes = malloc(sizeof(gxByteBuffer));
        bb_init(indexes);
        bb_setUInt8(indexes, 1);
        arr_push(&registerActivation.maskList, key_init(name, indexes));
        name = malloc(sizeof(gxByteBuffer));
        bb_init(name);
        bb_addString(name, "RATE2");
        indexes = malloc(sizeof(gxByteBuffer));
        bb_init(indexes);
        bb_setUInt8(indexes, 1);
        bb_setUInt8(indexes, 2);
        arr_push(&registerActivation.maskList, key_init(name, indexes));
    }
    return ret;
}


///////////////////////////////////////////////////////////////////////
// Add script table object for meter reset. This will erase the EEPROM.
///////////////////////////////////////////////////////////////////////
int addscriptTableGlobalMeterReset()
{
    int ret;
    static gxScript SCRIPTS[1] = {0};
    const unsigned char ln[6] = {0, 0, 10, 0, 0, 255};
    if ((ret = INIT_OBJECT(scriptTableGlobalMeterReset, DLMS_OBJECT_TYPE_SCRIPT_TABLE, ln)) == 0)
    {
        gxScript *s = (gxScript *)malloc(sizeof(gxScript));
        s->id = 1;
        arr_init(&s->actions);
        // Add executed script to script list.
        arr_push(&scriptTableGlobalMeterReset.scripts, s);
    }
    return ret;
}

/////////////////////////////////////////////////////////////////////
// Add script table object for disconnect control.
// Action 1 calls remote_disconnect #1 (close).
// Action 2 calls remote_connect #2(open).
///////////////////////////////////////////////////////////////////////
int addscriptTableDisconnectControl()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 10, 0, 106, 255};
    if ((ret = INIT_OBJECT(scriptTableDisconnectControl, DLMS_OBJECT_TYPE_SCRIPT_TABLE, ln)) == 0)
    {
        gxScript *s = (gxScript *)malloc(sizeof(gxScript));
        s->id = 1;
        arr_init(&s->actions);
        gxScriptAction *a = (gxScriptAction *)malloc(sizeof(gxScriptAction));

        a->type = DLMS_SCRIPT_ACTION_TYPE_EXECUTE;
        a->target = BASE(disconnectControl);
        a->index = 1;
        var_init(&a->parameter);
        // Action data is Int8 zero.
        GX_INT8(a->parameter) = 0;
        arr_push(&s->actions, a);
        // Add executed script to script list.
        arr_push(&scriptTableDisconnectControl.scripts, s);

        s = (gxScript *)malloc(sizeof(gxScript));
        s->id = 2;
        arr_init(&s->actions);
        a = (gxScriptAction *)malloc(sizeof(gxScriptAction));
        a->type = DLMS_SCRIPT_ACTION_TYPE_EXECUTE;
        a->target = BASE(disconnectControl);
        a->index = 2;
        var_init(&a->parameter);
        // Action data is Int8 zero.
        GX_INT8(a->parameter) = 0;
        arr_push(&s->actions, a);
        // Add executed script to script list.
        arr_push(&scriptTableDisconnectControl.scripts, s);
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// Add script table object for test mode. In test mode meter is sending trace to the serial port.
///////////////////////////////////////////////////////////////////////
int addscriptTableActivateTestMode()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 10, 0, 101, 255};
    if ((ret = INIT_OBJECT(scriptTableActivateTestMode, DLMS_OBJECT_TYPE_SCRIPT_TABLE, ln)) == 0)
    {
        gxScript *s = (gxScript *)malloc(sizeof(gxScript));
        s->id = 1;
        arr_init(&s->actions);
        // Add executed script to script list.
        arr_push(&scriptTableActivateTestMode.scripts, s);
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// Add script table object for Normal mode. In normal mode meter is NOT sending trace to the serial port.
///////////////////////////////////////////////////////////////////////
int addscriptTableActivateNormalMode()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 10, 0, 102, 255};
    if ((ret = INIT_OBJECT(scriptTableActivateNormalMode, DLMS_OBJECT_TYPE_SCRIPT_TABLE, ln)) == 0)
    {
        gxScript *s = (gxScript *)malloc(sizeof(gxScript));
        s->id = 1;
        arr_init(&s->actions);
        // Add executed script to script list.
        arr_push(&scriptTableActivateNormalMode.scripts, s);
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// Add Push Script Table.
///////////////////////////////////////////////////////////////////////
int addpushscripttable()
{
    int ret;
    // static gxScript SCRIPTS[1] = {0};
    const unsigned char ln[6] = {0, 0, 10, 0, 108, 255};
    if ((ret = INIT_OBJECT(pushscripttable, DLMS_OBJECT_TYPE_SCRIPT_TABLE, ln)) == 0)
    {
        // arr_init(&pushscripttable.scripts);
        // for(int j = 0 ; j<5; j++)
        // {
        //     arr_init(&arr[j].actions);
        //     arr[j].id = 5;
        //     for( int i = 0 ; i<9 ; i++ )
        //     {
        //         script[i].target = &pushSetup ;
        //         script[i].type = DLMS_SCRIPT_ACTION_TYPE_EXECUTE ; 
        //         script[i].index = 1 ;
        //         script[i].parameter.vt = DLMS_DATA_TYPE_UINT8 ;
        //         script[i].parameter.bVal = 0 ;
        //         arr_push(&arr[j].actions, &script[i]);
        //     }
        //     arr_push(&pushscripttable.scripts, &arr[j]);
        // }
    }
    return ret;
}



///////////////////////////////////////////////////////////////////////
// Add Predefined Scripts -Image activation
///////////////////////////////////////////////////////////////////////
int addPredefinedScriptsImageActivation()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 10, 0, 107, 255};
    if ((ret = INIT_OBJECT(predefinedscriptsimageactivation, DLMS_OBJECT_TYPE_SCRIPT_TABLE, ln)) == 0)
    {

    }
    return ret;
}






///////////////////////////////////////////////////////////////////////
// This method adds example clock object.
///////////////////////////////////////////////////////////////////////
int addClockObject()
{
    int ret = 0;
    // Add default clock. Clock's Logical Name is 0.0.1.0.0.255.
    const unsigned char ln[6] = {0, 0, 1, 0, 0, 255};
    if ((ret = INIT_OBJECT(clock1, DLMS_OBJECT_TYPE_CLOCK, ln)) == 0)
    {
        // Set default values.
        time_init(&clock1.begin, -1, 1, 1, 2, 0, 0, 0, 0);
        clock1.begin.extraInfo = DLMS_DATE_TIME_EXTRA_INFO_NONE;
        time_init(&clock1.end, -1, 6, 30, 2, 0, 0, 0, 0);
        clock1.end.extraInfo = DLMS_DATE_TIME_EXTRA_INFO_NONE;
        // Meter is using UTC time zone.
        clock1.timeZone = -210;
        // Deviation is 60 minutes.
        clock1.deviation = 60;
        clock1.enabled = 1;
        clock1.clockBase = DLMS_CLOCK_BASE_CRYSTAL;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// This method adds example TCP/UDP setup object.
///////////////////////////////////////////////////////////////////////
int addTcpUdpSetup()
{
    // Add Tcp/Udp setup. Default Logical Name is 0.0.25.0.0.255.
    const unsigned char ln[6] = {0, 0, 25, 0, 0, 255};
    INIT_OBJECT(udpSetup, DLMS_OBJECT_TYPE_TCP_UDP_SETUP, ln);
    udpSetup.port = atoi(Settings.ListenPORT);
//    printf("ListenPORT = %d\n",atoi(Settings.ListenPORT));
    udpSetup.ipSetup = &ip4Setup;
    udpSetup.maximumSimultaneousConnections = 5;
    udpSetup.maximumSegmentSize = 1280;
    udpSetup.inactivityTimeout = 600;
    return 0;
}


///////////////////////////////////////////////////////////////////////
// Add profile generic (historical data) object.
///////////////////////////////////////////////////////////////////////
int addLoadProfileProfileGeneric(dlmsSettings *settings)
{
    int ret;
    const unsigned char ln[6] = {1, 0, 99, 1, 0, 255};
    if ((ret = INIT_OBJECT(loadProfile, DLMS_OBJECT_TYPE_PROFILE_GENERIC, ln)) == 0)
    {
        gxTarget *capture;
        // Set default values if load the first time.
        loadProfile.sortMethod = DLMS_SORT_METHOD_FIFO;
        ///////////////////////////////////////////////////////////////////
        // Add 2 columns.
        // Add clock obect.
        capture = (gxTarget *)malloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&loadProfile.captureObjects, key_init(&clock1, capture));
        // Add active power.
        capture = (gxTarget *)malloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&loadProfile.captureObjects, key_init(&activePowerL1, capture));
        ///////////////////////////////////////////////////////////////////
        // Update amount of capture objects.
        // Set clock to sort object.
        loadProfile.sortObject = BASE(clock1);
        loadProfile.sortObjectAttributeIndex = 2;
        loadProfile.profileEntries = getProfileGenericBufferMaxRowCount(settings, &loadProfile);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////
// Add Standard Event Log object.
///////////////////////////////////////////////////////////////////////
int addStandardEventLog(dlmsSettings *settings)
{
    int ret;
    const unsigned char ln[6] = {0, 0, 99, 98, 0, 255};
    if ((ret = INIT_OBJECT(standardeventlog, DLMS_OBJECT_TYPE_PROFILE_GENERIC, ln)) == 0)
    {
        // standardeventlog.capturePeriod = 0;
        gxTarget *capture;
        // Set default values if load the first time.
        // standardeventlog.sortMethod = DLMS_SORT_METHOD_FIFO;
        ///////////////////////////////////////////////////////////////////
        // Add 3 columns.
        // Add clock obect.
        capture = (gxTarget *)malloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&standardeventlog.captureObjects, key_init(&clock1, capture));

        // Add eventCode. 
        capture = (gxTarget *)malloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&standardeventlog.captureObjects, key_init(&eventCode, capture));

        //Add eventparameter
        capture = (gxTarget *)malloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&standardeventlog.captureObjects, key_init(&eventparameter, capture));

        ///////////////////////////////////////////////////////////////////
        // Update amount of capture objects.
        // Set clock to sort object.
        //if this attr comment  the attr is equal 0
        // standardeventlog.sortObject = BASE(clock1);
        // standardeventlog.sortObjectAttributeIndex = 2;
        // standardeventlog.profileEntries = getProfileGenericBufferMaxRowCount(settings, &standardeventlog);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////
// Add Fraud Detection Log object.
///////////////////////////////////////////////////////////////////////
int addFraudDetectionLog(dlmsSettings *settings)
{
    int ret;
    const unsigned char ln[6] = {0, 0, 99, 98, 1, 255};
    if ((ret = INIT_OBJECT(frauddetectionlog, DLMS_OBJECT_TYPE_PROFILE_GENERIC, ln)) == 0)
    {
        gxTarget *capture;
        // Set default values if load the first time.
        // frauddetectionlog.capturePeriod = 0;
        // frauddetectionlog.sortMethod = DLMS_SORT_METHOD_FIFO;
        ///////////////////////////////////////////////////////////////////
        // Add 2 columns.
        // Add clock obect.
        capture = (gxTarget *)malloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&frauddetectionlog.captureObjects, key_init(&clock1, capture));
        // Add event object fraud detection log.
        capture = (gxTarget *)malloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&frauddetectionlog.captureObjects, key_init(&eventobjectfrauddetectionlog, capture));
        ///////////////////////////////////////////////////////////////////
        // Update amount of capture objects.
        // Set clock to sort object.
        // frauddetectionlog.sortObject = BASE(clock1);
        frauddetectionlog.sortObjectAttributeIndex = 2;
        frauddetectionlog.profileEntries = getProfileGenericBufferMaxRowCount(settings, &frauddetectionlog);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////
// Add  Communication Log object.
///////////////////////////////////////////////////////////////////////
int addCommunicationLog(dlmsSettings *settings)
{
    int ret;
    const unsigned char ln[6] = {0, 0, 99, 98, 5, 255};
    if ((ret = INIT_OBJECT(communicationlog, DLMS_OBJECT_TYPE_PROFILE_GENERIC, ln)) == 0)
    {
        gxTarget *capture;
        // Set default values if load the first time.
        // communicationlog.capturePeriod = 0;
        // communicationlog.sortMethod = DLMS_SORT_METHOD_FIFO;
        ///////////////////////////////////////////////////////////////////
        // Add 2 columns.
        // Add clock obect.
        capture = (gxTarget *)malloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&communicationlog.captureObjects, key_init(&clock1, capture));
        // Add event object communication log.
        capture = (gxTarget *)malloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&communicationlog.captureObjects, key_init(&eventobjectcommunicationlog, capture));
        ///////////////////////////////////////////////////////////////////
        // Update amount of capture objects.
        // Set clock to sort object.
        // communicationlog.sortObject = BASE(clock1);
        // communicationlog.sortObjectAttributeIndex = 2;
        // communicationlog.profileEntries = getProfileGenericBufferMaxRowCount(settings, &communicationlog);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////
// Add profile generic (historical data) object.
///////////////////////////////////////////////////////////////////////
int addEventLogProfileGeneric(dlmsSettings *settings)
{
    int ret;
    const unsigned char ln[6] = {1, 0, 99, 98, 0, 255};
    if ((ret = INIT_OBJECT(eventLog, DLMS_OBJECT_TYPE_PROFILE_GENERIC, ln)) == 0)
    {
        eventLog.sortMethod = DLMS_SORT_METHOD_FIFO;
        ///////////////////////////////////////////////////////////////////
        // Add 2 columns as default.
        gxTarget *capture;
        // Add clock obect.
        capture = (gxTarget *)malloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&eventLog.captureObjects, key_init(&clock1, capture));

        // Add event code.
        capture = (gxTarget *)malloc(sizeof(gxTarget));
        capture->attributeIndex = 2;
        capture->dataIndex = 0;
        arr_push(&eventLog.captureObjects, key_init(&eventCode, capture));
        // Set clock to sort object.
        eventLog.sortObject = BASE(clock1);
        eventLog.sortObjectAttributeIndex = 2;
        eventLog.profileEntries = getProfileGenericBufferMaxRowCount(settings, &eventLog);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////
// Add Auto connect object.
///////////////////////////////////////////////////////////////////////
int addAutoConnect()
{
     gxByteBuffer *str;
    // gxtime *start, *end;
    const unsigned char ln[6] = {0, 0, 2, 1, 0, 255};
    INIT_OBJECT(autoConnect, DLMS_OBJECT_TYPE_AUTO_CONNECT, ln);
    autoConnect.mode = DLMS_AUTO_CONNECT_MODE_PERMANENTLY_CONNECT;
    autoConnect.repetitions = 0 ;
    autoConnect.repetitionDelay = 0 ;
    //this comment is ok
    // Calling is allowed between 1am to 6am.
    // start = (gxtime *)malloc(sizeof(gxtime));
    // time_init(start, -1, -1, -1, 1, 0, 0, -1, -1);
    // end = (gxtime *)malloc(sizeof(gxtime));
    // time_init(end, -1, -1, -1, 6, 0, 0, -1, -1);
    // arr_push(&autoConnect.callingWindow, key_init(start, end));


     str = (gxByteBuffer *)malloc(sizeof(gxByteBuffer));
     bb_init(str);
     char str1[100];
     memset(str1,0,sizeof(str1));
     sprintf(str1,"%s:%s",Settings.IP,Settings.PORT);
     printf("IP:PORT = %s\n",str1);
     bb_addString(str, str1);
     arr_push(&autoConnect.destinations, str);

    return 0;
}




///////////////////////////////////////////////////////////////////////
// Add Activity Calendar object.
///////////////////////////////////////////////////////////////////////
int addActivityCalendar()
{
    gxDayProfile *dp;
    gxSeasonProfile *sp;
    gxWeekProfile *wp;
    gxDayProfileAction *act;

    const unsigned char ln[6] = {0, 0, 13, 0, 0, 255};
    INIT_OBJECT(activityCalendar, DLMS_OBJECT_TYPE_ACTIVITY_CALENDAR, ln);

    bb_addString(&activityCalendar.calendarNameActive, "Active");
    // Add season profile.
    sp = (gxSeasonProfile *)malloc(sizeof(gxSeasonProfile));
    bb_init(&sp->name);
    bb_addString(&sp->name, "Summer time");
    time_init(&sp->start, -1, 3, 31, -1, -1, -1, -1, -clock1.timeZone);
    bb_init(&sp->weekName);
    arr_push(&activityCalendar.seasonProfileActive, sp);
    // Add week profile.
    wp = (gxWeekProfile *)malloc(sizeof(gxWeekProfile));
    bb_init(&wp->name);
    bb_addString(&wp->name, "Monday");
    wp->monday = wp->tuesday = wp->wednesday = wp->thursday = wp->friday = wp->saturday = wp->sunday = 1;
    arr_push(&activityCalendar.weekProfileTableActive, wp);

    // Add day profile.
    dp = (gxDayProfile *)malloc(sizeof(gxDayProfile));
    arr_init(&dp->daySchedules);

    dp->dayId = 1;
    act = (gxDayProfileAction *)malloc(sizeof(gxDayProfileAction));
    time_init(&act->startTime, -1, -1, -1, 0, 0, 0, 0, 0x8000);
#ifndef DLMS_IGNORE_OBJECT_POINTERS
    act->script = BASE(tarifficationScriptTable);
#else
    memcpy(act->scriptLogicalName, tarifficationScriptTable.base.logicalName, 6);
#endif // DLMS_IGNORE_OBJECT_POINTERS

    act->scriptSelector = 1;
    arr_push(&dp->daySchedules, act);
    arr_push(&activityCalendar.dayProfileTableActive, dp);
    bb_addString(&activityCalendar.calendarNamePassive, "Passive");

    sp = (gxSeasonProfile *)malloc(sizeof(gxSeasonProfile));
    bb_init(&sp->name);
    bb_addString(&sp->name, "Winter time");
    time_init(&sp->start, -1, 10, 30, -1, -1, -1, -1, 0x8000);
    bb_init(&sp->weekName);
    arr_push(&activityCalendar.seasonProfilePassive, sp);
    // Add week profile.
    wp = (gxWeekProfile *)malloc(sizeof(gxWeekProfile));
    bb_init(&wp->name);
    bb_addString(&wp->name, "Tuesday");
    wp->monday = wp->tuesday = wp->wednesday = wp->thursday = wp->friday = wp->saturday = wp->sunday = 1;
    arr_push(&activityCalendar.weekProfileTablePassive, wp);

    // Add day profile.
    dp = (gxDayProfile *)malloc(sizeof(gxDayProfile));
    arr_init(&dp->daySchedules);
    dp->dayId = 1;
    act = (gxDayProfileAction *)malloc(sizeof(gxDayProfileAction));
    time_init(&act->startTime, -1, -1, -1, 0, 0, 0, 0, 0x8000);
#ifndef DLMS_IGNORE_OBJECT_POINTERS
    act->script = BASE(tarifficationScriptTable);
#else
    memcpy(act->scriptLogicalName, tarifficationScriptTable.base.logicalName, 6);
#endif // DLMS_IGNORE_OBJECT_POINTERS
    act->scriptSelector = 1;
    arr_push(&dp->daySchedules, act);
    arr_push(&activityCalendar.dayProfileTablePassive, dp);
    // Activate passive calendar is not called.
    time_init(&activityCalendar.time, -1, -1, -1, -1, -1, -1, -1, 0x8000);
    return 0;
}

///////////////////////////////////////////////////////////////////////
// Add Optical Port Setup object.
///////////////////////////////////////////////////////////////////////
int addOpticalPortSetup(dlmsServerSettings *settings)
{
    const unsigned char ln[6] = {0, 0, 20, 0, 0, 255};
    INIT_OBJECT(localPortSetup, DLMS_OBJECT_TYPE_IEC_LOCAL_PORT_SETUP, ln);
    localPortSetup.defaultMode = DLMS_OPTICAL_PROTOCOL_MODE_DEFAULT;		//at first it was == 1
    localPortSetup.proposedBaudrate = DLMS_BAUD_RATE_9600;
    localPortSetup.defaultBaudrate = DLMS_BAUD_RATE_300;
    localPortSetup.responseTime = DLMS_LOCAL_PORT_RESPONSE_TIME_200_MS;
    // bb_addString(&localPortSetup.deviceAddress, "Gurux");
    // bb_addString(&localPortSetup.password1, "Gurux");
    // bb_addString(&localPortSetup.password2, "Gurux");
    // bb_addString(&localPortSetup.password5, "Gurux");
    settings->localPortSetup = &localPortSetup;
    return 0;
}



///////////////////////////////////////////////////////////////////////
// Add Register Monitor object.
///////////////////////////////////////////////////////////////////////
int addRegisterMonitor()
{
    gxActionSet *action;
    dlmsVARIANT *tmp;

    const unsigned char ln[6] = {0, 0, 16, 1, 0, 255};
    INIT_OBJECT(registerMonitor, DLMS_OBJECT_TYPE_REGISTER_MONITOR, ln);

    // Add low value.
    tmp = (dlmsVARIANT *)malloc(sizeof(dlmsVARIANT));
    var_init(tmp);
    var_setUInt32(tmp, 10000);
    va_push(&registerMonitor.thresholds, tmp);
    // Add high value.
    tmp = (dlmsVARIANT *)malloc(sizeof(dlmsVARIANT));
    var_init(tmp);
    var_setUInt32(tmp, 30000);
    va_push(&registerMonitor.thresholds, tmp);
    // Add last values so script is not invoke multiple times.
    dlmsVARIANT empty;
    var_init(&empty);
    va_addValue(&registerMonitor.lastValues, &empty, 2);
    registerMonitor.monitoredValue.attributeIndex = 2;
#ifndef DLMS_IGNORE_OBJECT_POINTERS
    registerMonitor.monitoredValue.target = BASE(activePowerL1);
#else
    registerMonitor.monitoredValue.objectType = activePowerL1.base.objectType;
    memcpy(registerMonitor.monitoredValue.logicalName, activePowerL1.base.logicalName, 6);
#endif // DLMS_IGNORE_OBJECT_POINTERS

    //////////////////////
    // Add low action. Turn LED OFF.
    action = (gxActionSet *)malloc(sizeof(gxActionSet));
#ifndef DLMS_IGNORE_OBJECT_POINTERS
    action->actionDown.script = &scriptTableDisconnectControl;
#else
    memcpy(action->actionDown.logicalName, scriptTableDisconnectControl.base.logicalName, 6);
#endif // DLMS_IGNORE_OBJECT_POINTERS

    action->actionDown.scriptSelector = 1;
#ifndef DLMS_IGNORE_OBJECT_POINTERS
    action->actionUp.script = NULL;
#else
    memset(action->actionUp.logicalName, 0, 6);
#endif // DLMS_IGNORE_OBJECT_POINTERS
    action->actionUp.scriptSelector = 2;
    arr_push(&registerMonitor.actions, action);
    //////////////////////
    // Add high action. Turn LED ON.
    action = (gxActionSet *)malloc(sizeof(gxActionSet));
#ifndef DLMS_IGNORE_OBJECT_POINTERS
    action->actionDown.script = NULL;
#else
    memset(action->actionDown.logicalName, 0, 6);
#endif // DLMS_IGNORE_OBJECT_POINTERS
    action->actionDown.scriptSelector = 1;
#ifndef DLMS_IGNORE_OBJECT_POINTERS
    action->actionUp.script = &scriptTableDisconnectControl;
#else
    memset(action->actionUp.logicalName, 0, 6);
#endif // DLMS_IGNORE_OBJECT_POINTERS
    action->actionUp.scriptSelector = 1;
    arr_push(&registerMonitor.actions, action);
    return 0;
}

///////////////////////////////////////////////////////////////////////
// Add action schedule object for disconnect control to close the led.
///////////////////////////////////////////////////////////////////////
int addActionScheduleDisconnectClose()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 15, 0, 1, 255};
    if ((ret = INIT_OBJECT(actionScheduleDisconnectClose, DLMS_OBJECT_TYPE_ACTION_SCHEDULE, ln)) == 0)
    {
        actionScheduleDisconnectClose.executedScript = &scriptTableDisconnectControl;
        actionScheduleDisconnectClose.executedScriptSelector = 1;
        actionScheduleDisconnectClose.type = DLMS_SINGLE_ACTION_SCHEDULE_TYPE1;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// Add action schedule object for disconnect control to open the led.
///////////////////////////////////////////////////////////////////////
int addActionScheduleDisconnectOpen()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 15, 0, 3, 255};
    // Action schedule execution times.
    if ((ret = INIT_OBJECT(actionScheduleDisconnectOpen, DLMS_OBJECT_TYPE_ACTION_SCHEDULE, ln)) == 0)
    {
        actionScheduleDisconnectOpen.executedScript = &scriptTableDisconnectControl;
        actionScheduleDisconnectOpen.executedScriptSelector = 2;
        actionScheduleDisconnectOpen.type = DLMS_SINGLE_ACTION_SCHEDULE_TYPE1;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// Add Image Transfer Activation Scheduler
///////////////////////////////////////////////////////////////////////
int addImageTransferActivationScheduler()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 15, 0, 2, 255};
    // Action schedule execution times.
    if ((ret = INIT_OBJECT(imagetransferactivationSchedule, DLMS_OBJECT_TYPE_ACTION_SCHEDULE, ln)) == 0)
    {
        imagetransferactivationSchedule.executedScript = &predefinedscriptsimageactivation;
        imagetransferactivationSchedule.executedScriptSelector = 1;
        imagetransferactivationSchedule.type = DLMS_SINGLE_ACTION_SCHEDULE_TYPE1;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// Add Disconnect control object.
///////////////////////////////////////////////////////////////////////
int addDisconnectControl()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 96, 3, 10, 255};
    if ((ret = INIT_OBJECT(disconnectControl, DLMS_OBJECT_TYPE_DISCONNECT_CONTROL, ln)) == 0)
    {
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// Add image transfer object.
///////////////////////////////////////////////////////////////////////
int addImageTransfer()
{
    unsigned char ln[6] = {0, 0, 44, 0, 0, 255};
    INIT_OBJECT(imageTransfer, DLMS_OBJECT_TYPE_IMAGE_TRANSFER, ln);
     imageTransfer.imageBlockSize = 900;

    // imageTransfer.imageFirstNotTransferredBlockNumber = 10;
    // Enable image transfer.
     imageTransfer.imageTransferEnabled = 1;
    // imageTransfer.imageTransferStatus = DLMS_IMAGE_TRANSFER_STATUS_VERIFICATION_SUCCESSFUL;

    return 0;
}

///////////////////////////////////////////////////////////////////////
// Add IEC HDLC Setup object.
///////////////////////////////////////////////////////////////////////
int addIecHdlcSetup(dlmsServerSettings *settings)
{
    int ret = 0;
    unsigned char ln[6] = {0, 0, 22, 0, 0, 255};
    if ((ret = INIT_OBJECT(hdlc, DLMS_OBJECT_TYPE_IEC_HDLC_SETUP, ln)) == 0)
    {
        hdlc.communicationSpeed = DLMS_BAUD_RATE_9600;
        hdlc.windowSizeReceive = hdlc.windowSizeTransmit = 1;
        // hdlc.maximumInfoLengthTransmit = hdlc.maximumInfoLengthReceive = 128;
        hdlc.inactivityTimeout = 180;
        // hdlc.deviceAddress = 0x10;
        // hdlc.interCharachterTimeout = 30;
    }
    settings->hdlc = &hdlc;
    return ret;
}

///////////////////////////////////////////////////////////////////////
// Add IEC HDLC Electrical Port object.
///////////////////////////////////////////////////////////////////////
int addHdlcElectricalRS485Port(dlmsServerSettings *settings)
{
    int ret = 0;
    unsigned char ln[6] = {0, 2, 22, 0, 0, 255};
    if ((ret = INIT_OBJECT(hdlcelectricalrs485port, DLMS_OBJECT_TYPE_IEC_HDLC_SETUP, ln)) == 0)
    {
        hdlcelectricalrs485port.communicationSpeed = DLMS_BAUD_RATE_9600;
        hdlcelectricalrs485port.windowSizeReceive = hdlcelectricalrs485port.windowSizeTransmit = 1;
        // hdlcelectricalrs485port.maximumInfoLengthTransmit = hdlcelectricalrs485port.maximumInfoLengthReceive = 128;
        // hdlcelectricalrs485port.inactivityTimeout = 180;
        // hdlcelectricalrs485port.deviceAddress = 0x10;
        hdlcelectricalrs485port.interCharachterTimeout = 30;
    }
    // settings->hdlcelectricalrs485port = &hdlcelectricalrs485port;
    return ret;
}

///////////////////////////////////////////////////////////////////////
// Add compact data object.
///////////////////////////////////////////////////////////////////////
int addCompactData(
    dlmsSettings *settings,
    objectArray *objects)
{
    gxTarget *capture;
    gxKey *k;
    unsigned char ln[6] = {0, 0, 66, 0, 1, 255};
    INIT_OBJECT(compactData, DLMS_OBJECT_TYPE_COMPACT_DATA, ln);
    compactData.templateId = 66;
#ifdef DLMS_ITALIAN_STANDARD
    // Some Italy meters require that there is a array count in some compact buffer.
    // This is against compact data structure defined in DLMS standard.
    compactData.appendAA = 1;
#endif // DLMS_ITALIAN_STANDARD
    // Buffer is captured when invoke is called.
    compactData.captureMethod = DLMS_CAPTURE_METHOD_INVOKE;
    ////////////////////////////////////////
    // Add capture objects.
    // Add compact data template ID as first object.
    capture = (gxTarget *)malloc(sizeof(gxTarget));
    capture->attributeIndex = 4;
    capture->dataIndex = 0;
    k = key_init(&compactData, capture);
    arr_push(&compactData.captureObjects, k);

    capture = (gxTarget *)malloc(sizeof(gxTarget));
    capture->attributeIndex = 4;
    capture->dataIndex = 0;
    k = key_init(&actionScheduleDisconnectOpen, capture);
    arr_push(&compactData.captureObjects, k);
    return compactData_updateTemplateDescription(settings, &compactData);
}

///////////////////////////////////////////////////////////////////////
// Add SAP Assignment object.
///////////////////////////////////////////////////////////////////////
int addSapAssignment()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 41, 0, 0, 255};
    if ((ret = INIT_OBJECT(sapAssignment, DLMS_OBJECT_TYPE_SAP_ASSIGNMENT, ln)) == 0)
    {
        char tmp[17];
        gxSapItem *it = (gxSapItem *)malloc(sizeof(gxSapItem));
        bb_init(&it->name);
        ret = sprintf(tmp, "%s%.13lu", FLAG_ID, SERIAL_NUMBER);
        bb_addString(&it->name, tmp);
        it->id = SERIAL_NUMBER % 10000 + 1000;
//        it->id = 1;
        ret = arr_push(&sapAssignment.sapAssignmentList, it);
    }
    return ret;
}


// Add event code object.
int addEventCode()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 96, 11, 0, 255};
    if ((ret = INIT_OBJECT(eventCode, DLMS_OBJECT_TYPE_DATA, ln)) == 0)
    {
        // GX_UINT16(eventCode.value) = 0;
    }
    return ret;
}

// Add unix time object.
int addUnixTime()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 1, 1, 0, 255};
    if ((ret = INIT_OBJECT(unixTime, DLMS_OBJECT_TYPE_DATA, ln)) == 0)
    {
        // Set initial value.
        GX_UINT32(unixTime.value) = 0;
    }
    return ret;
}

// Add invocation counter object.
int addInvocationCounter()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 43, 1, 0, 255};
    if ((ret = INIT_OBJECT(invocationCounter, DLMS_OBJECT_TYPE_DATA, ln)) == 0)
    {
        // Initial invocation counter value.
        GX_UINT32_BYREF(invocationCounter.value, securitySetupHighGMac.minimumInvocationCounter);
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// Add Auto Answer object.
///////////////////////////////////////////////////////////////////////
int addAutoAnswer()
{
    // gxtime *start, *end;
    const unsigned char ln[6] = {0, 0, 2, 2, 0, 255};
    INIT_OBJECT(autoAnswer, DLMS_OBJECT_TYPE_AUTO_ANSWER, ln);

    // start = (gxtime *)malloc(sizeof(gxtime));
    // time_init(start, -1, -1, -1, 6, -1, -1, -1, -1);
    // end = (gxtime *)malloc(sizeof(gxtime));
    // time_init(end, -1, -1, -1, 8, -1, -1, -1, -1);

    autoAnswer.mode = DLMS_AUTO_CONNECT_MODE_NO_AUTO_CONNECT;
    // arr_push(&autoAnswer.listeningWindow, key_init(start, end));
    // autoAnswer.status = DLMS_AUTO_ANSWER_STATUS_INACTIVE;
    // autoAnswer.numberOfCalls = 2;
    // autoAnswer.numberOfRingsInListeningWindow = 1;
    // autoAnswer.numberOfRingsOutListeningWindow = 2;


    // gxByteBuffer tellnum;
    // bb_init(&tellnum);
    // bb_addString(&tellnum, "09152269328");
    // arr_init(&autoAnswer.listofallowedcallers);
    // arr_push(&autoAnswer.listofallowedcallers,&tellnum);


    return 0;
}

///////////////////////////////////////////////////////////////////////
// Add Modem Configuration object.
///////////////////////////////////////////////////////////////////////
int addModemConfiguration()
{
    //this comment is ok
    // gxModemInitialisation *init;
    const unsigned char ln[6] = {0, 0, 2, 0, 0, 255};
    INIT_OBJECT(modemConfiguration, DLMS_OBJECT_TYPE_MODEM_CONFIGURATION, ln);

    Profile_Modem *profile  ;
    profile = (Profile_Modem *)malloc(sizeof(Profile_Modem));


    // modemConfiguration.communicationSpeed = DLMS_BAUD_RATE_38400;
    // init = (gxModemInitialisation *)malloc(sizeof(gxModemInitialisation));
    // bb_init(&init->request);
    // bb_init(&init->response);
    // bb_addString(&init->request, "AT");
    // bb_addString(&init->response, "OK");
    // init->delay = 0;
    // arr_push(&modemConfiguration.initialisationStrings, init);

    arr_push(&modemConfiguration.modemProfile, profile);


    return 0;
}


unsigned long getIpAddress()
{
    int ret = -1;
    struct hostent *phe;
    char ac[80];
    if ((ret = gethostname(ac, sizeof(ac))) != -1)
    {
        phe = gethostbyname(ac);
        if (phe == 0)
        {
            ret = 0;
        }
        else
        {
            struct in_addr *addr = (struct in_addr *)phe->h_addr_list[0];

            return addr->s_addr;
        }
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// Add IP4 Setup object.
///////////////////////////////////////////////////////////////////////
int addIP4Setup()
{
    int ret;
    dlmsVARIANT *it;
    const unsigned char ln[6] = {0, 0, 25, 1, 0, 255};
    if ((ret = INIT_OBJECT(ip4Setup, DLMS_OBJECT_TYPE_IP4_SETUP, ln)) == 0)
    {
        // ip4Setup.ipAddress = getIpAddress();
        ip4Setup.dataLinkLayer = BASE(pppSetup);

        // ip4Setup.subnetMask = 0xFFFFFFFF;
        // ip4Setup.gatewayIPAddress = 0x0A000000;
        // ip4Setup.primaryDNSAddress = 0x0A0B0C0D;
        // ip4Setup.secondaryDNSAddress = 0x0C0D0E0F;
        // ip4Setup.useDHCP = 1;
        // ip4Setup.subnetMask = 0x0C0D0EFF;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////
// Add PPP Setup object.
///////////////////////////////////////////////////////////////////////
int addPppSetup()
{
    int ret;
    const unsigned char ln[6] = {0, 0, 25, 3, 0, 255};
    if ((ret = INIT_OBJECT(pppSetup, DLMS_OBJECT_TYPE_PPP_SETUP, ln)) == 0)
    {
        pppSetup.phy = BASE(gprsSetup);

        pppSetup.authentication = DLMS_PPP_AUTHENTICATION_TYPE_PAP;

        arr_init(&pppSetup.lcpOptions);
        for(int i = 0 ; i < 9; i++ )
        {
//            lcp[i].type = DLMS_PPP_SETUP_LCP_OPTION_TYPE_AUTH_PROTOCOL;
//            lcp[i].length = 2;
//            lcp[i].data.vt = DLMS_DATA_TYPE_UINT16;
//            lcp[i].data.uiVal = 0xC023;
            arr_push(&pppSetup.lcpOptions, &lcp[i]);

        }
        arr_init(&pppSetup.ipcpOptions);
        for(int i = 0 ; i < 6; i++ )
        {
//            ipcp[i].type = DLMS_PPP_SETUP_IPCP_OPTION_TYPE_IPCOMPRESSIONPROTOCOL;
//            ipcp[i].length = 2;
//            ipcp[i].data.vt = DLMS_DATA_TYPE_UINT16;
//            ipcp[i].data.uiVal = 0x0000;
            arr_push(&pppSetup.ipcpOptions, &ipcp[i]);
        }
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// Add GPRS Setup object.
///////////////////////////////////////////////////////////////////////
int addGprsSetup()
{

	int ret;
	//     static unsigned char APN[15];
	const unsigned char ln[6] = {0, 0, 25, 4, 0, 255};
	if ((ret = INIT_OBJECT(gprsSetup, DLMS_OBJECT_TYPE_GPRS_SETUP, ln)) == 0)
	{
	//	 BB_ATTACH(gprsSetup.apn, APN, 0);
		 ret = bb_addString(&gprsSetup.apn, Settings.APN);
		 printf("APN = %s\n",Settings.APN);
		// gprsSetup.pinCode = 16;
		// gprsSetup.defaultQualityOfService.delay = 1;
		// gprsSetup.defaultQualityOfService.meanThroughput = 10;
		// gprsSetup.defaultQualityOfService.peakThroughput = 100;

			 gprsSetup.requestedQualityOfService.delay = 1;
			 gprsSetup.requestedQualityOfService.meanThroughput = 10;
			 gprsSetup.requestedQualityOfService.peakThroughput = 100;
	}
	return ret;
}




///////////////////////////////////////////////////////////////////////
// Add push setup object. (On Connectivity)
///////////////////////////////////////////////////////////////////////
int addPushSetup()
{
    static unsigned char DESTINATION[20] = {0};

    const char dest[] = "127.0.0.1:7000";
     gxTarget *co;

    const unsigned char ln[6] = {0, 0, 25, 9, 0, 255};
    INIT_OBJECT(pushSetup, DLMS_OBJECT_TYPE_PUSH_SETUP, ln);
    bb_addString(&pushSetup.destination, "127.0.0.1:7000");
    // Add push object itself. This is needed to tell structure of data to
    // the Push listener.
     co = (gxTarget *)malloc(sizeof(gxTarget));
     co->attributeIndex = 1;
     co->dataIndex = 0;
     arr_push(&pushSetup.pushObjectList, key_init(&pushSetup, co));
     // Add logical device name.
     co = (gxTarget *)malloc(sizeof(gxTarget));
     co->attributeIndex = 1;
     co->dataIndex = 0;
     arr_push(&pushSetup.pushObjectList, key_init(&deviceid7, co));
     // Add 0.0.25.1.0.255 Ch. 0 IPv4 setup IP address.
//     co = (gxTarget *)malloc(sizeof(gxTarget));
//     co->attributeIndex = 2;
//     co->dataIndex = 0;
//     arr_push(&pushSetup.pushObjectList, key_init(&deviceid7, co));

     if(strcmp(Settings.MDM , SHAHAB_NEW_VERSION) == 0)
     {
         co = (gxTarget *)malloc(sizeof(gxTarget));
         co->attributeIndex = 1;
         co->dataIndex = 0;
         arr_push(&pushSetup.pushObjectList, key_init(&deviceid6, co));

//         co = (gxTarget *)malloc(sizeof(gxTarget));
//         co->attributeIndex = 2;
//         co->dataIndex = 0;
//         arr_push(&pushSetup.pushObjectList, key_init(&deviceid6, co));
     }

    pushSetup.randomisationStartInterval = 0;
    // pushSetup.numberOfRetries = 5;
    // pushSetup.repetitionDelay = 1;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Load security settings from the EEPROM.
/////////////////////////////////////////////////////////////////////////////
int loadSecurity(dlmsSettings *settings)
{
    const char *fileName = "/usr/bin/ZS-DMG/security.raw";
    int ret = 0;
    // Update keys.
#if _MSC_VER > 1400
    FILE *f = NULL;
    fopen_s(&f, fileName, "rb");
#else
    FILE *f = fopen(fileName, "rb");
#endif
    if (f != NULL)
    {
        // Check that file is not empty.
        fseek(f, 0L, SEEK_END);
        long size = ftell(f);
        if (size != 0)
        {
            fseek(f, 0L, SEEK_SET);
            gxByteBuffer bb;
            bb_init(&bb);
            bb_capacity(&bb, size);
            bb.size += fread(bb.data, 1, size, f);
            fclose(f);
            if ((ret = bb_clear(&settings->cipher.blockCipherKey)) != 0 ||
                (ret = bb_clear(&settings->cipher.authenticationKey)) != 0 ||
                (ret = bb_clear(&settings->kek)) != 0 ||
                (ret = bb_set2(&settings->cipher.blockCipherKey, &bb, 0, 16)) != 0 ||
                (ret = bb_set2(&settings->cipher.authenticationKey, &bb, bb.position, 16)) != 0 ||
                (ret = bb_set2(&settings->kek, &bb, bb.position, 16)) != 0 ||
                // load last server IC.
                (ret = bb_getUInt32(&bb, &settings->cipher.invocationCounter)) != 0 ||
                // load last client IC.
                (ret = bb_getUInt32(&bb, &securitySetupHighGMac.minimumInvocationCounter)) != 0)
            {
            }
            bb_clear(&bb);
            return ret;
        }
    }
    return saveSecurity(settings);
}

/////////////////////////////////////////////////////////////////////////////
// Load data from the EEPROM.
// Returns serialization version or zero if data is not saved.
/////////////////////////////////////////////////////////////////////////////
int loadSettings(dlmsSettings *settings)
{
    const char *fileName = "/usr/bin/ZS-DMG/settings.raw";
    int ret = 0;
    // Update keys.
#if _MSC_VER > 1400
    FILE *f = NULL;
    fopen_s(&f, fileName, "rb");
#else
    FILE *f = fopen(fileName, "rb");
#endif
    if (f != NULL)
    {
        // Check that file is not empty.
        fseek(f, 0L, SEEK_END);
        long size = ftell(f);
        if (size != 0)
        {
            fseek(f, 0L, SEEK_SET);
            gxSerializerSettings serializerSettings;
            ser_init(&serializerSettings);
            serializerSettings.stream = f;
            serializerSettings.ignoredAttributes = NON_SERIALIZED_OBJECTS;
            serializerSettings.count = sizeof(NON_SERIALIZED_OBJECTS) / sizeof(NON_SERIALIZED_OBJECTS[0]);
            ret = ser_loadObjects(settings, &serializerSettings, ALL_OBJECTS, sizeof(ALL_OBJECTS) / sizeof(ALL_OBJECTS[0]));
            return ret;
        }
        fclose(f);
    }
    return saveSettings();
}



int svr_InitObjects(
    dlmsServerSettings *settings)
{
    char buff[17];
    int ret;
    Read_Settings(&Settings);
    SERIAL_NUMBER = atoi(Settings.SerialNumber);
    OA_ATTACH(settings->base.objects, ALL_OBJECTS);
    ///////////////////////////////////////////////////////////////////////
    // Add Logical Device Name. 123456 is meter serial number.
    ///////////////////////////////////////////////////////////////////////
    // COSEM Logical Device Name is defined as an octet-string of 16 octets.
    // The first three octets uniquely identify the manufacturer of the device and it corresponds
    // to the manufacturer's identification in IEC 62056-21.
    // The following 13 octets are assigned by the manufacturer.
    // The manufacturer is responsible for guaranteeing the uniqueness of these octets.

    sprintf(buff, "ZSS03107%.8lu", SERIAL_NUMBER);
    {
        const unsigned char ln[6] = {0, 0, 42, 0, 0, 255};
        INIT_OBJECT(ldn, DLMS_OBJECT_TYPE_DATA, ln);
        var_addBytes(&ldn.value, (unsigned char *)buff, 16);
    }

    // active firmware id 2
    {
        char buf[49];
        char AT_Response[512];
        char *Firmware_Version;

        const unsigned char ln[6] = {1, 2, 0, 2, 0, 255};
        INIT_OBJECT(activefirmwareid2, DLMS_OBJECT_TYPE_DATA, ln);

        exec("serial_atcmd AT+GMR", AT_Response, 30);		//AT command send
        Firmware_Version = strtok(AT_Response, "\n");		//Cutting firmware version
        Firmware_Version = strtok(NULL, "\n");
        if(Firmware_Version != NULL)
        {
        	sprintf(buf, "%s\0", Firmware_Version);
        	var_setString(&activefirmwareid2.value, buf, 49);
        }
    }

    // active firmware signature 2
    {
        const unsigned char ln[6] = {1, 2, 0, 2, 8, 255};
        INIT_OBJECT(activefirmwaresignature2, DLMS_OBJECT_TYPE_DATA, ln);
         char buf[49];
         sprintf(buf, "0\0");
         var_setString(&activefirmwaresignature2.value, buf, 49);
    }


    // Device ID 1
    {
        const unsigned char ln[6] = {0, 0, 96, 1, 0, 255};
        INIT_OBJECT(deviceid1, DLMS_OBJECT_TYPE_DATA, ln);
        char buf[8];
        sprintf(buf,"%s",Settings.SerialNumber);
        var_setString(&deviceid1.value, buf, 8);
    }

    // Device ID 2
    {
        const unsigned char ln[6] = {0, 0, 96, 1, 1, 255};
        INIT_OBJECT(deviceid2, DLMS_OBJECT_TYPE_DATA, ln);
         char buf[49];
         memset(buf,0,sizeof(buf));

         sprintf(buf,"31");
         var_setString(&deviceid2.value, buf, 49);
    }

    // Device ID 3
    {
        const unsigned char ln[6] = {0, 0, 96, 1, 2, 255};
        INIT_OBJECT(deviceid3, DLMS_OBJECT_TYPE_DATA, ln);
         char buf[49];
         sprintf(buf,"Function Location");
         var_setString(&deviceid3.value, buf, 49);
    }

    // Device ID 4
    {
        const unsigned char ln[6] = {0, 0, 96, 1, 3, 255};
        INIT_OBJECT(deviceid4, DLMS_OBJECT_TYPE_DATA, ln);
         char buf[49];
         sprintf(buf,"36.441788,59.420799");
         var_setString(&deviceid4.value, buf, 49);
    }

    // Device ID 5
    {
        const unsigned char ln[6] = {0, 0, 96, 1, 4, 255};
        INIT_OBJECT(deviceid5, DLMS_OBJECT_TYPE_DATA, ln);
         char buf[49];
         sprintf(buf, "RAMZ RAYANEH");
         var_setString(&deviceid5.value, buf, 49);
    }

    // Device ID 6
    {
        const unsigned char ln[6] = {0, 0, 96, 1, 5, 255};
        INIT_OBJECT(deviceid6, DLMS_OBJECT_TYPE_DATA, ln);
         char buf[17];
         sprintf(buf, "0\n");
         var_setString(&deviceid6.value, buf, 17);
    }

    // Device ID 7
   {
       const unsigned char ln[6] = {1, 0, 0, 0, 0, 255};
       INIT_OBJECT(deviceid7, DLMS_OBJECT_TYPE_DATA, ln);
       char buf[15];
       sprintf(buf, "%s31%s%s",Settings.manufactureID,Settings.ProductYear,Settings.SerialNumber);
       var_setString(&deviceid7.value, buf, 14);
   }

   // Error Register
   {
       const unsigned char ln[6] = {0, 0, 97, 97, 0, 255};
       INIT_OBJECT(errorregister, DLMS_OBJECT_TYPE_DATA, ln);
		uint32_t value = 0;
		var_setUInt32(&errorregister.value, value);
   }

   // Unread Log Files Status Register
   {
       const unsigned char ln[6] = {0, 0, 94, 98, 26, 255};
       INIT_OBJECT(unreadlogfilesstatusregister, DLMS_OBJECT_TYPE_DATA, ln);
		uint32_t value = 0;
		var_setUInt32(&unreadlogfilesstatusregister.value, value);
   }

   // Alarm Register 1
   {
       const unsigned char ln[6] = {0, 0, 97, 98, 0, 255};
       INIT_OBJECT(alarmregister1, DLMS_OBJECT_TYPE_DATA, ln);
		uint32_t value = 0;
		var_setUInt32(&alarmregister1.value, value);
   }
   // Alarm Filter 1
   {
       const unsigned char ln[6] = {0, 0, 97, 98, 10, 255};
       INIT_OBJECT(alarmfilter1, DLMS_OBJECT_TYPE_DATA, ln);
		uint32_t value = 0;
		var_setUInt32(&alarmfilter1.value, value);
   }

   // Alarm Register 2
   {
       const unsigned char ln[6] = {0, 0, 97, 98, 1, 255};
       INIT_OBJECT(alarmregister2, DLMS_OBJECT_TYPE_DATA, ln);
		double long value = 0;
		var_setUInt32(&alarmregister2.value, value);
   }

   // Alarm Filter 2
   {
       const unsigned char ln[6] = {0, 0, 97, 98, 11, 255};
       INIT_OBJECT(alarmfilter2, DLMS_OBJECT_TYPE_DATA, ln);
		double long value = 0;
		var_setUInt32(&alarmfilter2.value, value);
   }

   // Event Parameter
   // Event Parameter-Standard Event Log
   {
       const unsigned char ln[6] = {0, 0, 96, 11, 10, 255};
       INIT_OBJECT(eventparameter, DLMS_OBJECT_TYPE_DATA, ln);
		uint8_t value = 0;
		var_setEnum(&eventparameter.value, value);
   }

   // Event Object - Fraud Detection Log
   {
       const unsigned char ln[6] = {0, 0, 96, 11, 1, 255};
       INIT_OBJECT(eventobjectfrauddetectionlog, DLMS_OBJECT_TYPE_DATA, ln);
		uint8_t value = 255;
		var_setEnum(&eventobjectfrauddetectionlog.value, value);
   }

   // Event Object - Communication Log
   {
       const unsigned char ln[6] = {0, 0, 96, 11, 5, 255};
       INIT_OBJECT(eventobjectcommunicationlog, DLMS_OBJECT_TYPE_DATA, ln);
		uint8_t value = 255;
		var_setEnum(&eventobjectcommunicationlog.value, value);
   }

   // GPRS Keep Alive Time Interval
   {
		const unsigned char ln[6] = {0, 0, 94, 98, 19, 255};
		INIT_OBJECT(gprskeepalivetimeinterval, DLMS_OBJECT_TYPE_DATA, ln);

		va_init(&gprs_kat_VarArr);

		var_init(&gprs_kat_dlmsVar[0]);
		var_init(&gprs_kat_dlmsVar[1]);
		var_init(&gprs_kat_dlmsVar[2]);

		var_setBoolean	(&gprs_kat_dlmsVar[0], 1);
		var_setUInt32	(&gprs_kat_dlmsVar[1], 60);
		var_setUInt32	(&gprs_kat_dlmsVar[2], 10);

		va_push(&gprs_kat_VarArr,&gprs_kat_dlmsVar[0]);
		va_push(&gprs_kat_VarArr,&gprs_kat_dlmsVar[1]);
		va_push(&gprs_kat_VarArr,&gprs_kat_dlmsVar[2]);

		dlmsVARIANT** p = (dlmsVARIANT**) gprs_kat_VarArr.data;

		var_attachStructure(&gprskeepalivetimeinterval.value, p, 3);

   }

   // Local Authentication Protection
   {
		const unsigned char ln[6] = {0, 0, 94, 98, 20, 255};
		INIT_OBJECT(localauthenticationprotection, DLMS_OBJECT_TYPE_DATA, ln);

		va_init(&local_auth_VarArr);

		var_init(&local_auth_dlmsVar[0]);
		var_init(&local_auth_dlmsVar[1]);

		var_setUInt8(&local_auth_dlmsVar[0], 5);
		var_setUInt8(&local_auth_dlmsVar[1], 5);

		va_push(&local_auth_VarArr,&local_auth_dlmsVar[0]);
		va_push(&local_auth_VarArr,&local_auth_dlmsVar[1]);

		dlmsVARIANT** p = (dlmsVARIANT**) local_auth_VarArr.data;

		var_attachStructure(&localauthenticationprotection.value, p, 2);
   }

    // IMEI
    {
        const unsigned char ln[6] = {0, 0, 94, 98, 22, 255};
        INIT_OBJECT(imei, DLMS_OBJECT_TYPE_DATA, ln);
        // char buf[17];
        // sprintf(buf, "0\n");
        // var_setString(&imei.value, buf, 17);
    }

    // Security-Receive Frame Counter broadcast Key
    {
        const unsigned char ln[6] = {0, 0, 43, 1, 1, 255};
        INIT_OBJECT(securityreceiveframecounterbroadcastkey, DLMS_OBJECT_TYPE_DATA, ln);
        double long value = 0;
        var_setUInt32(&securityreceiveframecounterbroadcastkey.value, value);
    }

    if (
        (ret = addSecuritySetupManagementClient()) != 0 ||
        (ret = addSapAssignment()) != 0 ||
        (ret = addEventCode()) != 0 ||
        (ret = addUnixTime()) != 0 ||
        (ret = addInvocationCounter()) != 0 ||
        (ret = addClockObject()) != 0 ||
        (ret = addRegisterObject()) != 0 ||
        (ret = addClockTimeShiftLimit()) != 0 ||
        (ret = addAssociationNone()) != 0 ||
        (ret = addAssociationLow()) != 0 ||
        (ret = addAssociationHigh()) != 0 ||
        (ret = addAssociationHighGMac()) != 0 ||
        (ret = addSecuritySetupHigh()) != 0 ||
        (ret = addSecuritySetupHighGMac()) != 0 ||
        (ret = addPushSetup()) != 0 ||
        (ret = addPredefinedScriptsImageActivation()) != 0 ||
        (ret = addscriptTableGlobalMeterReset()) != 0 ||
        (ret = addscriptTableDisconnectControl()) != 0 ||
        (ret = addscriptTableActivateTestMode()) != 0 ||
        (ret = addscriptTableActivateNormalMode()) != 0 ||
        (ret = addpushscripttable()) != 0 ||
        (ret = addTarifficationScriptTable()) != 0 ||
        (ret = addRegisterActivation()) != 0 ||
        (ret = addLoadProfileProfileGeneric(&settings->base)) != 0 ||
        (ret = addStandardEventLog(&settings->base)) != 0 ||
        (ret = addFraudDetectionLog(&settings->base)) != 0 ||
        (ret = addCommunicationLog(&settings->base)) != 0 ||
        (ret = addEventLogProfileGeneric(&settings->base)) != 0 ||
        (ret = addActionScheduleDisconnectOpen()) != 0 ||
        (ret = addActionScheduleDisconnectClose()) != 0 ||
        (ret = addImageTransferActivationScheduler()) != 0 ||
        (ret = addDisconnectControl()) != 0 ||
        (ret = addIecHdlcSetup(settings)) != 0 ||
        (ret = addHdlcElectricalRS485Port(settings)) != 0 ||
        (ret = addTcpUdpSetup()) != 0 ||
        (ret = addAutoConnect()) != 0 ||
        (ret = addActivityCalendar()) != 0 ||
        (ret = addOpticalPortSetup(settings)) != 0 ||
        (ret = addRegisterMonitor()) != 0 ||
        (ret = addAutoAnswer()) != 0 ||
        (ret = addModemConfiguration()) != 0 ||
        (ret = addIP4Setup()) != 0 ||
        (ret = addPppSetup()) != 0 ||
        (ret = addGprsSetup()) != 0 ||
        (ret = addImageTransfer()) != 0 ||
        (ret = addCompactData(&settings->base, &settings->base.objects)) != 0 ||
        (ret = oa_verify(&settings->base.objects)) != 0 )
    {
        GXTRACE_INT(("Failed to start the meter!"), ret);
        executeTime = 0;
        return ret;
    }
    if ((ret = loadSettings(&settings->base)) != 0)
    {
        GXTRACE_INT(("Failed to load settings!"), ret);
        executeTime = 0;
        return ret;
    }
    if ((ret = loadSecurity(&settings->base)) != 0)
    {
        GXTRACE_INT(("Failed to load security settings!"), ret);
        executeTime = 0;
        return ret;
    }
    updateState(&settings->base, GURUX_EVENT_CODES_POWER_UP);
//    GXTRACE(("Meter started."), NULL);
    return 0;
}




/**
 * Start server on serial port.
 */
int IEC_start(connection *con, char *file)
{
    int ret;
    con->settings.pushClientAddress = 64;    
    
    //Set flag id.
    memcpy(con->settings.flagId, FLAG_ID, 3);
    memcpy(con->settings.base.cipher.authenticationKey.data	,Settings.AuthKey, 	16);
    memcpy(con->settings.base.cipher.blockCipherKey.data	,Settings.UniEncKey,16);

    con->settings.hdlc = &hdlc;
    con->settings.localPortSetup = &localPortSetup;

    if ((ret = IEC_Serial_Start(con, file)) != 0)
    {
        return ret;
    }


    ///////////////////////////////////////////////////////////////////////
    // Server must initialize after all objects are added.
    ret = svr_initialize(&con->settings);
    if (ret != DLMS_ERROR_CODE_OK)
    {
        return ret;
    }
//    bb_addString(&con->settings.base.kek, "1111111111111111");


    return DLMS_ERROR_CODE_OK;
}








/**
 * Start server on serial port.
 */
int rs485_start(connection *con, char *file)
{
    int ret;

    con->settings.hdlc = &hdlcelectricalrs485port;
    if ((ret = RS485_Serial_Start(con, file)) != 0)
    {
        return ret;
    }
}




/**
 * Start server.
 */
int TCP_start(connection *con)
{
    int ret;
    con->settings.pushClientAddress = 102;
    if ((ret = Socket_Connection_Start(con)) != 0)
    {
        return ret;
    }

	con->settings.wrapper = &udpSetup;
    memcpy(con->settings.base.cipher.authenticationKey.data	,Settings.AuthKey, 	16);
    memcpy(con->settings.base.cipher.blockCipherKey.data	,Settings.UniEncKey,16);

    ///////////////////////////////////////////////////////////////////////
    // Server must initialize after all objects are added.
    ret = svr_initialize(&con->settings);
    if (ret != DLMS_ERROR_CODE_OK)
    {
        return ret;
    }
//    bb_addString(&con->settings.base.kek, "1111111111111111");
    return DLMS_ERROR_CODE_OK;
}


int svr_findObject(
    dlmsSettings *settings,
    DLMS_OBJECT_TYPE objectType,
    int sn,
    unsigned char *ln,
    gxValueEventArg *e)
{
    GXTRACE_LN(("findObject"), objectType, ln);
    if (objectType == DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME)
    {
        uint16_t pos;
        objectArray objects;
        gxObject *tmp[6];
        oa_attach(&objects, tmp, sizeof(tmp) / sizeof(tmp[0]));
        objects.size = 0;
        if (oa_getObjects(&settings->objects, DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, &objects) == 0)
        {
            gxAssociationLogicalName *a;
            for (pos = 0; pos != objects.size; ++pos)
            {
                if (oa_getByIndex(&objects, pos, (gxObject **)&a) == 0)
                {
                    if (a->clientSAP == settings->clientAddress &&
                        a->authenticationMechanismName.mechanismId == settings->authentication &&
                        (memcmp(ln, DEFAULT_ASSOCIATION, sizeof(DEFAULT_ASSOCIATION)) == 0 || memcmp(a->base.logicalName, ln, 6) == 0))
                    {
                        e->target = (gxObject *)a;
                        break;
                    }
                }
            }
        }
    }
    if (e->target == NULL)
    {
        GXTRACE_LN(("Unknown object"), objectType, ln);
    }
    return 0;
}

/**
  Find restricting object.
*/
int getRestrictingObject(dlmsSettings *settings,
                         gxValueEventArg *e,
                         gxObject **obj,
                         short *index)
{
    int ret;
    dlmsVARIANT *it, *it2;
    if ((ret = va_getByIndex(e->parameters.Arr, 0, &it)) != 0)
    {
        return ret;
    }
    DLMS_OBJECT_TYPE ot;
    unsigned char *ln;
    if ((ret = va_getByIndex(it->Arr, 0, &it2)) != 0)
    {
        return ret;
    }
    ot = it2->iVal;
    if ((ret = va_getByIndex(it->Arr, 1, &it2)) != 0)
    {
        return ret;
    }
    ln = it2->byteArr->data;
    if ((ret = va_getByIndex(it->Arr, 3, &it2)) != 0)
    {
        return ret;
    }
    *index = it2->iVal;
    if ((ret = oa_findByLN(&settings->objects, ot, ln, obj)) != 0)
    {
        return ret;
    }
    return ret;
}

/**
  Find start index and row count using start and end date time.

  settings: DLMS settings.
             Start time.
  type: Profile Generic type.
  e: Parameters
*/
int getProfileGenericDataByRangeFromRingBuffer(
    dlmsSettings *settings,
    const char *fileName,
    gxValueEventArg *e)
{
    // Get all data if there are no sort object.
    uint32_t s = 0;
    uint32_t l = 0xFFFFFFFF;
    dlmsVARIANT tmp;
    int ret = 0;
    dlmsVARIANT *it;
    uint32_t pos;
    uint32_t last = 0;
    gxObject *obj = NULL;
    short index;
    if ((ret = getRestrictingObject(settings, e, &obj, &index)) != 0)
    {
        return ret;
    }
    var_init(&tmp);
    // Check sort object
    if ((ret = va_getByIndex(e->parameters.Arr, 1, &it)) != 0)
    {
        return ret;
    }
    if (it->vt == DLMS_DATA_TYPE_UINT32)
    {
        s = it->ulVal;
    }
    else
    {
        if ((ret = dlms_changeType(it->byteArr, DLMS_DATA_TYPE_DATETIME, &tmp)) != 0)
        {
            var_clear(&tmp);
            return ret;
        }
        // Start time.
        s = time_toUnixTime2(tmp.dateTime);
        var_clear(&tmp);
    }
    if ((ret = va_getByIndex(e->parameters.Arr, 2, &it)) != 0)
    {
        return ret;
    }
    if (it->vt == DLMS_DATA_TYPE_UINT32)
    {
        l = it->ulVal;
    }
    else
    {
        if ((ret = dlms_changeType(it->byteArr, DLMS_DATA_TYPE_DATETIME, &tmp)) != 0)
        {
            var_clear(&tmp);
            return ret;
        }
        l = time_toUnixTime2(tmp.dateTime);
        var_clear(&tmp);
    }

    uint32_t t;
    gxProfileGeneric *pg = (gxProfileGeneric *)e->target;
    if (pg->entriesInUse != 0)
    {
#if _MSC_VER > 1400
        FILE *f = NULL;
        fopen_s(&f, fileName, "rb");
#else
        FILE *f = fopen(fileName, "rb");
#endif
        uint16_t rowSize = 0;
        uint8_t columnSizes[10];
        DLMS_DATA_TYPE dataTypes[10];
        if (f != NULL)
        {
            getProfileGenericBufferColumnSizes(settings, pg, dataTypes, columnSizes, &rowSize);
            // Skip current index and total amount of the entries.
            fseek(f, 4, SEEK_SET);
            for (pos = 0; pos != pg->entriesInUse; ++pos)
            {
                // Load time from EEPROM.
                fread(&t, sizeof(uint32_t), 1, f);
                // seek to begin of next row.
                fseek(f, rowSize - sizeof(uint32_t), SEEK_CUR);
                // If value is inside of start and end time.
                if (t >= s && t <= l)
                {
                    if (last == 0)
                    {
                        // Save end position if we have only one row.
                        e->transactionEndIndex = e->transactionStartIndex = 1 + pos;
                    }
                    else
                    {
                        if (last <= t)
                        {
                            e->transactionEndIndex = pos + 1;
                        }
                        else
                        {
                            // Index is one based, not zero.
                            if (e->transactionEndIndex == 0)
                            {
                                ++e->transactionEndIndex;
                            }
                            e->transactionEndIndex += pg->entriesInUse - 1;
                            e->transactionStartIndex = pos;
                            break;
                        }
                    }
                    last = t;
                }
            }
            fclose(f);
        }
    }
    return ret;
}

int readProfileGeneric(
    dlmsSettings *settings,
    gxProfileGeneric *pg,
    gxValueEventArg *e)
{
    unsigned char first = e->transactionEndIndex == 0;
    int ret = 0;
    gxArray captureObjects;
    arr_init(&captureObjects);
    char fileName[20];
    getProfileGenericFileName(pg, fileName);
    if (ret == DLMS_ERROR_CODE_OK)
    {
        e->byteArray = 1;
        e->handled = 1;
        // If reading first time.
        if (first)
        {
            // Read all.
            if (e->selector == 0)
            {
                e->transactionStartIndex = 1;
                e->transactionEndIndex = pg->entriesInUse;
            }
            else if (e->selector == 1)
            {
                // Read by entry. Find start and end index from the ring buffer.
                if ((ret = getProfileGenericDataByRangeFromRingBuffer(settings, fileName, e)) != 0 ||
                    (ret = cosem_getColumns(&pg->captureObjects, e->selector, &e->parameters, &captureObjects)) != 0)
                {
                    e->transactionStartIndex = e->transactionEndIndex = 0;
                }
            }
            else if (e->selector == 2)
            {
                dlmsVARIANT *it;
                if ((ret = va_getByIndex(e->parameters.Arr, 0, &it)) == 0)
                {
                    e->transactionStartIndex = var_toInteger(it);
                    if ((ret = va_getByIndex(e->parameters.Arr, 1, &it)) == 0)
                    {
                        e->transactionEndIndex = var_toInteger(it);
                    }
                }
                if (ret != 0)
                {
                    e->transactionStartIndex = e->transactionEndIndex = 0;
                }
                else
                {
                    // If start index is too high.
                    if (e->transactionStartIndex > pg->entriesInUse)
                    {
                        e->transactionStartIndex = e->transactionEndIndex = 0;
                    }
                    // If end index is too high.
                    if (e->transactionEndIndex > pg->entriesInUse)
                    {
                        e->transactionEndIndex = pg->entriesInUse;
                    }
                }
            }
        }
        bb_clear(e->value.byteArr);
        arr_clear(&captureObjects);
        if (ret == 0 && first)
        {
            if (e->transactionEndIndex == 0)
            {
                ret = cosem_setArray(e->value.byteArr, 0);
            }
            else
            {
                ret = cosem_setArray(e->value.byteArr, (uint16_t)(e->transactionEndIndex - e->transactionStartIndex + 1));
            }
        }
        if (ret == 0 && e->transactionEndIndex != 0)
        {
            // Loop items.
            uint32_t pos;
            gxtime tm;
            uint16_t pduSize;
            FILE *f = NULL;
#if _MSC_VER > 1400
            if (fopen_s(&f, fileName, "rb") != 0)
            {
                printf("Failed to open %s.\r\n", fileName);
                return -1;
            }
#else
            if ((f = fopen(fileName, "rb")) != 0)
            {
                printf("Failed to open %s.\r\n", fileName);
                return -1;
            }
#endif
            uint16_t dataSize = 0;
            uint8_t columnSizes[10];
            DLMS_DATA_TYPE dataTypes[10];
            if (f != NULL)
            {
                getProfileGenericBufferColumnSizes(settings, pg, dataTypes, columnSizes, &dataSize);
            }
            // Append data.
            if (ret == 0 && dataSize != 0)
            {
                // Skip current index and total amount of the entries (+4 bytes).
                if (fseek(f, 4 + ((e->transactionStartIndex - 1) * dataSize), SEEK_SET) != 0)
                {
                    printf("Failed to seek %s.\r\n", fileName);
                    return -1;
                }
                for (pos = e->transactionStartIndex - 1; pos != e->transactionEndIndex; ++pos)
                {
                    pduSize = (uint16_t)e->value.byteArr->size;
                    if ((ret = cosem_setStructure(e->value.byteArr, pg->captureObjects.size)) != 0)
                    {
                        break;
                    }
                    uint8_t colIndex;
                    gxKey *it;
                    // Loop capture columns and get values.
                    for (colIndex = 0; colIndex != pg->captureObjects.size; ++colIndex)
                    {
                        if ((ret = arr_getByIndex(&pg->captureObjects, colIndex, (void **)&it)) == 0)
                        {
                            // Date time is saved in EPOCH to save space.
                            if ((((gxObject *)it->key)->objectType == DLMS_OBJECT_TYPE_CLOCK || (gxObject *)it->key == BASE(unixTime)) && ((gxTarget *)it->value)->attributeIndex == 2)
                            {
                                uint32_t time;
                                fread(&time, 4, 1, f);
                                time_initUnix(&tm, time);
                                // Convert to meter time if UNIX time is not used.
                                if (((gxObject *)it->key) != BASE(unixTime))
                                {
                                    clock_utcToMeterTime(&clock1, &tm);
                                }
                                if ((ret = cosem_setDateTimeAsOctetString(e->value.byteArr, &tm)) != 0)
                                {
                                    // Error is handled later.
                                }
                            }
                            else
                            {
                                // Append data type.
                                e->value.byteArr->data[e->value.byteArr->size] = dataTypes[colIndex];
                                ++e->value.byteArr->size;
                                // Read data.
                                fread(&e->value.byteArr->data[e->value.byteArr->size], columnSizes[colIndex], 1, f);
                                e->value.byteArr->size += columnSizes[colIndex];
                            }
                        }
                        if (ret != 0)
                        {
                            // Don't set error if PDU is full.
                            if (ret == DLMS_ERROR_CODE_OUTOFMEMORY)
                            {
                                --e->transactionStartIndex;
                                e->value.byteArr->size = pduSize;
                                ret = 0;
                            }
                            else
                            {
                                break;
                            }
                            break;
                        }
                    }
                    ++e->transactionStartIndex;
                }
                fclose(f);
            }
            else
            {
                printf("Failed to open %s.\r\n", fileName);
                return -1;
            }
        }
    }
    return ret;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void svr_preRead(
    dlmsSettings *settings,
    gxValueEventCollection *args)
{
    gxValueEventArg *e;
    int ret, pos;
    DLMS_OBJECT_TYPE type;
    for (pos = 0; pos != args->size; ++pos)
    {
        if ((ret = vec_getByIndex(args, pos, &e)) != 0)
        {
            return;
        }
//        GXTRACE_LN(("svr_preRead123: "), e->target->objectType, e->target->logicalName);
        // Let framework handle Logical Name read.
        if (e->index == 1)
        {
            continue;
        }

        // Get target type.
        type = (DLMS_OBJECT_TYPE)e->target->objectType;
        // Let Framework will handle Association objects and profile generic automatically.
        if (type == DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME ||
            type == DLMS_OBJECT_TYPE_ASSOCIATION_SHORT_NAME)
        {
            continue;
        }
        // Update value by one every time when user reads register.
        if (e->target == BASE(activePowerL1) && e->index == 2)
        {
            readActivePowerValue();
        }
        // Get time if user want to read date and time.
        if (e->target == BASE(clock1) && e->index == 2)
        {
            gxtime dt;
            time_now(&dt, 1);
            if (e->value.byteArr == NULL)
            {
                e->value.byteArr = (gxByteBuffer *)malloc(sizeof(gxByteBuffer));
                bb_init(e->value.byteArr);
            }
            e->error = cosem_setDateTimeAsOctetString(e->value.byteArr, &dt);
            e->value.vt = DLMS_DATA_TYPE_OCTET_STRING;
            e->handled = 1;
        }
        else if (e->target->objectType == DLMS_OBJECT_TYPE_PROFILE_GENERIC && e->index == 2)
        {
            e->error = (DLMS_ERROR_CODE)readProfileGeneric(settings, (gxProfileGeneric *)e->target, e);
        }
        // Update Unix time.
        if (e->target == BASE(unixTime) && e->index == 2)
        {
            gxtime dt;
            time_now(&dt, 0);
            e->value.ulVal = time_toUnixTime2(&dt);
            e->value.vt = DLMS_DATA_TYPE_UINT32;
            e->handled = 1;
        }
    }
}


int printValues(variantArray *values)
{
    int pos;
    dlmsVARIANT *it;
    gxByteBuffer bb;
    bb_init(&bb);
    for (pos = 0; pos != values->size; ++pos)
    {
        if (va_getByIndex(values, pos, &it) != 0 ||
            var_toString(it, &bb) != 0)
        {
            return DLMS_ERROR_CODE_READ_WRITE_DENIED;
        }
        char *tmp = bb_toString(&bb);
        printf("Writing %s\r\n", tmp);
        free(tmp);
        bb_clear(&bb);
    }
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void svr_preWrite(
    dlmsSettings *settings,
    gxValueEventCollection *args)
{
    char str[25];
    gxValueEventArg *e;
    int ret, pos;
    for (pos = 0; pos != args->size; ++pos)
    {
        if ((ret = vec_getByIndex(args, pos, &e)) != 0)
        {
            return;
        }
        if (e->target == BASE(clock1) && e->index == 2)
        {
            updateState(settings, GURUX_EVENT_CODES_TIME_CHANGE);
        }
        // Loop buffer elements in write.
        else if (e->target == &compactData.base && e->index == 2)
        {
            variantArray values;
            va_init(&values);
            if ((compactData_getValues(settings, &compactData.templateDescription, e->value.byteArr, &values)) != 0 ||
                printValues(&values) != 0)
            {
                e->error = DLMS_ERROR_CODE_READ_WRITE_DENIED;
                break;
            }
            va_clear(&values);
            break;
        }
        // If client try to update low level password when high level authentication is established.
        // This is possible in Indian standard.
        else if (e->target == BASE(associationHigh) && e->index == 7)
        {
            ret = cosem_getOctetString(e->value.byteArr, &associationLow.secret);
            saveSettings();
            e->handled = 1;
        }
        // If client try to update low level password when high level authentication is established.
        // This is possible in Indian standard.
        else if (e->target == BASE(associationHigh) && e->index == 7)
        {
            ret = cosem_getOctetString(e->value.byteArr, &associationLow.secret);
            saveSettings();
            e->handled = 1;
        }
        hlp_getLogicalNameToString(e->target->logicalName, str);
        printf("Writing %s\r\n", str);
    }
}

int sendPush(dlmsSettings *settings, gxPushSetup *push);

void handleProfileGenericActions(
    dlmsSettings *settings,
    gxValueEventArg *it)
{
    if (it->index == 1)
    {
        char fileName[30];
        getProfileGenericFileName((gxProfileGeneric *)it->target, fileName);
        // Profile generic clear is called. Clear data.
        ((gxProfileGeneric *)it->target)->entriesInUse = 0;
        FILE *f = NULL;
#if _MSC_VER > 1400
        fopen_s(&f, fileName, "r+b");
#else
        f = fopen(fileName, "r+b");
#endif
        if (f != NULL)
        {
            gxByteBuffer pdu;
            bb_init(&pdu);
            // Current index in ring buffer.
            bb_setUInt16(&pdu, 0);
            bb_setUInt16(&pdu, 0);
            // Update values to the EEPROM.
            fwrite(pdu.data, 1, 4, f);
            fclose(f);
        }
    }
    else if (it->index == 2)
    {
        // Increase power value before each load profile read to increase the value.
        // This is needed for demo purpose only.
        if (it->target == BASE(loadProfile))
        {
            readActivePowerValue();
        }
        captureProfileGeneric(settings, ((gxProfileGeneric *)it->target));
    }
    saveSettings();
}

// Allocate space for image tranfer.
void allocateImageTransfer(const char *fileName, uint32_t size)
{
    uint32_t pos;
    FILE *f = NULL;
#if _MSC_VER > 1400
    fopen_s(&f, fileName, "ab");
#else
    f = fopen(fileName, "ab");
#endif
    if (f != NULL)
    {
        fseek(f, 0, SEEK_END);
        if ((uint32_t)ftell(f) < size)
        {
            for (pos = (uint32_t)ftell(f); pos != size; ++pos)
            {
                if (fputc(0x00, f) != 0)
                {
                    printf("Error Writing to %s\n", fileName);
                    break;
                }
            }
        }
        fclose(f);
    }
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void svr_preAction(				//This function is used to do something when receive an action command before doing the same action
    dlmsSettings *settings,
    gxValueEventCollection *args)
{
    gxValueEventArg *e;
    int ret, pos;
    for (pos = 0; pos != args->size; ++pos)
    {
        if ((ret = vec_getByIndex(args, pos, &e)) != 0)
        {
            return;
        }
        GXTRACE_LN(("svr_preAction: "), e->target->objectType, e->target->logicalName);
        if (e->target->objectType == DLMS_OBJECT_TYPE_PROFILE_GENERIC)
        {
            handleProfileGenericActions(settings, e);
            e->handled = 1;
        }
        else if (e->target == BASE(activePowerL1))
        {
            // Set default value for active power.
            activePowerL1Value = 0;
            e->handled = 1;
        }
        else if (e->target == BASE(pushSetup) && e->index == 1)
        {
            updateState(settings, GURUX_EVENT_CODES_PUSH);
            sendPush(settings, (gxPushSetup *)e->target);
            e->handled = 1;
        }
        // If client wants to clear EEPROM data using Global meter reset script.
        else if (e->target == BASE(scriptTableGlobalMeterReset) && e->index == 1)
        {
            // Initialize data size so default values are used on next connection.
            const char *fileName = "/usr/bin/ZS-DMG/settings.raw";

            FILE *f = fopen(fileName, "wb");
            if (f != NULL)
            {
                fclose(f);
            }
            // Load objects again.
            if ((ret = loadSettings(settings)) != 0)
            {
                GXTRACE_INT(("Failed to load settings!"), ret);
                executeTime = 0;
                break;
            }
            if ((ret = loadSecurity(settings)) != 0)
            {
                GXTRACE_INT(("Failed to load security settings!"), ret);
                executeTime = 0;
                break;
            }
            updateState(settings, GURUX_EVENT_CODES_GLOBAL_METER_RESET);
            e->handled = 1;
        }
        else if (e->target == BASE(disconnectControl))
        {
            updateState(settings, GURUX_EVENT_CODES_OUTPUT_RELAY_STATE);
            // Disconnect. Turn led OFF.
            if (e->index == 1)
            {
                printf("%s\r\n", "Led is OFF.");
            }
            else // Reconnnect. Turn LED ON.
            {
                printf("%s\r\n", "Led is ON.");
            }
        }
        else if (e->target == BASE(scriptTableActivateTestMode))
        {
            // Activate test mode.
            testMode = 1;
            saveSettings();
        }
        else if (e->target == BASE(scriptTableActivateNormalMode))
        {
            // Activate normal mode.
            testMode = 0;
            saveSettings();
        }
        if (e->target == BASE(imageTransfer))
        {

            e->handled = 1;
			#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)

            	FILE* f;
				gxImageTransfer* i = (gxImageTransfer*)e->target;
				const char* imageFile = "/usr/bin/ZS-DMG/image.ipk";

				//Image name and size to transfer
				if (e->index == 1)
				{
					i->imageTransferStatus = DLMS_IMAGE_TRANSFER_STATUS_NOT_INITIATED;
					//There is only one image.

					imageTransfer.imageActivateInfo.size = 1;

					int pos;
					dlmsVARIANT* it;
					gxByteBuffer bb;
					bb_init(&bb);
					for (pos = 0; pos != e->parameters.Arr->size; ++pos)
					{
						if (va_getByIndex(e->parameters.Arr, pos, &it) != 0)
						{
							//return DLMS_ERROR_CODE_READ_WRITE_DENIED;
						}
						else
						{
							var_toString(it, &bb);
							char* tmp = bb_toString(&bb);
							free(tmp);
							bb_clear(&bb);
						}

					}

					uint16_t size;

					if (va_getByIndex(e->parameters.Arr, 0, &it) == 0)
					{
						bb_set2(&IMAGE_ACTIVATE_INFO[0].identification, it->byteArr, 0, it->byteArr->size);
					}
					else
					{
						e->error = DLMS_ERROR_CODE_INCONSISTENT_CLASS_OR_OBJECT;
						return;
					}


					if (va_getByIndex(e->parameters.Arr, 1, &it) == 0)
					{
						IMAGE_ACTIVATE_INFO[0].size= var_toInteger (it);
					}
					else
					{
						e->error = DLMS_ERROR_CODE_INCONSISTENT_CLASS_OR_OBJECT;
						return;
					}

					#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)//If Windows or Linux
						printf("Updating image %s Size: %d\r\n", imageFile, IMAGE_ACTIVATE_INFO[0].size);
					#endif

					allocateImageTransfer(imageFile, IMAGE_ACTIVATE_INFO[0].size);
					ba_clear(&i->imageTransferredBlocksStatus);
					i->imageTransferStatus = DLMS_IMAGE_TRANSFER_STATUS_INITIATED;
				}
				//Transfers one block of the Image to the server
				else if (e->index == 2)
				{
					uint32_t index;
					uint16_t blockSize;

					int pos;
					dlmsVARIANT* it;
					gxByteBuffer bb;
					bb_init(&bb);
					for (pos = 0; pos != e->parameters.Arr->size; ++pos)
					{
						if (va_getByIndex(e->parameters.Arr, pos, &it) != 0)
						{
							//return DLMS_ERROR_CODE_READ_WRITE_DENIED;
						}
						else
						{
							var_toString(it, &bb);
							char* tmp = bb_toString(&bb);

							free(tmp);
							bb_clear(&bb);
						}

					}


					if (va_getByIndex(e->parameters.Arr, 0, &it) == 0)
					{
						index= var_toInteger(it);
					}
					else
					{
						e->error = DLMS_ERROR_CODE_HARDWARE_FAULT;
						return;
					}


					if (va_getByIndex(e->parameters.Arr, 1, &it) == 0)
					{
						blockSize = it->byteArr->size;
					}
					else
					{
						e->error = DLMS_ERROR_CODE_HARDWARE_FAULT;
						return;
					}

					if ((ret = ba_setByIndex(&i->imageTransferredBlocksStatus, (uint16_t)index, 1)) == 0)
					{
						i->imageFirstNotTransferredBlockNumber = index + 1;
					}
					f = fopen(imageFile, "r+b");
					if (!f)
					{
//						#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)//If Windows or Linux
//							printf("Unable to open file %s\r\n", imageFile);
//						#endif

						e->error = DLMS_ERROR_CODE_HARDWARE_FAULT;
						return;
					}
					int ret = (int)fwrite(it->byteArr->data, 1, (int)blockSize, f);
					fclose(f);
					if (ret != (int)blockSize)
					{
						e->error = DLMS_ERROR_CODE_UNMATCH_TYPE;
					}
					var_clear(it);
					imageActionStartTime = time(NULL);

					return;
				}
				//Verifies the integrity of the Image before activation.
				else if (e->index == 3)
				{
					i->imageTransferStatus = DLMS_IMAGE_TRANSFER_STATUS_VERIFICATION_INITIATED;
					f = fopen(imageFile, "rb");
					if (!f)
					{
//						#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)	//If Windows or Linux
//							printf("Unable to open file %s\r\n", imageFile);
//						#endif

						e->error = DLMS_ERROR_CODE_HARDWARE_FAULT;
						return;
					}
					fseek(f, 0L, SEEK_END);
					long size = ftell(f);
					fclose(f);
					if (size != IMAGE_ACTIVATE_INFO[0].size)
					{
						i->imageTransferStatus = DLMS_IMAGE_TRANSFER_STATUS_VERIFICATION_FAILED;
						e->error = DLMS_ERROR_CODE_OTHER_REASON;
					}
					else
					{
						//Wait 5 seconds before image is verified.  This is for example only.
//						if (time(NULL) - imageActionStartTime < 5)
//						{
//
//							e->error = DLMS_ERROR_CODE_TEMPORARY_FAILURE;
//						}
//						else
						{
							i->imageTransferStatus = DLMS_IMAGE_TRANSFER_STATUS_VERIFICATION_SUCCESSFUL;
							imageActionStartTime = time(NULL);
						}
					}
				}
				//Activates the Image.
				else if (e->index == 4)
				{
					i->imageTransferStatus = DLMS_IMAGE_TRANSFER_STATUS_ACTIVATION_INITIATED;
					//Wait 5 seconds before image is activated. This is for example only.
//					if (time(NULL) - imageActionStartTime < 5)
//					{
//						e->error = DLMS_ERROR_CODE_TEMPORARY_FAILURE;
//					}
//					else
					{
						i->imageTransferStatus = DLMS_IMAGE_TRANSFER_STATUS_ACTIVATION_SUCCESSFUL;
						imageActionStartTime = time(NULL);

						char install_command[100];
						memset(install_command, 0, sizeof(install_command));
						sprintf(install_command, "opkg install %s", imageFile);
						system(install_command);
					}
				}
			#endif //defined(_WIN32) || defined(_WIN64) || defined(__linux__)
        }

        if ( e->target == BASE(securitySetupManagementClient) )
        {
        	//e->handled = 1;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void svr_postRead(
    dlmsSettings *settings,
    gxValueEventCollection *args)
{
    gxValueEventArg *e;
    int ret, pos;
    for (pos = 0; pos != args->size; ++pos)
    {
        if ((ret = vec_getByIndex(args, pos, &e)) != 0)
        {
            return;
        }
//        GXTRACE_LN(("svr_postRead321: "), e->target->objectType, e->target->logicalName);
    }
}
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void svr_postWrite(					//This function is used to when an object get value by MDM (TCP or IEC port) - TCP set in object then we get the value from the object
    dlmsSettings *settings,
    gxValueEventCollection *args)
{
    gxValueEventArg *e;
    int ret, pos;
    for (pos = 0; pos != args->size; ++pos)
    {
        if ((ret = vec_getByIndex(args, pos, &e)) != 0)
        {
            return;
        }
        GXTRACE_LN(("svr_postWrite: "), e->target->objectType, e->target->logicalName);
        if (e->target->objectType == DLMS_OBJECT_TYPE_PROFILE_GENERIC)
        {
            // Use want to change capture objects.
            if (e->index == 3)
            {
                saveSettings();
                // Clear buffer if user changes captured objects.
                gxValueEventArg it;
                ve_init(&it);
                it.index = 1;
                it.target = e->target;
                handleProfileGenericActions(settings, &it);
                // Count how many rows fit to the buffer.
                ((gxProfileGeneric *)e->target)->profileEntries = getProfileGenericBufferMaxRowCount(settings, ((gxProfileGeneric *)e->target));
                if (((gxProfileGeneric *)e->target)->captureObjects.size != 0)
                {
                    gxKey *k = NULL;
                    arr_getByIndex(&((gxProfileGeneric *)e->target)->captureObjects, 0, (void **)&k);
                    // Set 1st object to sort object.
                    ((gxProfileGeneric *)e->target)->sortObject = (gxObject *)k->key;
                }
                else
                {
                    ((gxProfileGeneric *)e->target)->sortObject = NULL;
                }
            }
            // Use want to change max amount of profile entries.
            else if (e->index == 8)
            {
                // Count how many rows fit to the buffer.
                uint16_t maxCount = getProfileGenericBufferMaxRowCount(settings, ((gxProfileGeneric *)e->target));
                // If use try to set max profileEntries bigger than can fit to allocated memory.
                if (maxCount < ((gxProfileGeneric *)e->target)->profileEntries)
                {
                    ((gxProfileGeneric *)e->target)->profileEntries = maxCount;
                }
            }
        }
        if(e->target->objectType == DLMS_OBJECT_TYPE_CLOCK)
        {
            // printf("In DLMS_OBJECT_TYPE_CLOCK at post write Function\n");
            struct tm tptr;
            struct timeval tv;
            extern DS1307_I2C_STRUCT_TYPEDEF	DS1307_Str;

            tptr.tm_year = clock1.time.value.tm_year;
            tptr.tm_mon = clock1.time.value.tm_mon;
            tptr.tm_mday = clock1.time.value.tm_mday;
            tptr.tm_hour = clock1.time.value.tm_hour;
            tptr.tm_min = clock1.time.value.tm_min;
            tptr.tm_sec = clock1.time.value.tm_sec;
            tptr.tm_isdst = -1;

            uint16_t 	My;
            uint8_t		Mm, Md;
            SH2M (&My, &Mm, &Md, 1900 + clock1.time.value.tm_year, 1 + clock1.time.value.tm_mon, clock1.time.value.tm_mday);

            DS1307_Str.year		= (uint8_t) (My - 2000)	;		// 123 - 100 = 23
            DS1307_Str.month	= Mm					;		// 0-11 => 1-12
			DS1307_Str.date		= Md					;
			DS1307_Str.hour		= clock1.time.value.tm_hour;
			DS1307_Str.minute	= clock1.time.value.tm_min;
			DS1307_Str.second	= clock1.time.value.tm_sec;
			DS1307_Str.H_12		= 0;
			DS1307_Set_Time( DS1307_Str);
			Set_System_Date_Time(&DS1307_Str)	;

        }
        if(e->target->objectType == DLMS_OBJECT_TYPE_DATA)
        {
        	if(e->target == BASE(gprskeepalivetimeinterval))
        	{
        		GPRS_KAT_GXDATA_STR gprs_kat_values;
        		GPRS_kat_gxData_Get_Value(&gprs_kat_values, &gprskeepalivetimeinterval);

            	Set_Socket_KAT_Option(lnWrapper.socket.Socket_fd, &gprs_kat_values);
        	}
        }
        if (e->error == 0)
        {
            // Save settings to EEPROM.

            saveSettings();
        }
        else
        {
            // Reject changes loading previous settings if there is an error.
            loadSettings(settings);
        }
    }
    // Reset execute time to update execute time if user add new execute times or changes the time.
    executeTime = 0;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void svr_postAction(
    dlmsSettings *settings,
    gxValueEventCollection *args)
{
    gxValueEventArg *e;
    int ret, pos;
    for (pos = 0; pos != args->size; ++pos)
    {
        if ((ret = vec_getByIndex(args, pos, &e)) != 0)
        {
            return;
        }
        GXTRACE_LN(("svr_postAction: "), e->target->objectType, e->target->logicalName);

        if (e->target == BASE(securitySetupHigh) ||
            e->target == BASE(securitySetupHighGMac)
			|| e->target == BASE(securitySetupManagementClient) )
        {
            // Update block cipher key authentication key or broadcast key.
            // Save settings to EEPROM.
        	printf("[INFO] - [exampleserver.c] - [svr_postAction] - [Update block cipher key authentication key or broadcast key - objectType:%d - index:%d - e->error:%d]\n", e->target->objectType, e->index, e->error);

            if (e->error == 0)
            {
                saveSecurity(settings);
            }
            else
            {
                // Load default settings if there is an error.
                loadSecurity(settings);
            }
        }
        // Check is client changing the settings with action.
        else if (svr_isChangedWithAction(e->target->objectType, e->index))
        {
        	printf("[INFO] - [exampleserver.c] - [svr_postAction] - [svr_isChangedWithAction - objectType:%d - index:%d - e->error:%d]\n", e->target->objectType, e->index, e->error);
            // Save settings to EEPROM.
            if (e->error == 0)
            {
                saveSettings();
            }
            else
            {
                // Load default settings if there is an error.
                loadSettings(settings);
            }
        }
    }
}


/**
 * Connect to Push listener.
 */
int connectServer(
    const char *address,
    int port,
    int *s)
{
    int ret;
    struct sockaddr_in add;
    // create socket.
    *s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (*s == -1)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    add.sin_port = htons(port);
    add.sin_family = AF_INET;
    add.sin_addr.s_addr = inet_addr(address);
    // If address is give as name
    if (add.sin_addr.s_addr == INADDR_NONE)
    {
        struct hostent *Hostent = gethostbyname(address);
        if (Hostent == NULL)
        {

            int err = errno;

            close(*s);
            return err;
        };
        add.sin_addr = *(struct in_addr *)(void *)Hostent->h_addr_list[0];
    };

    // Connect to the meter.
    ret = connect(*s, (struct sockaddr *)&add, sizeof(struct sockaddr_in));
    if (ret == -1)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    };
    return DLMS_ERROR_CODE_OK;
}

int sendPush(
    dlmsSettings *settings,
    gxPushSetup *push)
{
    char *p, *host;
    int ret, pos, port, s;
    message messages;
    gxByteBuffer *bb;
    p = strchr((char *)push->destination.data, ':');
    if (p == NULL)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    pos = (int)(p - (char *)push->destination.data);
    host = (char *)malloc(pos + 1);
    memcpy(host, push->destination.data, pos);
    host[pos] = '\0';
    sscanf(++p, "%d", &port);
    mes_init(&messages);
//    if ((ret = connectServer(host, port, &s)) == 0)
    {
    	int sec=settings->cipher.security;
    	settings->cipher.security=0;
    	ret = notify_generatePushSetupMessages(settings, 0, push, &messages);
    	settings->cipher.security=sec;
        if (ret == 0)
        {

            for (pos = 0; pos != messages.size; ++pos)
            {
                bb = messages.data[pos];
//                if ((ret = send(s, (char *)bb->data, bb->size, 0)) == -1)

                /****************/
                memcpy(lnWrapper.buffer.TX,(char *)bb->data, bb->size);
                lnWrapper.buffer.TX_Count = bb->size;
                usleep(500000);
                /****************/

                {
//                    mes_clear(&messages);
//                    break;
                }
            }
        }

//        close(s);
    }
    mes_clear(&messages);
    free(host);
    return 0;
}

unsigned char svr_isTarget(
    dlmsSettings *settings,
    unsigned long serverAddress,
    unsigned long clientAddress)
{
    GXTRACE(("svr_isTarget."), NULL);
    GXTRACE(("svr_isTarget2."), NULL);

    objectArray objects;
    oa_init(&objects);
    unsigned char ret = 0;
    uint16_t pos;
    gxObject *tmp[6];
    oa_attach(&objects, tmp, sizeof(tmp) / sizeof(tmp[0]));
    objects.size = 0;
    if (oa_getObjects(&settings->objects, DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, &objects) == 0)
    {
        gxAssociationLogicalName *a;
        for (pos = 0; pos != objects.size; ++pos)
        {
            if (oa_getByIndex(&objects, pos, (gxObject **)&a) == 0)
            {
                if (a->clientSAP == clientAddress)
                {
                    ret = 1;
                    switch (a->authenticationMechanismName.mechanismId)
                    {
                    case DLMS_AUTHENTICATION_NONE:
                        // Client connects without authentication.
                        GXTRACE(("Connecting without authentication."), NULL);
                        break;
                    case DLMS_AUTHENTICATION_LOW:
                        // Client connects using low authentication.
                        GXTRACE(("Connecting using Low authentication."), NULL);
                        break;
                    default:
                        // Client connects using High authentication.
                        GXTRACE(("Connecting using High authentication."), NULL);
                        break;
                    }

                    printf("clientAddress = %d\n",clientAddress);
                    settings->proposedConformance = a->xDLMSContextInfo.conformance;
                    settings->expectedClientSystemTitle = NULL;
                    // Set Invocation counter value.
                    settings->expectedInvocationCounter = NULL;
                    // Client can establish a ciphered connection only with Security Suite 1.
                    settings->expectedSecuritySuite = 0;
                    // Security policy is not defined by default. Client can connect using any security policy.
                    settings->expectedSecurityPolicy = 0xFF;
                    if (a->securitySetup != NULL)
                    {
                        // Set expected client system title. If this is set only client that is using expected client system title can connect to the meter.
                        if (a->securitySetup->clientSystemTitle.size == 8)
                        {
                        	GXTRACE(("Set the ClientSystem Title."), NULL);
                        	settings->expectedClientSystemTitle = a->securitySetup->clientSystemTitle.data;
                        	printf("-------->>>>>>. systemTitle = %s",a->securitySetup->clientSystemTitle.data);

                        }
                        // GMac authentication uses innocation counter.
                        if (a->securitySetup == &securitySetupHighGMac)
                        {
                            // Set invocation counter value. If this is set client's invocation counter must match with server IC.
                            settings->expectedInvocationCounter = &securitySetupHighGMac.minimumInvocationCounter;
                        }

                        // Set security suite that client must use.
                        settings->expectedSecuritySuite = a->securitySetup->securitySuite;
                        // Set security policy that client must use if it is set.
                        if (a->securitySetup->securityPolicy != 0)
                        {
                            settings->expectedSecurityPolicy = a->securitySetup->securityPolicy;
                        }
                    }
                    break;
                }
            }
        }
    }
    if (ret == 0)
    {
        GXTRACE_INT(("Invalid authentication level."), clientAddress);
        // Authentication is now allowed. Meter is quiet and doesn't return an error.
    }
    else
    {
        // If address is not broadcast or serial number.
        // Remove logical address from the server address.
        unsigned char broadcast = (serverAddress & 0x3FFF) == 0x3FFF || (serverAddress & 0x7F) == 0x7F;
        if (!(broadcast ||
              (serverAddress & 0x3FFF) == SERIAL_NUMBER % 10000 + 1000))
        {
            ret = 0;
            // Find address from the SAP table.
            gxSapAssignment *sap;
            objects.size = 0;
            if (oa_getObjects(&settings->objects, DLMS_OBJECT_TYPE_SAP_ASSIGNMENT, &objects) == 0)
            {
                // If SAP assigment is not used.
                if (objects.size == 0)
                {
                    ret = 1;
                }
                gxSapItem *it;
                uint16_t sapIndex, pos;
                for (sapIndex = 0; sapIndex != objects.size; ++sapIndex)
                {
                    if (oa_getByIndex(&objects, sapIndex, (gxObject **)&sap) == 0)
                    {
                        // If SAP assigment list is empty.
                        if (objects.size == 1 && sap->sapAssignmentList.size == 0)
                        {
                            ret = 1;
                        }
                        
                        for (pos = 0; pos != sap->sapAssignmentList.size; ++pos)
                        {
                            if (arr_getByIndex(&sap->sapAssignmentList, pos, (void **)&it) == 0)
                            {
                                    // Check server address with one byte.
                                if (((serverAddress & 0xFFFFFF00) == 0 && (serverAddress & 0x7F) == it->id) ||
                                    // Check server address with two bytes.
                                    ((serverAddress & 0xFFFF0000) == 0 && (serverAddress & 0x7FFF) == it->id))
                                {
                                    ret = 1;
                                    break;
                                }
                            }
                        }
                    }
                    if (ret != 0)
                    {
                        break;
                    }
                }
            }
            oa_empty(&objects);
        }
        // Set serial number as meter address if broadcast is used.
        if (broadcast)
        {
            settings->serverAddress = SERIAL_NUMBER % 10000 + 1000;
        }
        if (ret == 0)
        {
            GXTRACE_INT(("Invalid server address"), serverAddress);
        }
    }
    return ret;
}

DLMS_SOURCE_DIAGNOSTIC svr_validateAuthentication(
    dlmsServerSettings *settings,
    DLMS_AUTHENTICATION authentication,
    gxByteBuffer *password)
{
    GXTRACE(("svr_validateAuthentication"), NULL);
    if (authentication == DLMS_AUTHENTICATION_NONE)
    {
        // Uncomment this if authentication is always required.
        // return DLMS_SOURCE_DIAGNOSTIC_AUTHENTICATION_MECHANISM_NAME_REQUIRED;
        return DLMS_SOURCE_DIAGNOSTIC_NONE;
    }
    // Check Low Level security..
    if (authentication == DLMS_AUTHENTICATION_LOW)
    {
        if (bb_compare(password, associationLow.secret.data, associationLow.secret.size) == 0)
        {
            GXTRACE(("Invalid low level password."), (const char *)associationLow.secret.data);
            return DLMS_SOURCE_DIAGNOSTIC_AUTHENTICATION_FAILURE;
        }
    }
    // Hith authentication levels are check on phase two.
    return DLMS_SOURCE_DIAGNOSTIC_NONE;
}

// Get attribute access level for profile generic.
DLMS_ACCESS_MODE getProfileGenericAttributeAccess(
    dlmsSettings *settings,
    gxObject *obj,
    unsigned char index)
{
    // // Only read is allowed for event log.
    // if (obj == BASE(eventLog))
    // {
    //     return DLMS_ACCESS_MODE_READ;
    // }
    // // Write is allowed only for High authentication.
    // if (settings->authentication > DLMS_AUTHENTICATION_LOW)
    // {
    //     switch (index)
    //     {
    //     case 3: // captureObjects.
    //         return DLMS_ACCESS_MODE_READ_WRITE;
    //     case 4: // capturePeriod
    //         return DLMS_ACCESS_MODE_READ_WRITE;
    //     case 8: // Profile entries.
    //         return DLMS_ACCESS_MODE_READ_WRITE;
    //     default:
    //         break;
    //     }
    // }
    // return DLMS_ACCESS_MODE_READ;

    if(settings->clientAddress == PUBLIC_CLIENT)    return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if( (obj == BASE(standardeventlog)) || (obj == BASE(frauddetectionlog)) || (obj == BASE(communicationlog)) )
        {
            if(index == 3)      return DLMS_ACCESS_MODE_READ_WRITE;
            return DLMS_ACCESS_MODE_READ;
        }
        else     return DLMS_ACCESS_MODE_NONE;
    }
}

// Get attribute access level for Push Setup.
DLMS_ACCESS_MODE getPushSetupAttributeAccess(
    dlmsSettings *settings,
    unsigned char index)
{
    // // Write is allowed only for High authentication.
    // if (settings->authentication > DLMS_AUTHENTICATION_LOW)
    // {
    //     switch (index)
    //     {
    //     case 2: // pushObjectList
    //     case 4: // communicationWindow
    //         return DLMS_ACCESS_MODE_READ_WRITE;
    //     default:
    //         break;
    //     }
    // }
    // return DLMS_ACCESS_MODE_READ;
    if(settings->clientAddress == PUBLIC_CLIENT)    return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if(index == 1)      return DLMS_ACCESS_MODE_READ;
        return DLMS_ACCESS_MODE_READ_WRITE;
    }
}

// Get attribute access level for Disconnect Control.
DLMS_ACCESS_MODE getDisconnectControlAttributeAccess(
    dlmsSettings *settings,
    unsigned char index)
{
    return DLMS_ACCESS_MODE_READ;
}

// Get attribute access level for register schedule.
DLMS_ACCESS_MODE getActionSchduleAttributeAccess(
    dlmsSettings *settings,
    gxObject *obj,
    unsigned char index)
{
    // // Write is allowed only for High authentication.
    // if (settings->authentication > DLMS_AUTHENTICATION_LOW)
    // {
    //     switch (index)
    //     {
    //     case 4: // Execution time.
    //         return DLMS_ACCESS_MODE_READ_WRITE;
    //     default:
    //         break;
    //     }
    // }
    // return DLMS_ACCESS_MODE_READ;
    if(settings->clientAddress == PUBLIC_CLIENT)    return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if(obj == BASE(imagetransferactivationSchedule)) 
        {
            if(index == 1)      return DLMS_ACCESS_MODE_READ;
            return DLMS_ACCESS_MODE_READ_WRITE;
        }
        else            return DLMS_ACCESS_MODE_NONE;
    }
}

// Get attribute access level for register.
DLMS_ACCESS_MODE getRegisterAttributeAccess(
    dlmsSettings *settings,
    gxObject *obj,
    unsigned char index)
{
    // return DLMS_ACCESS_MODE_READ;
    if(settings->clientAddress == PUBLIC_CLIENT)    return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if(obj == BASE(activePowerL1))        return DLMS_ACCESS_MODE_NONE;
        else
        {
            if(index == 2)                        return DLMS_ACCESS_MODE_READ_WRITE;
            return DLMS_ACCESS_MODE_READ;
        }
    }
}

// Get attribute access level for data objects.
DLMS_ACCESS_MODE getDataAttributeAccess(
    dlmsSettings *settings,
    gxObject *obj,
    unsigned char index)
{
    if(settings->clientAddress == PUBLIC_CLIENT)
    {
        if (obj == BASE(securityreceiveframecounterbroadcastkey))      return DLMS_ACCESS_MODE_READ;
        if (obj == BASE(invocationCounter))                            return DLMS_ACCESS_MODE_READ_WRITE;
        if (obj == BASE(ldn))                                          return DLMS_ACCESS_MODE_READ;
        if (obj == BASE(deviceid7))                                    return DLMS_ACCESS_MODE_READ;
        return DLMS_ACCESS_MODE_NONE;
    }
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        printf("object in Data = %d\n",obj);
        if(index == 2)
        {
            if(obj == BASE(deviceid2))                      return DLMS_ACCESS_MODE_READ_WRITE;
            if(obj == BASE(deviceid3))                      return DLMS_ACCESS_MODE_READ_WRITE;
            if(obj == BASE(deviceid4))                      return DLMS_ACCESS_MODE_READ_WRITE;
            if(obj == BASE(deviceid5))                      return DLMS_ACCESS_MODE_READ_WRITE;
            if(obj == BASE(errorregister))                  return DLMS_ACCESS_MODE_READ_WRITE;
            if(obj == BASE(alarmregister1))                 return DLMS_ACCESS_MODE_READ_WRITE;
            if(obj == BASE(alarmfilter1))                   return DLMS_ACCESS_MODE_READ_WRITE;
            if(obj == BASE(alarmregister2))                 return DLMS_ACCESS_MODE_READ_WRITE;
            if(obj == BASE(alarmfilter2))                   return DLMS_ACCESS_MODE_READ_WRITE;
            if(obj == BASE(gprskeepalivetimeinterval))      return DLMS_ACCESS_MODE_READ_WRITE;
            if(obj == BASE(localauthenticationprotection))  return DLMS_ACCESS_MODE_READ_WRITE;
            if(obj == BASE(invocationCounter))              return DLMS_ACCESS_MODE_READ_WRITE;
        }
        return DLMS_ACCESS_MODE_READ;
    }
    // return DLMS_ACCESS_MODE_READ;
}

// Get attribute access level for script table.
DLMS_ACCESS_MODE getScriptTableAttributeAccess(
    dlmsSettings *settings,
    gxObject *obj,
    unsigned char index)
{
    // return DLMS_ACCESS_MODE_READ;
    if(settings->clientAddress == PUBLIC_CLIENT)                return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)       
    {
        if(obj == BASE(predefinedscriptsimageactivation))       return DLMS_ACCESS_MODE_READ;     
        else if(obj == BASE(pushscripttable))  
        {
            if(index == 2)      return DLMS_ACCESS_MODE_READ_WRITE;
            return DLMS_ACCESS_MODE_READ; 
        }
        else        return DLMS_ACCESS_MODE_NONE;
    }
}

// Get attribute access level for IEC HDLS setup.
DLMS_ACCESS_MODE getHdlcSetupAttributeAccess(
    dlmsSettings *settings,
    unsigned char index)
{
    // // Write is allowed only for High authentication.
    // if (settings->authentication > DLMS_AUTHENTICATION_LOW)
    // {
    //     switch (index)
    //     {
    //     case 2: // Communication speed.
    //     case 7:
    //     case 8:
    //         return DLMS_ACCESS_MODE_READ_WRITE;
    //     default:
    //         break;
    //     }
    // }
    // return DLMS_ACCESS_MODE_READ;
    if(settings->clientAddress == PUBLIC_CLIENT)    return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if(index == 1)      return DLMS_ACCESS_MODE_READ;     
        return DLMS_ACCESS_MODE_READ_WRITE;    
    }
}

// Get attribute access level for association LN.
DLMS_ACCESS_MODE getAssociationAttributeAccess(
    dlmsSettings *settings,
    gxObject *obj,
    unsigned char index)
{
    // If secret
    // if (settings->authentication == DLMS_AUTHENTICATION_LOW && index == 7)
    // {
    //     return DLMS_ACCESS_MODE_READ_WRITE;
    // }
    // return DLMS_ACCESS_MODE_READ;
    // return DLMS_ACCESS_MODE_READ;
    if(settings->clientAddress == PUBLIC_CLIENT)
    {
        if(index == 7)        return DLMS_ACCESS_MODE_NONE;
        return DLMS_ACCESS_MODE_READ;
    }
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if(index == 7)        return DLMS_ACCESS_MODE_WRITE;
        else                    return DLMS_ACCESS_MODE_READ;
    }
}

// Get attribute access level for security setup.
DLMS_ACCESS_MODE getSecuritySetupAttributeAccess(
    dlmsSettings *settings,
    gxObject *obj,
    unsigned char index)
{
    // // Only client system title is writable.
    // if (settings->authentication > DLMS_AUTHENTICATION_LOW && index == 4)
    // {
    //     return DLMS_ACCESS_MODE_READ_WRITE;
    // }
    // return DLMS_ACCESS_MODE_READ;
    if(settings->clientAddress == PUBLIC_CLIENT)    return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if(obj == BASE(securitySetupManagementClient))  
        {
            if(index == 2 || index == 3)        return DLMS_ACCESS_MODE_READ_WRITE;
            return DLMS_ACCESS_MODE_READ;
        }
        else if(obj == BASE(securitySetupHighGMac))
        {
            if(index == 2 || index == 3)        return DLMS_ACCESS_MODE_READ_WRITE;
            return DLMS_ACCESS_MODE_READ;
        }
        else    return DLMS_ACCESS_MODE_NONE;
    }
}

// Get attribute access level for security setup.
DLMS_ACCESS_MODE getActivityCalendarAttributeAccess(
    dlmsSettings *settings,
    unsigned char index)
{
    // // Only Activate passive calendar date-time and passive calendar settings are writeble.
    // if (settings->authentication > DLMS_AUTHENTICATION_LOW && index > 5)
    // {
    //     return DLMS_ACCESS_MODE_READ_WRITE;
    // }
    // return DLMS_ACCESS_MODE_READ;
    return DLMS_ACCESS_MODE_NONE;
}

DLMS_ACCESS_MODE getClockAttributeAccess(
    dlmsSettings *settings,
    unsigned char index)
{
    if(settings->clientAddress == PUBLIC_CLIENT)    return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if(index == 1 || index == 4 || index == 9)      return DLMS_ACCESS_MODE_READ;
        return DLMS_ACCESS_MODE_READ_WRITE;
    }
}

DLMS_ACCESS_MODE getImageTransferAttributeAccess(
    dlmsSettings *settings,
    unsigned char index)
{
    if(settings->clientAddress == PUBLIC_CLIENT)    return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if(index == 5)          return DLMS_ACCESS_MODE_READ_WRITE;
        return DLMS_ACCESS_MODE_READ;
    }
}

DLMS_ACCESS_MODE getIecLocalPortSetupAttributeAccess(
    dlmsSettings *settings,
    unsigned char index)
{
    if(settings->clientAddress == PUBLIC_CLIENT)    return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if(index == 1)                  return DLMS_ACCESS_MODE_READ;
        else if( 2 <= index && index <= 6)       return DLMS_ACCESS_MODE_READ_WRITE;
        else if(index >= 7)             return DLMS_ACCESS_MODE_WRITE;
    }

}

DLMS_ACCESS_MODE getTcpUdpSetupAttributeAccess(
    dlmsSettings *settings,
    unsigned char index)
{
    if(settings->clientAddress == PUBLIC_CLIENT)    return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if(index == 1 || index == 3)           return DLMS_ACCESS_MODE_READ;
        else     return DLMS_ACCESS_MODE_READ_WRITE;
    }

}


DLMS_ACCESS_MODE getIp4SetupAttributeAccess(
    dlmsSettings *settings,
    unsigned char index)
{
    if(settings->clientAddress == PUBLIC_CLIENT)    return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if(index == 1)      return DLMS_ACCESS_MODE_READ;
        return DLMS_ACCESS_MODE_READ_WRITE;
    }

}


DLMS_ACCESS_MODE getPppSetupAttributeAccess(
    dlmsSettings *settings,
    unsigned char index)
{
    if(settings->clientAddress == PUBLIC_CLIENT)    return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if(index == 1)      return DLMS_ACCESS_MODE_READ;
        return DLMS_ACCESS_MODE_READ_WRITE;
    }
}


DLMS_ACCESS_MODE getAutoConnectAttributeAccess(
    dlmsSettings *settings,
    unsigned char index)
{
    if(settings->clientAddress == PUBLIC_CLIENT)    return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if(index == 1)      return DLMS_ACCESS_MODE_READ;
        return DLMS_ACCESS_MODE_READ_WRITE;
    }
}


DLMS_ACCESS_MODE getGprsSetupAttributeAccess(
    dlmsSettings *settings,
    unsigned char index)
{
    if(settings->clientAddress == PUBLIC_CLIENT)    return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if(index == 1)      return DLMS_ACCESS_MODE_READ;
        return DLMS_ACCESS_MODE_READ_WRITE;
    }
}



DLMS_ACCESS_MODE getModemConfigurationAttributeAccess(
    dlmsSettings *settings,
    unsigned char index)
{
    if(settings->clientAddress == PUBLIC_CLIENT)    return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if(index == 1)      return DLMS_ACCESS_MODE_READ;
        return DLMS_ACCESS_MODE_READ_WRITE;
    }
}





DLMS_ACCESS_MODE getAutoAnswerAttributeAccess(
    dlmsSettings *settings,
    unsigned char index)
{
    if(settings->clientAddress == PUBLIC_CLIENT)    return DLMS_ACCESS_MODE_NONE;
    else if(settings->clientAddress == MANAGEMENT_CLIENT)
    {
        if(index == 1)      return DLMS_ACCESS_MODE_READ;
        return DLMS_ACCESS_MODE_READ_WRITE;
    }
}




/**
 * Get attribute access level.
 */
DLMS_ACCESS_MODE svr_getAttributeAccess(
    dlmsSettings *settings,
    gxObject *obj,
    unsigned char index)
{
    GXTRACE("svr_getAttributeAccess", NULL);
    printf("Logical name of Object enter svr_getAttributeAccess : ");
    for(int i=0 ; i<6 ; i++)
    {
        printf("%d.",obj->logicalName[i]);
    }
    printf("\n");
    // Only read is allowed if authentication is not used.
    // if (index == 1 || settings->authentication == DLMS_AUTHENTICATION_NONE)
    // {
    //     GXTRACE("DLMS_ACCESS_MODE_READ", NULL);
    //     return DLMS_ACCESS_MODE_READ;
    // }
    if (obj->objectType == DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME)
    {
        GXTRACE("DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME", NULL);
        
        return getAssociationAttributeAccess(settings, obj, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_PROFILE_GENERIC)
    {
        GXTRACE("DLMS_OBJECT_TYPE_PROFILE_GENERIC", NULL);
        return getProfileGenericAttributeAccess(settings, obj, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_PUSH_SETUP)
    {
        GXTRACE("DLMS_OBJECT_TYPE_PUSH_SETUP", NULL);

        return getPushSetupAttributeAccess(settings, index);
    }
    // if (obj->objectType == DLMS_OBJECT_TYPE_DISCONNECT_CONTROL)
    // {
    //     return getDisconnectControlAttributeAccess(settings, index);
    // }
    // if (obj->objectType == DLMS_OBJECT_TYPE_DISCONNECT_CONTROL)
    // {
    //     return getDisconnectControlAttributeAccess(settings, index);
    // }
    if (obj->objectType == DLMS_OBJECT_TYPE_ACTION_SCHEDULE)
    {
        return getActionSchduleAttributeAccess(settings, obj, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_SCRIPT_TABLE)
    {
        return getScriptTableAttributeAccess(settings, obj, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_REGISTER)
    {
        return getRegisterAttributeAccess(settings, obj, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_DATA)
    {
        return getDataAttributeAccess(settings, obj, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_IEC_HDLC_SETUP)
    {
        return getHdlcSetupAttributeAccess(settings, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_SECURITY_SETUP)
    {
        return getSecuritySetupAttributeAccess(settings, obj, index);
    }
    if (obj->objectType == DLMS_OBJECT_TYPE_ACTIVITY_CALENDAR)
    {
        return getActivityCalendarAttributeAccess(settings, index);
    }
    if(obj->objectType == DLMS_OBJECT_TYPE_CLOCK)
    {
        return getClockAttributeAccess(settings, index);
    }
    if(obj->objectType == DLMS_OBJECT_TYPE_IMAGE_TRANSFER)
    {
        return getImageTransferAttributeAccess(settings, index);
    }
    if(obj->objectType == DLMS_OBJECT_TYPE_IEC_LOCAL_PORT_SETUP)
    {
        return getIecLocalPortSetupAttributeAccess(settings, index);
    }
    if(obj->objectType == DLMS_OBJECT_TYPE_TCP_UDP_SETUP)
    {
        return getTcpUdpSetupAttributeAccess(settings, index);
    }
    if(obj->objectType == DLMS_OBJECT_TYPE_IP4_SETUP)
    {
        return getIp4SetupAttributeAccess(settings, index);
    }
    if(obj->objectType == DLMS_OBJECT_TYPE_PPP_SETUP)
    {
        return getPppSetupAttributeAccess(settings, index);
    }
    if(obj->objectType == DLMS_OBJECT_TYPE_AUTO_CONNECT)
    {
        return getAutoConnectAttributeAccess(settings, index);
    }
    if(obj->objectType == DLMS_OBJECT_TYPE_GPRS_SETUP)
    {
        return getGprsSetupAttributeAccess(settings, index);
    }
    if(obj->objectType == DLMS_OBJECT_TYPE_MODEM_CONFIGURATION)
    {
        return getModemConfigurationAttributeAccess(settings, index);
    }
    if(obj->objectType == DLMS_OBJECT_TYPE_AUTO_ANSWER)
    {
        return getAutoAnswerAttributeAccess(settings, index);
    }
    return DLMS_ACCESS_MODE_NONE;





    // // Only clock write is allowed.
    // if (settings->authentication == DLMS_AUTHENTICATION_LOW)
    // {
    //     if (obj->objectType == DLMS_OBJECT_TYPE_CLOCK)
    //     {
    //         return DLMS_ACCESS_MODE_READ_WRITE;
    //     }
    //     return DLMS_ACCESS_MODE_READ;
    // }
    // // All writes are allowed.
    // return DLMS_ACCESS_MODE_READ_WRITE;
}

/**
 * Get method access level.
 */
DLMS_METHOD_ACCESS_MODE svr_getMethodAccess(
    dlmsSettings *settings,
    gxObject *obj,
    unsigned char index)
{
    // Methods are not allowed.
    if (settings->authentication == DLMS_AUTHENTICATION_NONE)
    {
        return DLMS_METHOD_ACCESS_MODE_NONE;
    }
    // Only clock methods are allowed.
    if (settings->authentication == DLMS_AUTHENTICATION_LOW)
    {
        if (obj->objectType == DLMS_OBJECT_TYPE_CLOCK)
        {
            return DLMS_METHOD_ACCESS_MODE_ACCESS;
        }
        return DLMS_METHOD_ACCESS_MODE_NONE;
    }
    return DLMS_METHOD_ACCESS_MODE_ACCESS;
}





void srv_Save(dlmsServerSettings *settings)
{
	SvrSettings svrsettings;


	svrsettings.protocolVersion					=settings->base.protocolVersion;
//	printf("----in write -->>  settings->base.protocolVersion = %d\n",settings->base.connected);


//	printf("----in write -->>  dedicatedKey->capacity = %lu\n",settings->base.cipher.dedicatedKey->capacity);
//	printf("----in write -->>  dedicatedKey->position = %lu\n",settings->base.cipher.dedicatedKey->position);
//	printf("----in write -->>  dedicatedKey->size = %lu\n",settings->base.cipher.dedicatedKey->size);



//	svrsettings.dedicatedKey_capacity			=dedicatedKey->capacity;
//	svrsettings.dedicatedKey_position			=dedicatedKey->position;
//	svrsettings.dedicatedKey_size				=dedicatedKey->size;
//	memcpy( svrsettings.dedicatedKey_data 		,&dedicatedKey->data , dedicatedKey->capacity);
//

	if(settings->base.cipher.dedicatedKey!=NULL)
	{

		svrsettings.dedicatedKey_capacity					=settings->base.cipher.dedicatedKey->capacity;
		svrsettings.dedicatedKey_size						=settings->base.cipher.dedicatedKey->size;
		svrsettings.dedicatedKey_position					=settings->base.cipher.dedicatedKey->position;
		memcpy(svrsettings.dedicatedKey_data 				,settings->base.cipher.dedicatedKey->data ,settings->base.cipher.dedicatedKey->capacity);

//		printf("----in write -->>  dedicatedKey->capacity = %u\n",settings->base.cipher.dedicatedKey->capacity);
//		printf("----in write -->>  dedicatedKey->position = %u\n",settings->base.cipher.dedicatedKey->position);
//		printf("----in write -->>  dedicatedKey->size     = %u\n",settings->base.cipher.dedicatedKey->size);
//
//		printf("----in write -->>  dedicatedKey->data = \n");
//		for(int n=0;n< settings->base.cipher.dedicatedKey->capacity;n++)
//			printf("%02X",settings->base.cipher.dedicatedKey->data[n]);
//		printf("\n");
	}
	else
	{
		svrsettings.dedicatedKey_capacity					=0xFFFFFFFF;
		svrsettings.dedicatedKey_size						=0xFFFFFFFF;
		svrsettings.dedicatedKey_position					=0xFFFFFFFF;

//		printf("----in write -->>  dedicatedKey->capacity = 0xFFFFFFFF\n");
//		printf("----in write -->>  dedicatedKey->position = 0xFFFFFFFF\n");
//		printf("----in write -->>  dedicatedKey->size     = 0xFFFFFFFF\n");
	}





	memcpy( svrsettings.sourceSystemTitle 		,&settings->base.sourceSystemTitle , sizeof(svrsettings.sourceSystemTitle));
//	printf("----in write -->>  settings->base.sourceSystemTitle = \n");
//	for(int n=0;n< sizeof(svrsettings.sourceSystemTitle);n++)
//		printf("%02X",svrsettings.sourceSystemTitle[n]);
//	printf("\n");







	svrsettings.transaction_command				=settings->transaction.command;
	svrsettings.transaction_capacity			=settings->transaction.data.capacity;
	svrsettings.transaction_position			=settings->transaction.data.position;
	svrsettings.transaction_size				=settings->transaction.data.size;
	memcpy(svrsettings.transaction_data 		,settings->transaction.data.data , settings->transaction.data.capacity);

//	printf("----in write -->>  settings->transaction.command = %d\n",settings->transaction.command);
//	printf("----in write -->>  settings->transaction.data.capacity = %d\n",settings->transaction.data.capacity);
//	printf("----in write -->>  settings->transaction.data.position = %d\n",settings->transaction.data.position);
//	printf("----in write -->>  settings->transaction.data.size = %d\n",settings->transaction.data.size);
//	printf("----in write -->>  settings->transaction.data.data = \n");
//	for(int n=0;n< settings->transaction.data.capacity;n++)
//		printf("%02X",settings->transaction.data.data[n]);
//	printf("\n");







	svrsettings.blockIndex						=settings->base.blockIndex;
	svrsettings.connected						=settings->base.connected;
	svrsettings.authentication					=settings->base.authentication;
	svrsettings.isAuthenticationRequired		=settings->base.isAuthenticationRequired;
	svrsettings.cipher_security					=settings->base.cipher.security;
//	printf("----in write -->>  settings->base.blockIndex = %d\n",settings->base.blockIndex);
//	printf("----in write -->>  settings->base.connected = %d\n",settings->base.connected);
//	printf("----in write -->>  settings->base.authentication = %d\n",settings->base.authentication);
//	printf("----in write -->>  settings->base.isAuthenticationRequired = %d\n",settings->base.isAuthenticationRequired);
//	printf("----in write -->>  settings->base.cipher.security = %d\n",settings->base.cipher.security);






	svrsettings.CtoS_capacity					=settings->base.ctoSChallenge.capacity;
	svrsettings.CtoS_position					=settings->base.ctoSChallenge.position;
	svrsettings.CtoS_size						=settings->base.ctoSChallenge.size;
	memcpy(svrsettings.CtoS_data 				,settings->base.ctoSChallenge.data ,settings->base.ctoSChallenge.capacity);

//	printf("----in write -->>  settings->base.ctoSChallenge.capacity = %d\n",settings->base.ctoSChallenge.capacity);
//	printf("----in write -->>  settings->base.ctoSChallenge.position = %d\n",settings->base.ctoSChallenge.position);
//	printf("----in write -->>  settings->base.ctoSChallenge.size = %d\n",settings->base.ctoSChallenge.size);
//	printf("----in write -->>  settings->base.ctoSChallenge.data = \n");
//	for(int n=0;n< settings->base.ctoSChallenge.capacity;n++)
//		printf("%02X",settings->base.ctoSChallenge.data[n]);
//	printf("\n");




	svrsettings.StoC_capacity					=settings->base.stoCChallenge.capacity;
	svrsettings.StoC_position					=settings->base.stoCChallenge.position;
	svrsettings.StoC_size						=settings->base.stoCChallenge.size;
	memcpy(svrsettings.StoC_data 				,settings->base.stoCChallenge.data , settings->base.stoCChallenge.capacity);

//	printf("----in write -->>  settings->base.stoCChallenge.capacity = %d\n",settings->base.stoCChallenge.capacity);
//	printf("----in write -->>  settings->base.stoCChallenge.position = %d\n",settings->base.stoCChallenge.position);
//	printf("----in write -->>  settings->base.stoCChallenge.size = %d\n",settings->base.stoCChallenge.size);
//	printf("----in write -->>  settings->base.stoCChallenge.data = \n");
//	for(int n=0;n< settings->base.stoCChallenge.capacity;n++)
//		printf("%02X",settings->base.stoCChallenge.data[n]);
//	printf("\n");





	svrsettings.senderFrame						=settings->base.senderFrame;
	svrsettings.receiverFrame					=settings->base.receiverFrame;
	svrsettings.serverAddress					=settings->base.serverAddress;
	svrsettings.clientAddress					=settings->base.clientAddress;
	svrsettings.dataReceived					=settings->dataReceived;
	svrsettings.frameReceived					=settings->frameReceived;
//	printf("----in write -->>  settings->base.senderFrame = %d\n",settings->base.senderFrame);
//	printf("----in write -->>  settings->base.receiverFrame = %d\n",settings->base.receiverFrame);
//	printf("----in write -->>  settings->base.serverAddress = %d\n",settings->base.serverAddress);
//	printf("----in write -->>  settings->base.clientAddress = %d\n",settings->base.clientAddress);
//	printf("----in write -->>  settings->dataReceived = %d\n",settings->dataReceived);
//	printf("----in write -->>  settings->frameReceived = %d\n",settings->frameReceived);



	svrsettings.negotiatedConformance			=settings->base.negotiatedConformance;
//	printf("----in write -->>  settings->base.negotiatedConformance = %d\n",settings->base.negotiatedConformance);


	svrsettings.maxPduSize						=settings->base.maxPduSize;
//	printf("----in write -->>  settings->base.maxPduSize = %d\n",settings->base.maxPduSize);




	FILE* f = fopen("/root/svr.RAW", "wb");
	int ret_fwrite = fwrite((unsigned char*) &svrsettings, sizeof(SvrSettings), 1, f);
//	printf("====================================>>>>ret_fwrite = %d - size of = %d\n", ret_fwrite, sizeof(SvrSettings));
//	printf("----in write -->>  con->settings.base.connected = %d\n",settings->base.connected);
	fclose(f);

}





void srv_Load(dlmsServerSettings *settings)
{
	SvrSettings svrsettings;
	FILE* f = fopen("/root/svr.RAW", "rb");

	if(!f) return;

	int ret_fread = fread((unsigned char*) &svrsettings, sizeof(SvrSettings), 1, f);

	fclose(f);

//	printf("====================================>>>>ret_fread = %d - size of = %d\n", ret_fread, sizeof(SvrSettings));



	if(ret_fread)
	{



		settings->base.protocolVersion						=svrsettings.protocolVersion;

		if(svrsettings.dedicatedKey_capacity!=0xFFFFFFFF)
		{
			settings->base.cipher.dedicatedKey=gxmalloc(sizeof(gxByteBuffer));
			BYTE_BUFFER_INIT(settings->base.cipher.dedicatedKey);

			bb_set(settings->base.cipher.dedicatedKey,svrsettings.dedicatedKey_data,svrsettings.dedicatedKey_capacity);

//			settings->base.stoCChallenge.capacity				=svrsettings.StoC_capacity;
//			settings->base.stoCChallenge.position				=svrsettings.StoC_position;
//			settings->base.stoCChallenge.size					=svrsettings.StoC_size;
		}


		memcpy(	settings->base.sourceSystemTitle 			,svrsettings.sourceSystemTitle			, sizeof(svrsettings.sourceSystemTitle));


//		settings->transaction.command						=svrsettings.transaction_command;
//		settings->transaction.data.capacity					=svrsettings.transaction_capacity;
//		settings->transaction.data.position					=svrsettings.transaction_position;
//		settings->transaction.data.size						=svrsettings.transaction_size;
		//memcpy(settings->transaction.data.data 		 		,svrsettings.transaction_data 			,sizeof(settings->transaction.data.data ));


		settings->base.blockIndex							=svrsettings.blockIndex;
		settings->base.connected							=svrsettings.connected;
		settings->base.authentication						=svrsettings.authentication;
		settings->base.isAuthenticationRequired				=svrsettings.isAuthenticationRequired;
		settings->base.cipher.security						=svrsettings.cipher_security;

		bb_set(&settings->base.ctoSChallenge				,svrsettings.CtoS_data,svrsettings.CtoS_capacity);
		settings->base.ctoSChallenge.capacity				=svrsettings.CtoS_capacity;
		settings->base.ctoSChallenge.position				=svrsettings.CtoS_position;
		settings->base.ctoSChallenge.size					=svrsettings.CtoS_size;


		bb_set(&settings->base.stoCChallenge				,svrsettings.StoC_data,svrsettings.StoC_capacity);
		settings->base.stoCChallenge.capacity				=svrsettings.StoC_capacity;
		settings->base.stoCChallenge.position				=svrsettings.StoC_position;
		settings->base.stoCChallenge.size					=svrsettings.StoC_size;


		settings->base.senderFrame							=svrsettings.senderFrame;
		settings->base.receiverFrame						=svrsettings.receiverFrame;


		settings->base.serverAddress						=svrsettings.serverAddress;
		settings->base.clientAddress						=svrsettings.clientAddress;
//		settings->dataReceived								=svrsettings.dataReceived;
		settings->frameReceived								=svrsettings.frameReceived;
		settings->base.negotiatedConformance				=svrsettings.negotiatedConformance;
		settings->base.maxPduSize							=svrsettings.maxPduSize;



//		printf("----in read -->>  settings->base.protocolVersion = %d\n",settings->base.connected);



//		if(settings->base.cipher.dedicatedKey!=NULL)
//		{
//
//			printf("----in read -->>  dedicatedKey->capacity = %u\n",settings->base.cipher.dedicatedKey->capacity);
//			printf("----in read -->>  dedicatedKey->position = %u\n",settings->base.cipher.dedicatedKey->position);
//			printf("----in read -->>  dedicatedKey->size     = %u\n",settings->base.cipher.dedicatedKey->size);
//
//			printf("----in read -->>  dedicatedKey->data = \n");
//			for(int n=0;n< settings->base.cipher.dedicatedKey->capacity;n++)
//				printf("%02X",settings->base.cipher.dedicatedKey->data[n]);
//			printf("\n");
//		}



//		printf("----in read -->>  settings->base.sourceSystemTitle = \n");
//		for(int n=0;n< sizeof(svrsettings.sourceSystemTitle);n++)
//			printf("%02X",svrsettings.sourceSystemTitle[n]);
//		printf("\n");

//		printf("----in read -->>  settings->transaction.command = %d\n",settings->transaction.command);
//		printf("----in read -->>  settings->transaction.data.capacity = %d\n",settings->transaction.data.capacity);
//		printf("----in read -->>  settings->transaction.data.position = %d\n",settings->transaction.data.position);
//		printf("----in read -->>  settings->transaction.data.size = %d\n",settings->transaction.data.size);
//		printf("----in read -->>  settings->transaction.data.data = \n");
//		for(int n=0;n< settings->transaction.data.capacity;n++)
//			printf("%02X",settings->transaction.data.data[n]);
//		printf("\n");



//		printf("----in read -->>  settings->base.blockIndex = %d\n",settings->base.blockIndex);
//		printf("----in read -->>  settings->base.connected = %d\n",settings->base.connected);
//		printf("----in read -->>  settings->base.authentication = %d\n",settings->base.authentication);
//		printf("----in read -->>  settings->base.isAuthenticationRequired = %d\n",settings->base.isAuthenticationRequired);
//		printf("----in read -->>  settings->base.cipher.security = %d\n",settings->base.cipher.security);
//
//
//		printf("----in read -->>  settings->base.ctoSChallenge.capacity = %d\n",settings->base.ctoSChallenge.capacity);
//		printf("----in read -->>  settings->base.ctoSChallenge.position = %d\n",settings->base.ctoSChallenge.position);
//		printf("----in read -->>  settings->base.ctoSChallenge.size = %d\n",settings->base.ctoSChallenge.size);
//		printf("----in read -->>  settings->base.ctoSChallenge.data = \n");
//		for(int n=0;n< settings->base.ctoSChallenge.capacity;n++)
//			printf("%02X",settings->base.ctoSChallenge.data[n]);
//		printf("\n");
//
//
//		printf("----in read -->>  settings->base.stoCChallenge.capacity = %d\n",settings->base.stoCChallenge.capacity);
//		printf("----in read -->>  settings->base.stoCChallenge.position = %d\n",settings->base.stoCChallenge.position);
//		printf("----in read -->>  settings->base.stoCChallenge.size = %d\n",settings->base.stoCChallenge.size);
//		printf("----in read -->>  settings->base.stoCChallenge.data = \n");
//		for(int n=0;n< settings->base.stoCChallenge.capacity;n++)
//			printf("%02X",settings->base.stoCChallenge.data[n]);
//		printf("\n");
//
//
//		printf("----in read -->>  settings->base.senderFrame = %d\n",settings->base.senderFrame);
//		printf("----in read -->>  settings->base.receiverFrame = %d\n",settings->base.receiverFrame);
//		printf("----in read -->>  settings->base.serverAddress = %d\n",settings->base.serverAddress);
//		printf("----in read -->>  settings->base.clientAddress = %d\n",settings->base.clientAddress);
//		printf("----in read -->>  settings->dataReceived = %d\n",settings->dataReceived);
//		printf("----in read -->>  settings->frameReceived = %d\n",settings->frameReceived);
//
//
//		printf("----in write -->>  settings->base.negotiatedConformance = %d\n",settings->base.negotiatedConformance);

//		printf("----in write -->>  settings->base.maxPduSize = %d\n",settings->base.maxPduSize);


	}



}













/////////////////////////////////////////////////////////////////////////////
// Client has made connection to the server.
/////////////////////////////////////////////////////////////////////////////
int svr_connected(
    dlmsServerSettings *settings)
{
    printf("Connected %d.\r\n", settings->base.connected);
#ifdef DLMS_ITALIAN_STANDARD
    if (settings->base.clientAddress == 1)
    {
        if (settings->base.connected != DLMS_CONNECTION_STATE_DLMS)
        {
            if (settings->base.preEstablishedSystemTitle != NULL)
            {
                bb_clear(settings->base.preEstablishedSystemTitle);
            }
            else
            {
                settings->base.preEstablishedSystemTitle = (gxByteBuffer *)malloc(sizeof(gxByteBuffer));
                bb_init(settings->base.preEstablishedSystemTitle);
            }
            bb_addString(settings->base.preEstablishedSystemTitle, "ABCDEFGH");
            settings->base.cipher.security = DLMS_SECURITY_AUTHENTICATION_ENCRYPTION;
        }
        else
        {
            // Return error if client can connect only using pre-established connnection.
            return DLMS_ERROR_CODE_READ_WRITE_DENIED;
        }
    }
#else
#endif // DLMS_ITALIAN_STANDARD
    return 0;
}

/**
 * Client has try to made invalid connection. Password is incorrect.
 *
 * @param connectionInfo
 *            Connection information.
 */
int svr_invalidConnection(dlmsServerSettings *settings)
{
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
int svr_disconnected(
    dlmsServerSettings *settings)
{
    printf("Disconnected  %d.\r\n", settings->base.connected);
    if (settings->base.cipher.security != 0 && (settings->base.connected & DLMS_CONNECTION_STATE_DLMS) != 0)
    {
        // Save Invocation counter value when connection is closed.
        saveSecurity(&settings->base);
    }
    return 0;
}

void svr_preGet(
    dlmsSettings *settings,
    gxValueEventCollection *args)
{
    gxValueEventArg *e;
    int ret, pos;
    for (pos = 0; pos != args->size; ++pos)
    {
        if ((ret = vec_getByIndex(args, pos, &e)) != 0)
        {
            return;
        }
    }
}

void svr_postGet(
    dlmsSettings *settings,
    gxValueEventCollection *args)
{
}

/**
 * This is reserved for future use. Do not use it.
 *
 * @param args
 *            Handled data type requests.
 */
void svr_getDataType(
    dlmsSettings *settings,
    gxValueEventCollection *args)
{
}






void Read_Settings(SETTINGS *settings)
{
	char buffer[1000],data[20][100], temp[100];
	memset(buffer,0,sizeof(buffer));
	for(int i=0;i<20;i++)
	{
		memset(data[i],0,sizeof(data[i]));
	}
    int fd = open(SETTINGS_PATH, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
    }
    // Read data from the file
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
    close(fd);
    int ptr=0,cnt=0;
    char tmp[50];
    for(int i=0 ; i<bytes_read ; i++)
    {
    	if(buffer[i]=='\n')
    	{
    		memcpy(data[cnt],buffer+ptr,i-ptr);
    		ptr=i+1;
    		cnt++;
    		memset(tmp,0,sizeof(tmp));
    		switch(cnt)
    		{
				case 1 :
				{
					memset(settings->SerialNumber,0,sizeof(settings->SerialNumber));
					strcpy(settings->SerialNumber,data[0]+13);
//					printf("SerialNumber = %s\n",settings->SerialNumber);
					break;
				}
				case 2 :
				{
					memset(settings->ProductYear,0,sizeof(settings->ProductYear));
					strcpy(settings->ProductYear,data[1]+12);
//					printf("ProductYear = %s\n",settings->ProductYear);
					break;
				}
				case 3 :
				{
					memset(settings->manufactureID,0,sizeof(settings->manufactureID));
					strcpy(settings->manufactureID,data[2]+14);
//					printf("manufactureID = %s\n",settings->manufactureID);
					break;
				}
				case 4 :
				{
					memset(settings->IP,0,sizeof(settings->IP));
					strcpy(settings->IP,data[3]+3);
//					printf("IP = %s\n",settings->IP);
					break;
				}
				case 5 :
				{
					memset(settings->PORT,0,sizeof(settings->PORT));
					strcpy(settings->PORT,data[4]+5);
//					printf("PORT = %s\n",settings->PORT);
					break;
				}
				case 6 :
				{
					memset(settings->ListenPORT,0,sizeof(settings->ListenPORT));
					strcpy(settings->ListenPORT,data[5]+11);
//					printf("ListenPORT = %s\n",settings->ListenPORT);
					break;
				}
				case 7 :
				{
					memset(settings->APN,0,sizeof(settings->APN));
					strcpy(settings->APN,data[6]+4);
//					printf("APN = %s\n",settings->APN);
					break;
				}
				case 8 :
				{
					int i;
					memset(temp, 0, sizeof(temp));
					memset(settings->AuthKey,0,sizeof(settings->AuthKey));
					strcpy(temp, data[7]+8);

					for (int i=0; i < 16; i++)
					{
				        sscanf((temp) + (2*i), "%2hhx", &settings->AuthKey[i]);
				    }

//				    printf("AuthKey - Original string: %s\n", temp);
//				    printf("AuthKey - Hexadecimal value: ");
//				    for (i = 0; i < 16; i++)
//				    {
//				        printf("%02X", settings->AuthKey[i]);
//				    }
//				    printf("\n");

					break;
				}
				case 9 :
				{
					int i;
					memset(temp, 0, sizeof(temp));
					memset(settings->BroadEncKey,0,sizeof(settings->BroadEncKey));
					strcpy(temp, data[8]+12);

					for (int i=0; i < 16; i++)
					{
				        sscanf((temp) + (2*i), "%2hhx", &settings->BroadEncKey[i]);
				    }

//				    printf("BroadEncKey - Original string: %s\n", temp);
//				    printf("BroadEncKey - Hexadecimal value: ");
//				    for (i = 0; i < 16; i++)
//				    {
//				        printf("%02X", settings->BroadEncKey[i]);
//				    }
//				    printf("\n");

					break;
				}
				case 10 :
				{
					int i;
					memset(temp, 0, sizeof(temp));
					memset(settings->UniEncKey,0,sizeof(settings->UniEncKey));
					strcpy(temp, data[9]+10);

					for (int i=0; i < 16; i++)
					{
				        sscanf((temp) + (2*i), "%2hhx", &settings->UniEncKey[i]);
				    }

//				    printf("UniEncKey - Original string: %s\n", temp);
//				    printf("UniEncKey - Hexadecimal value: ");
//				    for (i = 0; i < 16; i++)
//				    {
//				        printf("%02X", settings->UniEncKey[i]);
//				    }
//				    printf("\n");

					break;
				}
				case 11 :
				{
//					int i;
//					memset(temp, 0, sizeof(temp));
//					memset(settings->KEK,0,sizeof(settings->KEK));
//					strcpy(temp, data[10]+4);
//
//					for (int i=0; i < 16; i++)
//					{
//				        sscanf((temp) + (2*i), "%2hhx", &settings->KEK[i]);
//				    }

//				    printf("KEK - Original string: %s\n", temp);
//				    printf("KEK - Hexadecimal value: ");
//				    for (i = 0; i < 16; i++)
//				    {
//				        printf("%02X", settings->KEK[i]);
//				    }
//				    printf("\n");

					break;
				}
				case 12:
				{
					memset(settings->LLSPass,0,sizeof(settings->LLSPass));
					strcpy(settings->LLSPass,data[11]+8);
//					printf("LLSPass = %s\n",settings->LLSPass);
					break;
				}
				case 13:
				{
					memset(settings->MDM,0,sizeof(settings->MDM));
					strcpy(settings->MDM,data[12]+4);
//					settings->MDM = data[12][5];
					break;
				}
				default :
				{
//					printf("");
					break;
				}

    		}
    	}
    }
}

