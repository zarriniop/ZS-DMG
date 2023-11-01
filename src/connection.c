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


pthread_t Wan_Connection_pthread_var;

extern gxGPRSSetup 	gprsSetup	;
extern gxData 		imei		;
extern gxData 		deviceid6	;
extern connection 	lnWrapper , lniec , rs485;
extern gxTcpUdpSetup udpSetup;
extern gxAutoConnect autoConnect;

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
    return con->socket.Socket_fd != -1;
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
//    printf("baudRate in GetLinuxBaudRate = %d\n",baudRate);
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
        break;
    default:
        return B9600;
    }
    return br;
}

int com_updateSerialportSettings(connection* con, unsigned char iec, uint32_t baudRate)
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

int Quectel_Update_Serial_Port_Settings(connection* con, unsigned char iec, uint32_t baudRate)
{
	ST_UARTDCB dcb;
    int ret;

    if (iec)
    {
    	dcb.parity = PB_EVEN;
    	dcb.stopbit = SB_1;
    	dcb.databit = DB_CS7;
    	dcb.baudrate = B_300;
    }
    else
    {
        // 8n1, see termios.h for more information
    	dcb.parity = PB_NONE;
    	dcb.stopbit = SB_1;
    	dcb.databit = DB_CS8;
    	dcb.baudrate = baudRate;
    }

    //hardware flow control is used as default.
    //options.c_cflag |= CRTSCTS;

    if (Ql_UART_SetDCB(con->comPort, &dcb) != 0)
    {
        ret = errno;
        printf("Failed to Open port. tcsetattr failed.\r");
        return DLMS_ERROR_TYPE_COMMUNICATION_ERROR | ret;
    }
    return 0;
}

int com_initializeSerialPort(connection* con, char* serialPort, unsigned char iec)
{
    int ret;
    // read/write | not controlling term | don't wait for DCD line signal.
//    con->comPort = open(serialPort, O_RDWR | O_NOCTTY | O_NONBLOCK);

    con->comPort = Ql_UART_Open(serialPort, B_9600, FC_NONE);
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

//    return com_updateSerialportSettings(con,iec, baudRate);
    return Quectel_Update_Serial_Port_Settings(con, iec, baudRate);
}


void Socket_Receive_Thread(void* pVoid)
{
    connection* con = (connection*)pVoid;
    int ret;
    char tmp[2048];
    int pos;
    gxByteBuffer reply;
    struct sockaddr_in client;
    //Get buffer data
    bb_init(&reply);
    memset(&client, 0, sizeof(client));

    while (1)
    {
        if (con->socket.Status.Connected == true)
        {
			//If client is left wait for next client.

//        	 if ((ret = recv(con->socket.Socket_fd, (char*) bb.data + bb.size, bb.capacity - bb.size, 0)) == -1)

        	ret = recv(con->socket.Socket_fd, tmp, 2048, 0);

			if(ret <= 0)
        	{
				//Notify error.
				svr_reset(&con->settings);
				con->socket.Status.Connected = false;
        	}
			else
			{
				if (con->trace > GX_TRACE_LEVEL_WARNING)
				{
					unsigned char tcp_client_rx[4096] = {0};
					for (pos = 0; pos != ret; ++pos)
					{
						sprintf(tcp_client_rx + strlen(tcp_client_rx), "%.2X ", tmp[pos]);
					}
					report(CLIENT, RX, tcp_client_rx);
				}

	//			appendLog(0, &bb);

				if(tmp[8] == 0xE6)
				{
					memcpy(con->buffer.RX, tmp, ret);
					con->buffer.RX_Count += ret;
//					printf("Checking 0xE6 - rx:%d\n", con->buffer.RX_Count);
				}
				else
				{
					if (svr_handleRequest2(&con->settings, &tmp, ret, &reply) != 0)
					{

					}
				}

				if (reply.size != 0)
				{
					if (con->trace > GX_TRACE_LEVEL_WARNING)
					{
						unsigned char tcp_client_tx[4096] = {0};
						for (pos = 0; pos != reply.size; ++pos)
						{
							sprintf(tcp_client_tx + strlen(tcp_client_tx), "%.2X ", reply.data[pos]);
						}
						report(CLIENT, TX, tcp_client_tx);
					}
	//				appendLog(1, &reply);

					memcpy(con->buffer.TX, (const char*)reply.data, reply.size - reply.position);
					con->buffer.TX_Count = reply.size - reply.position;

					bb_clear(&reply);
				}
			}
        }
        usleep(1000);
    }
}



