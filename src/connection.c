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
#include "../include/connection.h"
#include "../../development/include/server.h"

#include <stdlib.h> // malloc and free needs this or error is generated.
#include <stdio.h>

#define INVALID_HANDLE_VALUE -1
#include <unistd.h>
#include <errno.h> //Add support for sockets
#include <netdb.h> //Add support for sockets
#include <sys/types.h> //Add support for sockets
#include <sys/socket.h> //Add support for sockets
#include <netinet/in.h> //Add support for sockets
#include <arpa/inet.h> //Add support for sockets
#include <termios.h> // POSIX terminal control definitions
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h> // string function definitions
#include <unistd.h> // UNIX standard function definitions
#include <fcntl.h> // File control definitions
#include <errno.h> // Error number definitions

//Initialize connection buffers.
void con_initializeBuffers(connection * connection, int size)
{
    if (size == 0)
    {
        bb_clear(&connection->data);
    }
    else
    {
        //Allocate 50 bytes more because some meters count this wrong and send few bytes too many.
        bb_capacity(&connection->data, 50 + size);
    }
}

unsigned char isConnected(connection* con)
{
    return con->socket != -1;
}

void appendLog(unsigned char send, gxByteBuffer* reply)
{
#if _MSC_VER > 1400
    FILE * f = NULL;
    fopen_s(&f, "trace.txt", "a");
#else
    FILE* f = fopen("trace.txt", "a");
#endif
    if (f != NULL)
    {
        char* tmp = bb_toHexString(reply);
        if (tmp != NULL)
        {
            if (send)
            {
                fprintf(f, "TX: %s\r\n", tmp);
            }
            else
            {
                fprintf(f, "RX: %s\r\n", tmp);
            }
            free(tmp);
        }
        fclose(f);
    }
}
uint16_t GetLinuxBaudRate(uint32_t baudRate)
{
    uint16_t br;
    printf("baudRate in GetLinuxBaudRate = %d\n",baudRate);
    switch (baudRate) {
    case 110:
        br = B110;
        break;
    case 300:
        br = B300;
        break;
    case 600:
        br = B600;
        break;
    case 1200:
        br = B1200;
        break;
    case 2400:
        br = B2400;
        break;
    case 4800:
        br = B4800;
        break;
    case 9600:
        br = B9600;
        break;
    case 19200:
        br = B19200;
        break;
    case 38400:
        br = B38400;
        break;
    case 57600:
        br = B57600;
        break;
    case 115200:
        br = B115200;
        printf("B115200\n");
        break;
    default:
        return B9600;
    }
    return br;
}

int com_updateSerialportSettings(connection* con,
    unsigned char iec,
    uint32_t baudRate)
{
    int ret;
    struct termios options;
    memset(&options, 0, sizeof(options));
    options.c_iflag = 0;
    options.c_oflag = 0;
    if (iec)
    {
        options.c_cflag |= PARENB;
        options.c_cflag &= ~PARODD;
        options.c_cflag &= ~CSTOPB;
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS7;
        //Set Baud Rates
        cfsetospeed(&options, B300);
        cfsetispeed(&options, B300);
    }
    else
    {
        // 8n1, see termios.h for more information
        options.c_cflag = CS8 | CREAD | CLOCAL;
        /*
        options.c_cflag &= ~PARENB
        options.c_cflag &= ~CSTOPB
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;
        */
        //Set Baud Rates
        ret = GetLinuxBaudRate(baudRate) ;

        printf("ret = %d\n",ret);
        cfsetospeed(&options, GetLinuxBaudRate(baudRate));
        cfsetispeed(&options, GetLinuxBaudRate(baudRate));
    }
    options.c_lflag = 0;
    options.c_cc[VMIN] = 1;
    //How long we are waiting reply charachter from serial port.
    options.c_cc[VTIME] = 5;

    //hardware flow control is used as default.
    //options.c_cflag |= CRTSCTS;

    if (tcsetattr(con->comPort, TCSAFLUSH, &options) != 0)
    {
        ret = errno;
        printf("Failed to Open port. tcsetattr failed.\r");
        return DLMS_ERROR_TYPE_COMMUNICATION_ERROR | ret;
    }
    return 0;
}

