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

#ifndef CONNECTION_H
#define CONNECTION_H

#include "../../development/include/bytebuffer.h"
#include "../../development/include/dlmssettings.h"
#include <stdio.h>


#include <pthread.h>

static const unsigned int RECEIVE_BUFFER_SIZE = 200;

#ifdef  __cplusplus
extern "C" {
#endif

static uint32_t Boudrate[]={ 300 , 600 , 1200 , 2400 , 4800 , 9600 , 19200 , 38400 , 57600 , 115200 };


typedef struct
{
    unsigned char TX[2048];
    unsigned char RX[2048];
    uint32_t TX_Count;
    uint32_t RX_Count;
} Buffer;

typedef struct
{
    //Is trace used.
    unsigned char trace;

    //Socked handle.
    int socket;
    //Serial port handle.
    int comPort;
    //Receiver thread handle.
    pthread_t receiverThread;
    unsigned long   waitTime;
    //Received data.
    gxByteBuffer data;
    //If receiver thread is closing.
    unsigned char closing;
    dlmsServerSettings settings;
    Buffer buffer;
} connection;

void con_initializeBuffers(
    connection* connection,
    int size);

int svr_listen(
    connection* con,
    unsigned short port);


uint16_t GetLinuxBaudRate(uint32_t baudRate);





int com_updateSerialportSettings(connection* con,
    unsigned char iec,
    uint32_t baudRate);





int com_initializeSerialPort(connection* con,
    char* serialPort,
    unsigned char iec);


void ListenerThread(void* pVoid);


void* UnixListenerThread(void* pVoid);

void* UnixSerialPortThread(void* pVoid);


void* Unixrs485RecSerialThread(void* pVoid);

void* Unixrs485SendSerialThread(void* pVoid);


int svr_listen_serial(
    connection* con,
     char *file);


int rs485_listen_serial(
    connection* con,
     char *file);


int svr_listen_TCP(
    connection* con,
    unsigned short port);




//Close connection..
int con_close(
    connection* con);




// void report(char *format, ... );



#ifdef  __cplusplus
}
#endif

#endif //CONNECTION_H