void Socket_Send_Thread(void* pVoid)
{
    connection* con = (connection*)pVoid;
    while (1)
    {

        if (con->socket.Status.Connected == true && con->buffer.TX_Count>0)
        {
			if (send(con->socket.Socket_fd, con->buffer.TX, con->buffer.TX_Count, 0) == -1)
			{
				//If error has occured
				svr_reset(&con->settings);
				con->socket.Status.Connected = false;
			}
			else
			{
				unsigned char tcp_client_tx[4096] = {0};
		    	for (int m=0; m<con->buffer.TX_Count; m++)
		    	{
		    		sprintf(tcp_client_tx + strlen(tcp_client_tx), "%.2X ",con->buffer.TX[m]);
		    	}
		    	report(CLIENT, TX, tcp_client_tx);

		    	con->buffer.TX_Count = 0;
			}

        }
        if(con->serversocket.Status.Connected == true && con->buffer.TX_Count>0)
        {
			if (send(con->serversocket.Accept_fd, con->buffer.TX, con->buffer.TX_Count, 0) == -1)
			{
				//If error has occured
				svr_reset(&con->settings);
				con->serversocket.Status.Connected = false;
			}
			else
			{
				unsigned char tcp_svr_tx[4096] = {0};
				for (int m=0; m<con->buffer.TX_Count; m++)
				{
					sprintf(tcp_svr_tx + strlen(tcp_svr_tx), "%.2X ",con->buffer.TX[m]);
				}
				report(SERVER, TX, tcp_svr_tx);

				con->buffer.TX_Count = 0;
			}
        }
        usleep(1000);
    }
}


//Initialize connection settings.
int Socket_Connection_Start(connection* con)
{
	gxByteBuffer* tmp_bb		;
    struct sockaddr_in add = { 0 };
    int ret;
    //Reply wait time is 5 seconds.
    con->waitTime = 5000;
    con->comPort = -1;
    con->receiverThread = -1;
    con->socket.Socket_fd = -1;
    con->serversocket.Socket_fd = -1;
    con->closing = 0;
    bb_init(&con->data);
    bb_capacity(&con->data, 50);

    con->buffer.TX_Count = 0;
    con->buffer.RX_Count = 0;

    con->socket.Status.Connected 		= false	;
    con->socket.Status.Opened 			= true	;
    con->serversocket.Status.Connected 	= false	;

    arr_getByIndex(&autoConnect.destinations, 0, (void**)&tmp_bb)		;
//    char *IP_Port= bb_toString(tmp_bb)								;
    sprintf(con->socket.Parameters.IP, strtok(bb_toString(tmp_bb),":"))	;		//converting server port-ip type to string, IP:PORT
    con->socket.Parameters.PORT 	= atoi(strtok(NULL,":"))			;
    con->serversocket.server_port 	= udpSetup.port						;

    ret = pthread_create(&con->receiverThread, 		NULL, Socket_Receive_Thread	, (void*)con);
    ret = pthread_create(&con->sendThread, 			NULL, Socket_Send_Thread	, (void*)con);
    ret = pthread_create(&con->managerThread, 		NULL, Socket_Manage_Thread	, (void*)con);
    ret = pthread_create(&con->serverstart, 		NULL, Socket_Server			, (void*)con);
    return ret;
}


void Socket_get_open(connection* con)
{
	if(con->socket.Status.Opened == true)							//===============( Close Connection )======================
	{
		Socket_get_close(con);
		con->socket.Status.Opened=false;
																	//===============( Create Socket )=========================
		if(Socket_create(con) == 1)
		{
			report(CLIENT, CONNECTION,"OPEN");
		}
	}
																	//===============( Connect to Server )=========================

	if(connect(con->socket.Socket_fd, (struct sockaddr *)&con->socket.Serv_addr, sizeof(con->socket.Serv_addr)) == 0)
	{
		con->socket.Status.Connected	=true;
		con->socket.Status.Opened		=true;

		report(CLIENT, CONNECTION,"CONNECTED");
	}
	else
	{
		con->socket.Status.Connected=false;
		report(CLIENT, CONNECTION,"CLOSE");
	}

}