int com_initializeSerialPort(connection* con,
    char* serialPort,
    unsigned char iec)
{
    int ret;
    // read/write | not controlling term | don't wait for DCD line signal.
    con->comPort = open(serialPort, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (con->comPort == -1) // if open is unsuccessful.
    {
        ret = errno;
        printf("Failed to open serial port: %s\n", serialPort);
        return DLMS_ERROR_TYPE_COMMUNICATION_ERROR | ret;
    }
    if (!isatty(con->comPort))
    {
        ret = errno;
        printf("Failed to Open port %s. This is not a serial port.\n", serialPort);
        return DLMS_ERROR_TYPE_COMMUNICATION_ERROR | ret;
    }
    uint32_t baudRate = Boudrate[con->settings.hdlc->communicationSpeed];
    printf("baudRate = %d\n",baudRate);

    return com_updateSerialportSettings(con,iec, baudRate);
}





void ListenerThread(void* pVoid)
{
    int socket;
    connection* con = (connection*)pVoid;
    struct sockaddr_in add;
    int ret;
    char tmp[10];
    socklen_t len;
    socklen_t AddrLen = sizeof(add);
    int pos;
    char* info;
    gxByteBuffer bb, reply, senderInfo;
    struct sockaddr_in client;
    //Get buffer data
    bb_init(&senderInfo);
    bb_init(&bb);
    bb_init(&reply);
    bb_capacity(&bb, 2048);
    memset(&client, 0, sizeof(client));
    while (isConnected(con))
    {
        len = sizeof(client);
        bb_clear(&senderInfo);
        socket = accept(con->socket, (struct sockaddr*) & client, &len);

        printf("socket of client = %d\n",socket);
        // printf("client = %s\n",client.sin_addr);
        // printf("len = %s\n",len);



        if (isConnected(con))
        {
            if ((ret = getpeername(socket, (struct sockaddr*) & add, &AddrLen)) == -1)
            {
                close(socket);
                socket = -1;
                continue;
                //Notify error.
            }
            // printf("client = %s\n",client.sin_addr);
            // printf("len = %s\n",len);
            info = inet_ntoa(add.sin_addr);
            bb_set(&senderInfo, (unsigned char*)info, (unsigned short)strlen(info));
            bb_setInt8(&senderInfo, ':');
            hlp_intToString(tmp, 10, add.sin_port, 0, 0);
            bb_set(&senderInfo, (unsigned char*)tmp, (unsigned short)strlen(tmp));
            while (isConnected(con))
            {
                //If client is left wait for next client.
                if ((ret = recv(socket, (char*)
                    bb.data + bb.size,
                    bb.capacity - bb.size, 0)) == -1)
                {
                    //Notify error.
                    svr_reset(&con->settings);
                    close(socket);
                    socket = -1;
                    break;
                }


                printf("receive from server = %s\n",&bb.data);

                
                //If client is closed the connection.
                if (ret == 0)
                {
                    svr_reset(&con->settings);
                    close(socket);
                    socket = -1;
                    break;
                }
                if (con->trace > GX_TRACE_LEVEL_WARNING)
                {
                    printf("\r\nRX %d:\t", ret);
                    for (pos = 0; pos != ret; ++pos)
                    {
                        printf("%.2X ", bb.data[bb.size + pos]);
                    }
                    printf("\r\n");
                }
                bb.size = bb.size + ret;
                appendLog(0, &bb);


                // printf("client = %s\n",client.sin_addr);
                // printf("len = %s\n",len);                
                if (svr_handleRequest(&con->settings, &bb, &reply) != 0)
                {
                    close(socket);
                    socket = -1;
                }
                bb.size = 0;
                if (reply.size != 0)
                {
                    if (con->trace > GX_TRACE_LEVEL_WARNING)
                    {
                        printf("\r\nTX %u:\t", (unsigned int)reply.size);
                        for (pos = 0; pos != reply.size; ++pos)
                        {
                            printf("%.2X ", reply.data[pos]);
                        }
                        printf("\r\n");
                    }
                    appendLog(1, &reply);
                    if (send(socket, (const char*)reply.data, reply.size - reply.position, 0) == -1)
                    {
                        //If error has occured
                        svr_reset(&con->settings);

                        close(socket);
                        socket = -1;
                    }
                    printf("send from server = %s\n",&reply.data);

                    bb_clear(&reply);
                }
            }
            svr_reset(&con->settings);
        }
    }
    bb_clear(&bb);
    bb_clear(&reply);
    bb_clear(&senderInfo);
}

void* UnixListenerThread(void* pVoid)
{
    ListenerThread(pVoid);
    return NULL;
}

//Initialize connection settings.
int svr_listen(
    connection* con,
    unsigned short port)
{
    struct sockaddr_in add = { 0 };
    int fFlag = 1;
    int ret;
    //Reply wait time is 5 seconds.
    con->waitTime = 5000;
    con->comPort = -1;
    con->receiverThread = -1;
    con->socket = -1;
    con->closing = 0;
    bb_init(&con->data);
    bb_capacity(&con->data, 50);

    con->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (!isConnected(con))
    {
        //socket creation.
        return -1;
    }
    if (setsockopt(con->socket, SOL_SOCKET, SO_REUSEADDR, (char*)& fFlag, sizeof(fFlag)) == -1)
    {
        //setsockopt.
        return -1;
    }
    add.sin_port = htons(port);
    add.sin_addr.s_addr = htonl(INADDR_ANY);
    add.sin_family = AF_INET;
    if ((ret = bind(con->socket, (struct sockaddr*) & add, sizeof(add))) == -1)
    {
        //bind;
        return -1;
    }
    if ((ret = listen(con->socket, 1)) == -1)
    {
        //socket listen failed.
        return -1;
    }

    ret = pthread_create(&con->receiverThread, NULL, UnixListenerThread, (void*)con);
    return ret;
}


void* UnixSerialPortThread(void* pVoid)
{
    int ret;
    unsigned char data;
    unsigned char first = 1;
    uint16_t pos;
    int bytesRead;
    gxServerReply sr;
    gxByteBuffer reply;
    connection* con = (connection*)pVoid;
    sr_initialize(&sr, &data, 1, &reply);

    while (1)
    {
        bytesRead = read(con->comPort, &data, 1);
        if (bytesRead < 1)
        {
            //If there is no data on the read buffer.
            if (errno != EAGAIN)
            {
                break;
            }
        }
        else
        {
            if (con->trace > GX_TRACE_LEVEL_WARNING)
            {
                if (first)
                {
                    printf("\nRX:\t");
                    first = 0;
                }
                printf("%.2X ", data);
            }


            if (svr_handleRequest4(&con->settings, &sr) != 0)
            {
                break;
            }
            if (reply.size != 0)
            {
                first = 1;
                if (con->trace > GX_TRACE_LEVEL_WARNING)
                {
                    printf("\nTX\t");
                    for (pos = 0; pos != reply.size; ++pos)
                    {
                        printf("%.2X ", reply.data[pos]);
                    }
                    printf("\n");
                }
                ret = write(con->comPort, reply.data, reply.size);
                if (ret != reply.size)
                {
                    printf("Write failed\n");
                }
                if (con->settings.base.interfaceType == DLMS_INTERFACE_TYPE_HDLC_WITH_MODE_E && sr.newBaudRate != 0)
                {
                    if (con->settings.base.connected == DLMS_CONNECTION_STATE_IEC)
                    {
                        /*Change baud rate settings if optical probe is used.*/
                        printf("%s %d", "Connected with optical probe. The new baudrate is:", sr.newBaudRate);
                        com_updateSerialportSettings(con,0, sr.newBaudRate);
                    }
                    else if (con->settings.base.connected == DLMS_CONNECTION_STATE_NONE)
                    {
                        //Wait until reply message is send before baud rate is updated.
                        //Without this delay, disconnect message might be cleared before send.

                        usleep(100000);
                        uint16_t baudRate = 300 << (int)con->settings.localPortSetup->defaultBaudrate;
                        printf("%s %d", "Disconnected with optical probe. The new baudrate is:", baudRate);
                        com_updateSerialportSettings(con,1, 300);
                    }
                }
                bb_clear(&reply);
            }
        }
    }
    return NULL;
}


void* Unixrs485RecSerialThread(void* pVoid)
{
    int ret;
    unsigned char data;
    unsigned char first = 1;
    int bytesRead;
    connection* con = (connection*)pVoid;


    while (1)
    {
        bytesRead = read(con->comPort, &data, 1);
        if (bytesRead < 1)
        {
            //If there is no data on the read buffer.
            if (errno != EAGAIN)
            {
                break;
            }
        }
        else
        {
            printf("data = %.2X\n",data);
            printf("con->buffer.RX_Count  = %d\n",con->buffer.RX_Count);
            con->buffer.RX[con->buffer.RX_Count] = data ;
            con->buffer.RX_Count++ ;
            printf("con->rs485.RX = ");
            for(int i = 0 ; i < con->buffer.RX_Count ; i++)
            {
                printf("%.2X ",con->buffer.RX[i]);
            }
            printf("\n");
            // if (con->trace > GX_TRACE_LEVEL_WARNING)
            // {
            //     if (first)
            //     {
            //         printf("\nRX:\t");
            //         first = 0;
            //     }
            //     printf("%.2X ", data);

            // }
        }
    }
    return NULL;
}





void* Unixrs485SendSerialThread(void* pVoid)
{
    int ret;
    connection* con = (connection*)pVoid;
    while (1)
    {
        if(con->buffer.TX_Count>0)
        {
            ret = write(con->comPort, con->buffer.TX, con->buffer.TX_Count);
            con->buffer.TX_Count = 0 ;
        }
        usleep(10000);
    }
    return NULL;
}











//Initialize connection settings.
int svr_listen_serial(
    connection* con,
     char *file)
{
    int ret;
    if (!isConnected(con))
    {
        //socket creation.
        return -1;
    }

    DLMS_INTERFACE_TYPE interfaceType;

    if(con->settings.localPortSetup->defaultMode==0) interfaceType=DLMS_INTERFACE_TYPE_HDLC_WITH_MODE_E;
    else interfaceType=DLMS_INTERFACE_TYPE_HDLC;
    if ((ret = com_initializeSerialPort(
        con,
        file,
        interfaceType == DLMS_INTERFACE_TYPE_HDLC_WITH_MODE_E)) != 0)
    {
        return ret;
    }
    ret = pthread_create(&con->receiverThread, NULL, UnixSerialPortThread, (void*)con);
    return ret;
}




//Initialize connection settings.
int rs485_listen_serial(
    connection* con,
     char *file)
{
    int ret;
    if ((ret = com_initializeSerialPort(
        con,
        file,
        0)) != 0)
    {
        return ret;
    }
    
    ret = pthread_create(&con->receiverThread, NULL, Unixrs485RecSerialThread, (void*)con);
    ret = pthread_create(&con->receiverThread, NULL, Unixrs485SendSerialThread, (void*)con);
    return ret;
}





//Initialize connection settings.
int svr_listen_TCP(
    connection* con,
    unsigned short port)
{
    struct sockaddr_in add = { 0 };
    int fFlag = 1;
    int ret;
    //Reply wait time is 5 seconds.
    con->waitTime = 5000;
    con->comPort = -1;
    con->receiverThread = -1;
    con->socket = -1;
    con->closing = 0;
    bb_init(&con->data);
    bb_capacity(&con->data, 50);

    con->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (!isConnected(con))
    {
        //socket creation.
        return -1;
    }
    if (setsockopt(con->socket, SOL_SOCKET, SO_REUSEADDR, (char*)& fFlag, sizeof(fFlag)) == -1)
    {
        //setsockopt.
        return -1;
    }
    add.sin_port = htons(port);
    add.sin_addr.s_addr = htonl(INADDR_ANY);
    add.sin_family = AF_INET;
    if ((ret = bind(con->socket, (struct sockaddr*) & add, sizeof(add))) == -1)
    {
        //bind;
        return -1;
    }
    if ((ret = listen(con->socket, 1)) == -1)
    {
        //socket listen failed.
        return -1;
    }

    ret = pthread_create(&con->receiverThread, NULL, UnixListenerThread, (void*)con);
    return ret;
}





//Close connection.
int con_close(
    connection* con)
{
    if (isConnected(con))
    {

        close(con->socket);
        con->socket = -1;
        void* res;
        pthread_join(con->receiverThread, (void**)& res);
        free(res);


        if (con->comPort != -1)
        {
            int ret = close(con->comPort);
            if (ret < 0)
            {
                printf("Failed to close port.\r\n");
            }
            con->comPort = -1;
        }
        con->socket = -1;
        bb_clear(&con->data);
        con->closing = 0;
        con_initializeBuffers(con, 0);
        svr_disconnected(&con->settings);
    }
    svr_clear(&con->settings);
    return 0;
}

// void report(char *format, ... )
// {
// 	va_list args;
// 	int i=0,count=0;
// 	char ss[100],str[400];
// 	va_start( args, format );

// 	int		d;
// 	float   f;
// 	char   *s;

// 	memset(str,0,sizeof(str));

// 	for( i = 0; format[i] != '\0'; ++i )
// 	{
// 		switch( format[i] )
// 		{
// 	         case 'd':
// 	        	d=va_arg(args, int );
// 	            sprintf(ss,"%d\0",d);
// 	         break;

// 	         case 'f':
// 	        	 f=va_arg(args, double );
// 	             sprintf(ss,"%f\0",f);
// 	         break;


// 	         case 's':
// 	        	 s=va_arg(args, char *);
// 	             sprintf(ss,"%s\0",s);
// 	         break;

// 	         default:
// 	         break;
// 	      }

// 			memcpy(str+count,ss,strlen(ss));
// 			count=strlen(str);

// 	}


// 	va_end( args );


// #if DEBUG_MODE
// 	printf("((%s)) %s\n",get_time(),str);
// #else
// 	char str1[500];
// 	memset(str1,0,sizeof(str1));
// 	sprintf(str1,"logger -t \"=======(  ZS-LRM200  )=======\" \"%s\" ",str);
// 	std::system(str1);
// #endif
// }
