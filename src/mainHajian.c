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


#define closesocket close
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h> //Add support for sockets
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>

#include "../include/exampleserver.h"
#include "../include/connection.h"
#include "../../development/include/cosem.h"
#include "../../development/include/gxaes.h"


#define HDLC_HEADER_SIZE 17

#define HDLC_BUFFER_SIZE 128
#define PDU_BUFFER_SIZE 1024
#define WRAPPER_BUFFER_SIZE 8 + PDU_BUFFER_SIZE

unsigned char lnframeBuff[HDLC_BUFFER_SIZE + HDLC_HEADER_SIZE];
unsigned char lnpduBuff[PDU_BUFFER_SIZE];

unsigned char ln47frameBuff[WRAPPER_BUFFER_SIZE];
unsigned char ln47pduBuff[PDU_BUFFER_SIZE];

int startServers(int port, int trace)
{
    int ret;

    connection lnWrapper , lniec , rs485;
    
   //Initialize DLMS settings.
    svr_init(&lnWrapper.settings, 1, DLMS_INTERFACE_TYPE_WRAPPER, WRAPPER_BUFFER_SIZE, PDU_BUFFER_SIZE, ln47frameBuff, WRAPPER_BUFFER_SIZE, ln47pduBuff, PDU_BUFFER_SIZE);
    // //We have several server that are using same objects. Just copy them.
    unsigned char KEK[16] = { 0x31,0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31 };
    BB_ATTACH(lnWrapper.settings.base.kek, KEK, sizeof(KEK));
    svr_InitObjects(&lnWrapper.settings);

    DLMS_INTERFACE_TYPE interfaceType;
    if(lnWrapper.settings.localPortSetup->defaultMode==0) interfaceType=DLMS_INTERFACE_TYPE_HDLC_WITH_MODE_E;
    else interfaceType=DLMS_INTERFACE_TYPE_HDLC;
 
    //Initialize DLMS settings.
    svr_init(&lniec.settings, 1, interfaceType, HDLC_BUFFER_SIZE, PDU_BUFFER_SIZE, lnframeBuff, HDLC_HEADER_SIZE + HDLC_BUFFER_SIZE, lnpduBuff, PDU_BUFFER_SIZE);
    
    svr_InitObjects(&lniec.settings);

    //Start server
    if ((ret = svr_start(&lnWrapper, port)) != 0)
    {
        return ret;
    }

    //Start server
    if ((ret = svr_start_Serial(&lniec, "/dev/ttyUSB0")) != 0)
    {
        return ret;
    }



    if()

    lnWrapper.trace =lniec.trace =  trace;



    uint32_t lastMonitor = 0;
    while (1)
    {
        //Monitor values only once/second.
        if (time_current() - lastMonitor > 1)
        {
            lastMonitor = time_current();
            if ((ret = svr_monitorAll(&lnWrapper.settings)) != 0)
            {
                printf("lnWrapper monitor failed.\r\n");
            }
        }
        
    }
    con_close(&lnWrapper);
    return 0;
}



int main(int argc, char* argv[])
{
    int  port = 4060;
    strcpy(DATAFILE, argv[0]);

    char* p = strrchr(DATAFILE, '/');
    *p = '\0';
    strcpy(IMAGEFILE, DATAFILE);
    strcpy(TRACEFILE, DATAFILE);
    //Add empty file name.
    strcat(IMAGEFILE, "/empty.bin");
    strcat(DATAFILE, "/data.csv");
    strcat(TRACEFILE, "/trace.txt");
    FILE* f = fopen(TRACEFILE, "w");
    fclose(f);   
    startServers(port, GX_TRACE_LEVEL_INFO);
    return 0;
}