int Socket_Manage_Thread (connection* con)
{
	uint8_t i=0;
	uint8_t r1=0,r2=0;

//	report("s","RS232 --> TCP_CLIENT_CONNECTION_CHECK_Thread Started !");

	while(1)
	{
		if(con->socket.Status.Connected == false)
		{
//			printf("Socket_get_open\n");
			Socket_get_open(con);
		}

//		con->socket.Error=0;
//		con->socket.ErrorLen=sizeof(con->socket.Error);
//		getsockopt (con->socket.Socket_fd, SOL_SOCKET, SO_ERROR, &con->socket.Error, &con->socket.ErrorLen);

//		if(con->socket.Error!=0) report("sdsd","RS232 --> TCP-Client:Socket",i,"--> Connection Error =",con->socket.Error);

		sleep(1);

	}
}


int Socket_Server(connection* con)
{
    struct sockaddr_in add = { 0 };
    int fFlag = 1;
    int ret;
    //Reply wait time is 5 seconds.
    con->waitTime 			= 5000;
    con->comPort 			= -1;
    con->receiverThread 	= -1;
//    con->socket 			= -1;
    con->serversocket.Socket_fd	= -1;
    con->serversocket.Accept_fd = -1;
    con->closing 			= 0 ;
    bb_init(&con->data);
    bb_capacity(&con->data, 50);

    con->serversocket.Status.Connected = false;

    con->serversocket.Socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (con->serversocket.Socket_fd == -1)
    {
    	printf("ERROR - SERVER - SOCKET CREATED \n");
        //socket creation.
        return -1;
    }
    if (setsockopt(con->serversocket.Socket_fd, SOL_SOCKET, SO_REUSEADDR, (char*)& fFlag, sizeof(fFlag)) == -1)
    {
        //setsockopt.
        return -1;
    }
    add.sin_port 		= htons(con->serversocket.server_port);
    add.sin_addr.s_addr = htonl(INADDR_ANY);
    add.sin_family		= AF_INET;
    if ((ret = bind(con->serversocket.Socket_fd, (struct sockaddr*) & add, sizeof(add))) == -1)
    {
        //bind;
    	printf("ERROR - SERVER - SOCKET BIND \n");
        return -1;
    }
    if ((ret = listen(con->serversocket.Socket_fd, 1)) == -1)
    {
        //socket listen failed.
        return -1;
    }

    ret = pthread_create(&con->serverlistenThread, 	NULL, Socket_Listen_Thread	, (void*)con);
    return ret;
}


void Socket_Listen_Thread(void* pVoid)
{
    connection* con = (connection*)pVoid;
    struct sockaddr_in add;
    int ret;
    char tmp[2048];
    socklen_t len;
    socklen_t AddrLen = sizeof(add);
    int pos;
    char* info;
    gxByteBuffer reply, senderInfo;
    struct sockaddr_in client;
    //Get buffer data
    bb_init(&senderInfo);
    bb_init(&reply);
    memset(&client, 0, sizeof(client));
    while (con->serversocket.Socket_fd != -1)
    {
        len = sizeof(client);
        bb_clear(&senderInfo);
        con->serversocket.Accept_fd = accept(con->serversocket.Socket_fd, (struct sockaddr*) &client, &len);

//        printf("socket of client = %d\n",con->serversocket.Accept_fd);
        // printf("client = %s\n",client.sin_addr);
        // printf("len = %s\n",len);

        if (con->serversocket.Socket_fd != -1)
        {
//        	printf("SERVER - SOCKET ACCEPTED \n");
            if ((ret = getpeername(con->serversocket.Accept_fd, (struct sockaddr*) & add, &AddrLen)) == -1)
            {
                close(con->serversocket.Accept_fd);
                con->serversocket.Accept_fd = -1;
                continue;
                //Notify error.
            }
            con->serversocket.Status.Connected = true;
            report(SERVER, CONNECTION, "CONNECTED");

//            info = inet_ntoa(add.sin_addr);
//            bb_set(&senderInfo, (unsigned char*)info, (unsigned short)strlen(info));
//            bb_setInt8(&senderInfo, ':');
//            hlp_intToString(tmp, 10, add.sin_port, 0, 0);
//            bb_set(&senderInfo, (unsigned char*)tmp, (unsigned short)strlen(tmp));

            while (con->serversocket.Socket_fd != -1)
            {
    			//If client is left wait for next client.

    //        	 if ((ret = recv(con->socket.Socket_fd, (char*) bb.data + bb.size, bb.capacity - bb.size, 0)) == -1)
    			if ((ret = recv(con->serversocket.Accept_fd, tmp, 2048, 0)) <= 0)
    			{
    				//Notify error.
                    //Notify error.
                    svr_reset(&con->settings);
                    close(con->serversocket.Accept_fd);
                    con->serversocket.Accept_fd = -1;
                    con->serversocket.Status.Connected = false;
                    report(SERVER, CONNECTION, "CLOSE");
                    break;
    			}


    			if (con->trace > GX_TRACE_LEVEL_WARNING)
    			{
    				unsigned char tcp_svr_rx[4096] = {0};
    				for (pos = 0; pos != ret; ++pos)
    				{
    					sprintf(tcp_svr_rx + strlen(tcp_svr_rx), "%.2X ", tmp[pos]);
    				}
    				report(SERVER, RX, tcp_svr_rx);
    			}

    //			appendLog(0, &bb);

    			if(tmp[8] == 0xE6)
    			{

    				memcpy(con->buffer.RX, tmp, ret);
    				con->buffer.RX_Count = ret;
    			}
    			else
    			{
    //				struct timeval  tv,strt;
    //				long diff=0;

    				if (svr_handleRequest2(&con->settings, &tmp, ret, &reply) != 0)
    				{
                        close(con->serversocket.Accept_fd);
                        con->serversocket.Accept_fd = -1;
    				}

    //				 gettimeofday (&tv, NULL);
    //				 diff=tv.tv_usec - strt.tv_usec;
    //				 if(diff < 0) diff+=1000000;
    //				 printf("######### Delay in svr_handleRequest2=  %d\r\n",diff);

    			}

    			if (reply.size != 0)
    			{
    				if (con->trace > GX_TRACE_LEVEL_WARNING)
    				{
    					unsigned char svr_tx_tmp[4096] = {0};
    					for (pos = 0; pos != reply.size; ++pos)
    					{
    						sprintf(svr_tx_tmp + strlen(svr_tx_tmp), "%.2X ", reply.data[pos]);
    					}
    					report(SERVER, TX, svr_tx_tmp);
    				}
    //				appendLog(1, &reply);

    				memcpy(con->buffer.TX, (const char*)reply.data, reply.size - reply.position);
    				con->buffer.TX_Count = reply.size - reply.position;

    				bb_clear(&reply);
    			}
    //            svr_reset(&con->settings);
            }
            report(SERVER, CONNECTION, "DISCONNECTED");
            svr_reset(&con->settings);
        }
    }
    bb_clear(&reply);
//    bb_clear(&senderInfo);
}


int	Socket_get_close(connection* con)
{
	int sh=shutdown(con->socket.Socket_fd, SHUT_RDWR);
	int cl=close(con->socket.Socket_fd);

	//printf("s:%d  shut:%d  close:%d  \n",n,sh,cl);
	con->socket.Socket_fd=0;
	con->socket.Status.Connected=false;
	con->socket.Status.Opened=false;
	report(CLIENT, CONNECTION, "CLOSE");
	return 1;
}

int Socket_create(connection* con)								//===============( Create client )======================
{
	int fFlag = 1;

	if ((con->socket.Socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
//		report("sds","SOCKET --> Client Socket",n," --> Create error !");
		return -1;
	}

	con->socket.Serv_addr.sin_family = AF_INET;
	con->socket.Serv_addr.sin_port = htons(con->socket.Parameters.PORT);
	if(inet_pton(AF_INET, con->socket.Parameters.IP, &con->socket.Serv_addr.sin_addr)<=0)
	{

//		report("sds","SOCKET --> Client Socket",n," --> Invalid address !");
		printf("SOCKET --> Client Socket--> Invalid address !\n");

		con->socket.Status.Valid = 0;		//0 means false
		return -1;
	}
	else con->socket.Status.Valid = 1;		//1 means true

	if (setsockopt(con->socket.Socket_fd, SOL_SOCKET, SO_REUSEADDR, (char*)& fFlag, sizeof(fFlag)) == -1)
	{
		//setsockopt.
		return -1;
	}

	con->socket.Status.Connected = 0;

	return 1;
}



void* IEC_Serial_Thread(void* pVoid)
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
//                        com_updateSerialportSettings(con,0, sr.newBaudRate);
                        Quectel_Update_Serial_Port_Settings(con,0, sr.newBaudRate);
                    }
                    else if (con->settings.base.connected == DLMS_CONNECTION_STATE_NONE)
                    {
                        //Wait until reply message is send before baud rate is updated.
                        //Without this delay, disconnect message might be cleared before send.

                        usleep(100000);
                        uint16_t baudRate = 300 << (int)con->settings.localPortSetup->defaultBaudrate;
                        printf("%s %d", "Disconnected with optical probe. The new baudrate is:", baudRate);
//                        com_updateSerialportSettings(con,1, 300);
                        Quectel_Update_Serial_Port_Settings(con,1, 300);
                    }
                }
                bb_clear(&reply);
            }
        }
    }
    return NULL;
}


void* RS485_Receive_Thread(void* pVoid)
{
    int ret;
    unsigned char data[1024];
    unsigned char first = 1;
    int bytesRead;
    connection* con = (connection*)pVoid;
    Buffer temp;
    struct timeval  tim;

    while (1)
    {
    	/*
        bytesRead = read(con->comPort, &data, 1024);
        printf("RS485\n");
    	if(bytesRead>0)
    	{
    		gettimeofday (&tim, NULL);
    		if(temp.RX_Count + bytesRead < 2048)
    		{
    			printf("RCV 485:%d\n", bytesRead);
    			memcpy(temp.RX + temp.RX_Count, data, bytesRead);
    			temp.RX_Count += bytesRead;
    			bytesRead = 0;
    		}
    	}
    	else
    	{
    		if(temp.RX_Count > 0)
    		{
    			//==============( Send On Timeout )=================
//    			if( diff_time_ms(&tim) > 100)
    			{
    				memcpy(con->buffer.RX, temp.RX, temp.RX_Count);
    				con->buffer.RX_Count = temp.RX_Count;

    				temp.RX_Count = 0;

    	            printf("con->rs485.RX = ");
    	            for(int i = 0 ; i < con->buffer.RX_Count ; i++)
    	            {
    	                printf("%.2X ",con->buffer.RX[i]);
    	            }
    	            printf("\n");
    			}
    		}
    	}
    	*/

    	bytesRead = read(con->comPort, &con->buffer.RX, 1024);
    	con->buffer.RX_Count = bytesRead;

    	unsigned char rs_rx_tmp_info[4096] = {0};
    	for (int m=0; m<con->buffer.RX_Count; m++)
    	{
    		sprintf(rs_rx_tmp_info + strlen(rs_rx_tmp_info) ,"%.2X ",con->buffer.RX[m]);
    	}
    	report(RS485, RX, rs_rx_tmp_info);

    	usleep(1000);
    }

    return NULL;
}


void* RS485_Send_Thread(void* pVoid)
{
    int ret;
    uint32_t baudRate;
    uint32_t delay_us;
    connection* con = (connection*)pVoid;
    while (1)
    {
        if(con->buffer.TX_Count>0)
        {
        	system("echo 1 > /sys/class/leds/DIR_485/brightness");
            ret = write(con->comPort, con->buffer.TX, con->buffer.TX_Count);
//            ret = Ql_UART_Write(con->comPort, con->buffer.TX, con->buffer.TX_Count);

            baudRate = Boudrate[con->settings.hdlc->communicationSpeed];
            delay_us = ((con->buffer.TX_Count * 12000)/baudRate);
//            printf("RS485 Send : %d\n", con->buffer.TX_Count);

            unsigned char rs_tx_tmp_info[4096] = {0};
        	for (int m=0; m<con->buffer.TX_Count; m++)
        	{
        		sprintf(rs_tx_tmp_info + strlen(rs_tx_tmp_info) ,"%.2X " ,con->buffer.TX[m]);
        	}
        	report(RS485, TX, rs_tx_tmp_info);

            con->buffer.TX_Count = 0 ;

            usleep(delay_us*1000);

            system("echo 0 > /sys/class/leds/DIR_485/brightness");
        }
        usleep(1000);
//        usleep(10000);
    }
    return NULL;
}


//Initialize connection settings.
int IEC_Serial_Start(connection* con, char *file)
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
    if ((ret = com_initializeSerialPort(con, file, interfaceType == DLMS_INTERFACE_TYPE_HDLC_WITH_MODE_E)) != 0)
    {
        return ret;
    }
    ret = pthread_create(&con->receiverThread, NULL, IEC_Serial_Thread, (void*)con);
    return ret;
}




//Initialize connection settings.
int RS485_Serial_Start(connection* con, char *file)
{
    int ret;
    if ((ret = com_initializeSerialPort(con, file, 0)) != 0)
    {
    	report(RS485, CONNECTION, "CLOSE");
        return ret;
    }
    
    report(RS485, CONNECTION, "OPEN");
    con->buffer.RX_Count = 0 ;
    con->buffer.TX_Count = 0 ;
    con->buffer.Timeout_ms = 5000;
    ret = pthread_create(&con->receiverThread, 	NULL, RS485_Receive_Thread, (void*)con);
    ret = pthread_create(&con->sendThread, 		NULL, RS485_Send_Thread, 	(void*)con);
    ret = pthread_create(&con->managerThread, 	NULL, GW_Start, 			(void*)con);
    return ret;
}


void GW_Start (void* pVoid)
{
	connection* con = (connection*)pVoid;
	GW_Run(&lnWrapper.buffer , &rs485.buffer);
}

//Close connection.
int con_close(
    connection* con)
{
    if (isConnected(con))
    {

        close(con->socket.Socket_fd);
        con->socket.Socket_fd = -1;
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
        con->socket.Socket_fd = -1;
        bb_clear(&con->data);
        con->closing = 0;
        con_initializeBuffers(con, 0);
        svr_disconnected(&con->settings);
    }
    svr_clear(&con->settings);
    return 0;
}


void Initialize (void)
{
	Device_Init	()	;
	Sim_Init	()	;
	NW_Init		()	;
	WAN_Init	()	;
}


void LTE_Manager_Start (void)
{
	Initialize();
	int thread_ret = pthread_create(&Wan_Connection_pthread_var	, NULL, WAN_Connection	, NULL);
	IMEI_Get();
	ICCID_Get();
}


void IMEI_Get (void)
{
	QL_DEV_ERROR_CODE				Ret_Dev					;
	char 							imei_buf[20]={0}		;
	Ret_Dev = ql_dev_get_imei(&imei_buf)					;
	var_setString(&imei.value, imei_buf, 20)				;
//	printf("Ret - get imei:%d - strlen:%d - IMEI:%s\n", Ret_Dev, strlen(imei_buf), imei_buf);
}


void Sim_Init (void)
{
	int ret = 0;

	ret = ql_sim_release()	;
	ret = ql_sim_init()		;

	if (ret)
	{
		printf("!ERROR! - SIMCARD INITIALIZING - RET:%d\n", ret);
	}
//	else
//	{
//		printf("SIMCARD INITIALIZE DONE\n");
//	}
}


void ICCID_Get (void)
{
	char* ICCID_pointer;
    char iccid[32]={0};
    int ret = ql_sim_get_iccid(iccid,32);
    var_setString(&deviceid6.value, iccid, 32);
//    printf("ICCID - RET:%d , iccid:%s\n", ret, iccid);
}


void Device_Init (void)
{
	int ret = ql_dev_init();

	if(ret)
	{
		printf("!ERROR! INITIALIZE DEVICE - RET:%d\n", ret);
	}
//	else
//	{
//		printf("DEVICE INITIALIZE DONE\n");
//	}
}

/************************************/
/*** Network Initialize Function ****/
/************************************/
void NW_Init (void)
{
	int ret = ql_nw_release();
	if(ret!=0) printf("!ERROR! ql_nw_release - ret:%d", ret);

	ret = ql_nw_init();
	if(ret!=0) printf("!ERROR! ql_nw_init - ret:%d", ret);
}

/************************************/
/**** WAN Initializing Function *****/
/************************************/
void WAN_Init (void)
{
	int ret = ql_wan_release()	;
	if(ret!=0) printf("!Error! ql_wan_release - ret:%d\n", ret);

	ret = ql_wan_init()		;
	if(ret!=0) printf("!Error! ql_wan_init - ret:%d\n", ret);

//	printf("<= WAN Initialization: DONE! =>\n");
}

/************************************/
/***** WAN Connection Function ******/
/************************************/
void WAN_Connection (void)
{
	QL_DSI_AUTH_PREF_T 				auth			;
	ql_data_call_info 				payload			;
	APN_PARAM_STRUCT_TYPEDEF		APN_Param_Struct;
	nw_status_cb 					nw_cb			;
	QL_NW_ERROR_CODE 				Ret_Signal		;
	QL_NW_SIGNAL_STRENGTH_INFO_T 	Sig_Strg_Info	;
	QL_NW_ERROR_CODE 				Ret_Cell_Info	;
	QL_NW_CELL_INFO_T				NW_Cell_Info	;
	uint8_t 						blink_no	= 0	;

	memset(&Sig_Strg_Info,0,sizeof(QL_NW_SIGNAL_STRENGTH_INFO_T));

	APN_Param_Struct.op = START_A_DATA_CALL							;
	APN_Param_Struct.apn= bb_toString(&gprsSetup.apn)				;
//	printf("<= WAN Connection - APN:%s =>\n", APN_Param_Struct.apn)	;

	APN_Param_Struct.profile_idx 	= 1		;
	APN_Param_Struct.ip_type		= IPV4V6;


	int ret = ql_wan_setapn(APN_Param_Struct.profile_idx, APN_Param_Struct.ip_type, APN_Param_Struct.apn, &APN_Param_Struct.userName, &APN_Param_Struct.password, auth);
	if(ret!=0) printf("!ERROR! ql_wan_setapn - ret:%d\n", ret);

	int 	ip_type_get 	= 0	;
    char 	apn_get[128]	={0};
    char 	userName_get[64]={0};
    char 	password_get[64]={0};

    ret = ql_wan_getapn(APN_Param_Struct.profile_idx, &ip_type_get, apn_get, sizeof(apn_get), userName_get, sizeof(userName_get), password_get, sizeof(password_get));
	if(ret!=0) printf("!ERROR! ql_wan_getapn - ret:%d\n", ret);
//	else
//	{
//		printf("<= APN SET:%s =>\n", apn_get);
//	}


	ret = ql_wan_start(APN_Param_Struct.profile_idx, APN_Param_Struct.op, nw_cb);
//	if(ret!=0) printf("ql_wan_start-ret:%d", ret);


	while(1)
	{
		ret = ql_get_data_call_info(APN_Param_Struct.profile_idx, &payload);

		if (ret == 0)
		{
			if (payload.v4.state == 1)			//Data call status is Activation
			{
//				if(Flags_Struct.WAN_Struct.WAN_IPv4_Started == DOWN)
//				{
//					Flags_Struct.WAN_Struct.WAN_IPv4_Started 	= UP;
//					Flags_Struct.TCP_Struct.TCP_Ready_To_Start 	= UP;
//				}

				Ret_Signal 		= ql_nw_get_signal_strength (&Sig_Strg_Info);
				Ret_Cell_Info 	= ql_nw_get_cell_info		(&NW_Cell_Info)	;

//				printf("<= IP:%s - RSSI:%d - GSM:%d - UMTS:%d - LTE:%d =>\n", payload.v4.addr.ip, Sig_Strg_Info.LTE_SignalStrength.rssi, NW_Cell_Info.gsm_info_valid, NW_Cell_Info.umts_info_valid, NW_Cell_Info.lte_info_valid);

//				if		(NW_Cell_Info.lte_info_valid == 1)
//					blink_no = 6;
//
//				else if(NW_Cell_Info.umts_info_valid == 1)
//					blink_no = 4;
//
//				else if(NW_Cell_Info.gsm_info_valid == 1)
//					blink_no = 2;
//
//				while(blink_no > 0)
//				{
//					if(blink_no%2 == 0)
//						system("echo 1 > /sys/devices/platform/leds/leds/LED3/brightness");
//
//					else
//						system("echo 0 > /sys/devices/platform/leds/leds/LED3/brightness");
//
//					usleep(100000);
//
//					blink_no --;
//				}


//				printf("<= data_call_info v4: {\n    profile_idx:%d,\n    ip_type:%d,\n    state:%d,\n    ip:%s,\n    name:%s,\n    gateway:%s,\n    pri_dns:%s,\n    sec_dns:%s\n} =>\n",
//					payload.profile_idx, payload.ip_type, payload.v4.state, payload.v4.addr.ip, payload.v4.addr.name, payload.v4.addr.gateway,
//					payload.v4.addr.pri_dns, payload.v4.addr.sec_dns);

//				#define QUEC_AT_PORT        "/dev/smd1"
//				int smd_fd = 0;
//				smd_fd = open(QUEC_AT_PORT, O_RDWR | O_NONBLOCK | O_NOCTTY);
//				printf("<= open(\"%s\")=%d =>\n", QUEC_AT_PORT, smd_fd);
			}
			else
			{
//				Flags_Struct.WAN_Struct.WAN_IPv4_Started 	= DOWN;
//				Flags_Struct.TCP_Struct.TCP_Ready_To_Start 	= DOWN;

//				ret = ql_wan_release()	;
//				if(ret!=0)
//					printf("ql_wan_release - ret:%d", ret);
//				else
//					printf("<= WAN RELEASED! =>\n");
//
//				ret = ql_wan_init()		;
//				if(ret!=0)
//					printf("ql_wan_init - ret:%d", ret);
//				else
//					printf("<= WAN INITIALIZED! =>\n");

				WAN_Init();

				ret = ql_wan_start(APN_Param_Struct.profile_idx, APN_Param_Struct.op, nw_cb);
				if(ret!=0)
					printf("!ERROR! ql_wan_start - ret:%d\n", ret);
//				else
//					printf("<= WAN STARTED! nw_cb=%d =>\n", nw_cb);
			}
		}
		else
		{
			printf("!ERROR! ql_get_data_call_info - ret:%d", ret);
		}
		sleep(2);
	}
}


//calculating time differential based on micro-second
long diff_time_us(struct timeval *start)
{
	 struct timeval  tv;
	 long diff=0;
	 gettimeofday (&tv, NULL);
	 diff=tv.tv_usec - start->tv_usec;
	 if(diff < 0) diff+=1000000;

	 return diff;
}

//calculating time differential based on milli-second
long diff_time_ms(struct timeval *start)
{
	 struct timeval  tv;
	 long diff=0;
	 gettimeofday (&tv, NULL);
	 diff=tv.tv_usec - start->tv_usec;
	 if(diff < 0) diff += 1000000;
	 diff = diff/1000;
	 diff += ((tv.tv_sec - start->tv_sec)*1000);
	 return diff;
}




int PushSetup_OnConnectivity()
{
    int ret;
    extern gxPushSetup pushSetup;
    message messages;
    gxByteBuffer* bb;
    mes_init(&messages);
    dlmsSettings cl;

    cl_init(&cl, 1, 1, 102, DLMS_AUTHENTICATION_HIGH_GMAC, NULL, DLMS_INTERFACE_TYPE_WRAPPER);

    char *str1= bb_toString(&pushSetup.destination);
    char *IP, *Port;
    IP = strtok(str1,":");
    printf("IP= %s\n",IP);
    Port = strtok(NULL,":");
    printf("Port= %s\n",Port);
    int Socket;
    ret = connectServer_pushon(IP, Port, &Socket) ;

    printf("ret of connectServer_pushon = %d\n",ret);

    ret = notify_generatePushSetupMessages(&cl, NULL, &pushSetup, &messages);

    for (int pos = 0; pos != messages.size; ++pos)
    {
        bb = messages.data[pos];
        if ((ret = send(Socket, bb->data, bb->size, 0)) == -1)
        {
            mes_clear(&messages);
            break;
        }

        for (int n = 0; n < bb->size; n++)
        {
            printf("%.2X-", bb->data[n]);
        }
        printf("\n");
    }
    mes_clear(&messages);
    closeServer(&Socket);
    cl_clear(&cl);
}

int connectServer_pushon(char* address, char *port, int* s)
{
    int ret;
    struct sockaddr_in add;
    //create socket.
    *s = socket(AF_INET, SOCK_STREAM, 0);
    if (*s == -1)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    int port_i = atoi(port);

    add.sin_port = htons(port_i);
    add.sin_family = AF_INET;
    add.sin_addr.s_addr = inet_addr(address);
    //If address is give as name
    if (add.sin_addr.s_addr == INADDR_NONE)
    {
        struct hostent* Hostent = gethostbyname(address);
        if (Hostent == NULL)
        {
            int err = errno;
            closeServer(s);
            return err;
        };
        add.sin_addr = *(struct in_addr*)(void*)Hostent->h_addr_list[0];
    };

    //Connect to the meter.
    ret = connect(*s, (struct sockaddr*)&add, sizeof(struct sockaddr_in));
    if (ret == -1)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    };
    return DLMS_ERROR_CODE_OK;

}


int closeServer(int* s)
{
    if (*s != -1)
    {
        close(*s);
        * s = -1;
    }
    return 0;
}



