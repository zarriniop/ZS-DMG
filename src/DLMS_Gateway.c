/*
 * DLMS_Gateway.c
 *
 *  Created on: Oct 2, 2023
 *      Author: mhn
 */

#include <DLMS_Gateway.h>

/****************
 *	Variables	*
 ****************/
uint8_t 				GW_State 				= READY_TO_GENERATE_SNRM;
uint32_t 				Timer 					= 0;
int8_t 					Handle_GW_Frame_Ret 	= 0;

uint8_t 				Gate_Meter_Prtcl[500]	;
uint32_t				Gate_Meter_size			;
uint8_t 				Gate_HES_Prtcl[500]		;
uint32_t				Gate_HES_size			;

uint8_t Ctrl_Byte_Last 				= 0				;
uint8_t Ctrl_Byte					= 0				;
uint8_t Segment_bit					= 1				;
uint8_t Control_Byte_Remained_Data 	= 0x51			;
uint8_t Buffer_Data_Meter2GW_Frame_Convertor[8152]	;
uint16_t Last_Byte_Buffer_Meter2GW 	= 0				;


/***********************************************************************************************
 * Gateway Runnig Function - Managing and converting GW2HDLC and HDLC2GW
 ***********************************************************************************************/
void GW_Run (GW_STRUCT_TYPEDEF* GW_STRUCT, HDLC_STRUCT_TYPEDEF* HDLC_STRUCT)
{
	GW_Run_Init(GW_STRUCT, HDLC_STRUCT);

	while(1)
	{
		if(GW_State == WAITING_FOR_SNRM_RESPONSE)						//Waiting for receiving SNRM response (in defined timeout)
		{
			while (Timer <= HDLC_STRUCT->Timeout_ms)
			{
				if (HDLC_STRUCT->RX_Count > 0)							//Received data from meter (based on HDLC) as SNRM response
				{
					printf("SNRM RESPONSE RECEIVED\n");
					Timer = 0;
					GW_State = RECEIVED_SNRM_RESPONSE;
					Handle_GW_Frame_Ret = Handle_GW_Frame (GW_STRUCT, HDLC_STRUCT);		//Converting AARQ frame for sending to meter
					HDLC_STRUCT	->RX_Count = 0;
					GW_STRUCT	->RX_Count = 0;
					break;
				}
				usleep(1000);
				Timer ++;

				if(Timer >= HDLC_STRUCT->Timeout_ms)					//Do not receiving SNRM response in defined timeout
				{
					printf("TIMEOUT - SNRM RESPONSE\n");
					Timer = 0;
					GW_State = READY_TO_GENERATE_SNRM;
					HDLC_STRUCT	->RX_Count = 0;
					GW_STRUCT	->RX_Count = 0;
					break;
				}
			}
		}
		else if(GW_State == WAITING_FOR_RESPONSE)						//Waiting for request response in defined timeout
		{
			while (Timer <= HDLC_STRUCT->Timeout_ms)
			{
				if (HDLC_STRUCT->RX_Count > 0)							//Received data from meter (based on HDLC) as request response
				{
					printf("||RESPONSE RECEIVED||\n");
					Timer = 0;
					GW_State = AARQ_RESPONSE_RECEIVED;
					int64_t GW_size = Meter2GW_Frame_Convertor (HDLC_STRUCT, GW_STRUCT);		//Converting HDLC response frame to GW frame

					if(GW_size > 0 && GW_State == WAITING_FOR_REQUEST)			//Finalizing GW frame size (because of being multi-segment frame)
					{
						GW_STRUCT->TX_Count = GW_size;

						if(Check_GW_Frame_Type(GW_STRUCT->RX_Buffer) == RLRQ_TAG)				//Checking RLRQ frame for generating DISC frame
						{
							uint8_t ret_disc = HDLC_Send_DISC(GW_STRUCT, HDLC_STRUCT)	;		//Generating HDLC DISC frame for sending to meter
							GW_State = WAITING_FOR_DISC_RESPONSE						;
							printf("||DISC FRAME SENT - RET:%d||\n", ret_disc)			;
						}
					}

					HDLC_STRUCT	->RX_Count = 0;
					GW_STRUCT	->RX_Count = 0;
					break;
				}
				usleep(1000);
				Timer ++;

				if(Timer >= HDLC_STRUCT->Timeout_ms)								//Do not receiving request response in defined timeout
				{
					printf("||TIMEOUT RESPONSE||\n");
					Timer = 0;
					GW_State = WAITING_FOR_REQUEST;
					HDLC_STRUCT	->RX_Count = 0;
					GW_STRUCT	->RX_Count = 0;
					break;
				}
			}
		}
		else if(GW_State == RECEIVE_READY_FOR_SB_MODE)				//Generating receive ready frame for receiving remained data in multi-segment frame
		{
			printf("||RECEIVE_READY_FOR_SB_MODE||\n");
			Timer = 0;
			Control_Byte_Struct.Ctrl_Byte = Control_Byte (Control_Byte_Struct.RRR, Control_Byte_Struct.SSS, RECEIVE_READY);
			uint8_t HDLC_Data_Size = GW2HDLC_Poll_For_Remained_Data (GW_STRUCT, HDLC_STRUCT, Control_Byte_Struct.Ctrl_Byte);

			if(HDLC_Data_Size > 0)
			{
				HDLC_STRUCT->TX_Count = HDLC_Data_Size;
			}

			GW_State = WAITING_FOR_RESPONSE;
		}
		else if(GW_State == WAITING_FOR_DISC_RESPONSE)			//Waiting for DISC response in defined timeout
		{
			while (Timer <= HDLC_STRUCT->Timeout_ms)
			{
				if (HDLC_STRUCT->RX_Count > 0)					//Receiving DISC response from meter
				{
					printf("\n---------------------------------------------------------\n");
					printf("DISC RESPONSE - LEN:%d\n", HDLC_STRUCT->RX_Count);
					for(int i=0; i<HDLC_STRUCT->RX_Count; i++)
					{
						printf("0x%x-", HDLC_STRUCT->RX_Buffer[i]);
					}
					printf("\n---------------------------------------------------------\n");

					HDLC_STRUCT->RX_Count = 0;

					GW_State = READY_TO_GENERATE_SNRM;
					Timer = 0;
					break;
				}
				else if (Timer >= HDLC_STRUCT->Timeout_ms)		//Do not receiving DISC response in defined timeout
				{
					GW_State = WAITING_FOR_REQUEST;
					printf("||TIMEOUT DISC RESPONSE||\n");
					Timer = 0;
					break;
				}
				else
				{
					usleep(1000);
					Timer ++;
				}
			}
		}
		else if(GW_STRUCT->RX_Count > 0)		//Receiving frame from MDM
		{
			printf("FRAME (REQUEST) RECEIVED FROM MDM - START CONVERTING DLMS-GW2HDLC - LEN:%d\n", GW_STRUCT->RX_Count);

			Handle_GW_Frame_Ret = Handle_GW_Frame (GW_STRUCT, HDLC_STRUCT);		//Analyzing GW frame received from MDM
			Timer = 0;
		}
	}
}


/***********************************************************************************************
 * Gateway Initialize Function
 ***********************************************************************************************/
void GW_Run_Init(GW_STRUCT_TYPEDEF* GW_STRUCT,HDLC_STRUCT_TYPEDEF* HDLC_STRUCT)							//Initializing some variables
{
	printf("GW_Run_Init\n");
	memset(&Control_Byte_Struct	, 0, sizeof(Control_Byte_Struct));
}


/***********************************************************************************************
 * Handle_GW_Frame	-	Handling data received from MDM
 ***********************************************************************************************/
int8_t Handle_GW_Frame (GW_STRUCT_TYPEDEF* GW_STRUCT, HDLC_STRUCT_TYPEDEF* HDLC_STRUCT)			//Checking validation and type of GW frame received from MDM
{																								// and handling frame for converting in different modes
	static uint8_t 	GW_Frame_cpy[8192]		;
	uint8_t 		HDLC_Data[8152]			;
	uint16_t 		HDLC_Data_Size		=0	;
	uint8_t 		Gateway_Data[8152]		;
	int64_t 		Gateway_Data_Size	=0	;

	printf("\n---------------------------------------------------------\n");
	printf("GW FRAME - RECEVIED FRAME FROM MDM - LEN:%d\n", GW_STRUCT->RX_Count);
	for(int i=0; i<GW_STRUCT->RX_Count; i++)
	{
		printf("0x%x-", GW_STRUCT->RX_Buffer[i]);
	}
	printf("\n---------------------------------------------------------\n");

	memset(GW_Frame_cpy, 0, sizeof(GW_Frame_cpy));
	memcpy(GW_Frame_cpy, GW_STRUCT->RX_Buffer, GW_STRUCT->RX_Count);

	memset(HDLC_Data, 0, sizeof(HDLC_Data));

	memset(Gateway_Data, 0, sizeof(Gateway_Data));

	if(Check_GW_Frame_Valid(GW_STRUCT) == GW_FRAME_IS_VALID)			//Checking validation frame - Version, Header and Network ID
	{
		if(Check_GW_Frame_Type(GW_STRUCT) == AARQ_TAG)					//Checking AARQ tag in received frame
		{
			if(GW_State == READY_TO_GENERATE_SNRM || GW_State == WAITING_FOR_REQUEST)
			{
				printf("||CONNECT REQUEST FROM MDM||\n");

				uint8_t SNRM_generator_ret = GW2HDLC_SNRM_Generator (GW_STRUCT, HDLC_STRUCT);		//Generating SNRM frame

				if(SNRM_generator_ret > 0)
				{
					HDLC_STRUCT->TX_Count = SNRM_generator_ret;
					printf("||SNRM GENERATED FROM GW2HDLC - LEN:%d||\n", HDLC_STRUCT->TX_Count)	;
					GW_State = WAITING_FOR_SNRM_RESPONSE;
					return SNRM_GENERATED;
				}
				else
				{
					printf("||ERROR - SNRM GENERATED FROM DLMS2HDLC - RET:%d||\n", SNRM_generator_ret)	;
					GW_State = READY_TO_GENERATE_SNRM;
					return SNRM_ERROR;
				}
			}
			else if(GW_State == RECEIVED_SNRM_RESPONSE)
			{
				printf("||SNRM RESPONSE RECEIVED FROM METER2GW - LEN:%d||\n", HDLC_STRUCT->RX_Count)		;

				int16_t HDLC_Size = GW2HDLC_Frame_Convertor (GW_STRUCT, HDLC_STRUCT, AARQ_CONNECTION_CNTROLBYTE);		//Generating AARQ frame
				if(HDLC_Size >=0)
				{
					HDLC_STRUCT->TX_Count = HDLC_Size;

					printf("SNRM AARQ DLMS2HDLC - RET:%d\n", HDLC_STRUCT->TX_Count)	;
					GW_State = WAITING_FOR_RESPONSE;
					return AARQ_CONVERTED;
				}
				else
				{
					printf("ERROR - SNRM AARQ DLMS2HDLC - RET:%d\n", HDLC_Size)	;
					GW_State = WAITING_FOR_REQUEST;
					return AARQ_ERROR;
				}
			}
		}
		else		//Check_GW_Frame_Type(GW_Frame_cpy) != AARQ_TAG		-	Information frame
		{
			printf("REQUEST FROM MDM2GW (NOT REQUEST CONNECTION)\n");

			if(GW_State == WAITING_FOR_REQUEST)
			{
				Control_Byte_Struct.Ctrl_Byte = Control_Byte (Control_Byte_Struct.RRR, Control_Byte_Struct.SSS, INFORMATION)	;
				int16_t HDLC_Size  = GW2HDLC_Frame_Convertor (GW_STRUCT, HDLC_STRUCT, Control_Byte_Struct.Ctrl_Byte)			;			//Converting request frame to HDLC frame

				if(HDLC_Size > 0)
				{
					HDLC_STRUCT->TX_Count = HDLC_Size;
					printf("REQUEST CONVERTED FROM DLMS2HDLC - RET:%d\n", HDLC_STRUCT->TX_Count)					;
					GW_State = WAITING_FOR_RESPONSE;
					GW_STRUCT->RX_Count = 0;
					return AARQ_CONVERTED;
				}
				else
				{
					printf("ERROR - REQUEST CONVERTED FROM DLMS2HDLC - RET:%d\n", HDLC_Size)					;
					GW_State = WAITING_FOR_REQUEST;
					GW_STRUCT->RX_Count = 0;
					return AARQ_ERROR;
				}
			}
		}
	}
	else
	{
		return GW_FRAME_VALID_ERROR;
	}
}

/***********************************************************************************************
 * GW2HDLC_Frame_Convertor
 ***********************************************************************************************/
int16_t GW2HDLC_Frame_Convertor (GW_STRUCT_TYPEDEF* GW_STRUCT, HDLC_STRUCT_TYPEDEF* HDLC_STRUCT, uint8_t Control_Byte)		//Converting GW frame to HDLC frame
{
	printf("-----------------------------------------------------------------------\n");
	printf("RECEIVED FRAME FOR CONVERTING FROM GW2HDLC - LEN:%d\n", GW_STRUCT->RX_Count);
	for (int i=0; i< GW_STRUCT->RX_Count; i++)
	{
		printf("0x%x-",GW_STRUCT->RX_Buffer[i]);
	}
	printf("\n");
	printf("-----------------------------------------------------------------------\n");

	uint8_t		APDU[65536]			;
	uint8_t 	Add_Len 		= 0	;
	uint8_t 	APDU_start_add	= 0	;
	uint8_t 	dst_size 		= 0	;
	uint8_t 	APDU_start_byte	= 0	;
	uint16_t	Logical_Address = 1	;
	uint16_t	last_byte 		= 0	;
	uint16_t 	phy_add 		= 0	;
	uint16_t 	HCS 			= 0	;
	uint16_t 	FCS 			= 0	;
	uint16_t 	APDU_size 		= 0	;
	uint16_t 	MAC_frame_Size 	= 0	;
	uint16_t 	dst_add_log 	= 0	;
	uint16_t 	dst_add_phy 	= 0	;
	uint16_t 	Frame_Format	= 0	;

	memset(APDU, 0, sizeof(APDU));

	/* Detecting APDU in GW frame */
	Add_Len 		= GW_STRUCT->RX_Buffer[10]						;									//Reading Physical Address length from 10th array of received format
	APDU_start_add 	= HEADER_PREFIX_MIN_SIZE_GW_PRTCL + Add_Len		;									//Calculating start point to read APDU information
	APDU_size 		= (((uint16_t) (GW_STRUCT->RX_Buffer[6])) << 8) | GW_STRUCT->RX_Buffer[7]	;		//Reading APDU bytes size from received frame
	memcpy(APDU, GW_STRUCT->RX_Buffer + APDU_start_add, APDU_size)	;

	MAC_frame_Size = MIN_HDLC_SIZE;			//Min MAC frame size

	Logical_Address = (((uint16_t) (GW_STRUCT->RX_Buffer[HES_DST_ADD_START_BYTE])) << 8) | ((uint16_t) (GW_STRUCT->RX_Buffer[HES_DST_ADD_START_BYTE + 1]));

	if(Add_Len == 1)						//Max physical address (1-byte): 0b 0111 1111 = 127 => 0b 1111 1111
	{
		dst_size 		= 2;
		MAC_frame_Size += 2;
		phy_add 		= ((uint16_t) (GW_STRUCT->RX_Buffer[PHY_ADD_START_BYTE]));
	}
	else if(Add_Len == 2) 					//Max physical address(2-byte): 0b 0011 1111 1111 1111 = 16383 => 0b 1111 1110 1111 1111
	{
		dst_size = 4;
		MAC_frame_Size += 4;
		phy_add = (((uint16_t) (GW_STRUCT->RX_Buffer[PHY_ADD_START_BYTE])) << 8) | ((uint16_t) (GW_STRUCT->RX_Buffer[PHY_ADD_START_BYTE + 1]));
	}
	else
	{
		printf("GW2HDLC_Frame_Convertor: Error - Physical Address size must be 1 or 2 - it is %d\n", Add_Len);
		return -1;
	}

	MAC_frame_Size = MAC_frame_Size + APDU_size + LLC_SUB_LAYER_SIZE;

	uint8_t MAC_frame[MAC_frame_Size];
	memset(MAC_frame, 0, sizeof(MAC_frame));

	MAC_frame[0] 				= 0x7E;		//Start flag
	MAC_frame[MAC_frame_Size-1] = 0x7E;		//End flag

	Frame_Format 						= ((MAC_frame_Size - 2) & 0x07FF) | (0xA000);
	MAC_frame[FRAME_FRMT_START_BYTE] 	= (uint8_t) (Frame_Format >> 8)				;
	MAC_frame[FRAME_FRMT_START_BYTE+1] 	= (uint8_t) (Frame_Format & 0xFF)			;

	if(0 <= Logical_Address && Logical_Address <= 16383)
	{
		dst_add_log = ((Logical_Address << 2) & (0xFE00)) | ((Logical_Address << 1) & (0x00FE));
	}
	else
	{
		printf("GW2HDLC_Frame_Convertor: Error - Logical Address must be between 0 and 16383.\n");
		return -2;
	}

	dst_add_phy = ((phy_add << 2) & (0xFE00)) | ((phy_add << 1) & (0x00FE)) | 0x1;

	if(dst_size == 4)			//Combining logical address with physical address and turning to source address
	{
		MAC_frame[DST_ADD_START_BYTE] 	= (uint8_t) (dst_add_log >> 8);
		MAC_frame[DST_ADD_START_BYTE+1] = (uint8_t) (dst_add_log & 0xFF);
		MAC_frame[DST_ADD_START_BYTE+2] = (uint8_t) (dst_add_phy >> 8);
		MAC_frame[DST_ADD_START_BYTE+3] = (uint8_t) (dst_add_phy & 0xFF);

		last_byte = DST_ADD_START_BYTE+3;
	}
	else if(dst_size == 2)
	{
		MAC_frame[DST_ADD_START_BYTE] 	= (uint8_t) (dst_add_log & 0xFF);
		MAC_frame[DST_ADD_START_BYTE+1] = (uint8_t) (dst_add_phy & 0xFF);

		last_byte = DST_ADD_START_BYTE+1;
	}

	MAC_frame[last_byte + 1] = ((GW_STRUCT->RX_Buffer[HES_SRC_ADD_START_BYTE+1] << 1) & (0xFE)) | 0x1;			//Source Address

	if(Control_Byte >= 0 && Control_Byte <= 255)
	{
		MAC_frame[last_byte + 2] = Control_Byte;
	}
	else
	{
		printf("GW2HDLC_Frame_Convertor: Error - Physical Address must be between 0 and 255.\n");
		return -3;
	}


	HCS = countCRC(MAC_frame, 1, last_byte+2);
	MAC_frame[last_byte + 3] = (uint8_t) (HCS >> 8);
	MAC_frame[last_byte + 4] = (uint8_t) (HCS & 0x00FF);

	MAC_frame[last_byte + 5] = 0xE6;	//LLC sub-layer
	MAC_frame[last_byte + 6] = 0xE6;
	MAC_frame[last_byte + 7] = 0x00;

	APDU_start_byte = last_byte + MAC_MIN_APDU_START_BYTE;

	for(int k=0; k<APDU_size; k++)
	{
		MAC_frame[APDU_start_byte + k] = APDU[k];
	}

	last_byte = APDU_start_byte + APDU_size - 1;

	FCS = countCRC(&MAC_frame, 1, last_byte);
	MAC_frame[last_byte + 1] = (uint8_t) (FCS >> 8);
	MAC_frame[last_byte + 2] = (uint8_t) (FCS & 0x00FF);

	memcpy(HDLC_STRUCT->TX_Buffer, MAC_frame, sizeof(MAC_frame));
	Gate_Meter_size = MAC_frame_Size;

	Ctrl_Byte_Last = Control_Byte;

	if(Control_Byte_Struct.SSS >= 7)
	{
		Control_Byte_Struct.SSS = 0;
	}
	else
	{
		Control_Byte_Struct.SSS ++;
	}

	printf("-----------------------------------------------------------------------\n");
	printf("PREPARED DLMS FRAME FOR SENDING FROM GW2HDLC - LEN:%d\n", MAC_frame_Size);
	for (int i=0; i<MAC_frame_Size; i++)
	{
		printf("0x%x-",HDLC_STRUCT->TX_Buffer[i]);
	}
	printf("\n");
	printf("-----------------------------------------------------------------------\n");

	return MAC_frame_Size;
}

/***********************************************************************************************
 * Meter2GW_Frame_Convertor
 ***********************************************************************************************/
int64_t Meter2GW_Frame_Convertor (HDLC_STRUCT_TYPEDEF* HDLC_STRUCT, GW_STRUCT_TYPEDEF* GW_STRUCT)		//Converting HDLC frame to GW Frame
{
	printf("--------------------------------------------------------\n");
	printf("Meter2GW_Frame_Convertor:%d\n", HDLC_STRUCT->RX_Count);
	for(int i=0; i<HDLC_STRUCT->RX_Count; i++)
	{
		printf("0x%x-", HDLC_STRUCT->RX_Buffer[i]);
	}
	printf("\n--------------------------------------------------------\n");

	uint8_t		last_byte					= 0;
	uint8_t		last_byte_for_last_segment	= 0;
	uint16_t	Phy_Add						= 0;
	uint16_t	Log_Add						= 0;
	uint16_t	HDLC_rec_data_size			= 0;
	uint16_t	APDU_Len_for_Last_Segment	= MIN_GW_APDU_LEN_SIZE;		//len (header + net id + add len)

	uint8_t 	HES_Frame[5000];
	uint32_t 	HES_Frame_Size=0;
	uint32_t 	HES_Frame_Size_for_Last_Segment=0;

	HES_Frame_Size = HES_MIN_FRAME_SIZE;

	memset(HES_Frame, 0, sizeof(HES_Frame));


	if(HDLC_STRUCT->RX_Buffer[0] == 0x7E )
	{
		if((HDLC_STRUCT->RX_Buffer[FRAME_FRMT_START_BYTE] & 0xF0) == 0xA0 )
		{
			if((HDLC_STRUCT->RX_Buffer[FRAME_FRMT_START_BYTE] & 0x08) == 0x08)
			{
				Segment_bit = MIDDLE_FRAME_IN_SEGMENTATION;
				GW_State = RECEIVE_READY_FOR_SB_MODE;
			}
			else if(Segment_bit == MIDDLE_FRAME_IN_SEGMENTATION)
			{
				Segment_bit = LAST_FRAME_IN_SEGMENTATION;
				GW_State = WAITING_FOR_REQUEST;
			}
			else
			{
				Segment_bit = SINGLE_FRAME_IN_SEGMENTATION;
				memset(Buffer_Data_Meter2GW_Frame_Convertor, 0, sizeof(Buffer_Data_Meter2GW_Frame_Convertor));
				Last_Byte_Buffer_Meter2GW = 0;
				GW_State = WAITING_FOR_REQUEST;
			}

			HES_Frame[VERSION_START_BYTE] 	= 0;
			HES_Frame[VERSION_START_BYTE+1] = 1;				//Version

			HES_Frame[HEADER_BYTE] 			= 0xE7;				//Header
			HES_Frame[NETWORK_ID_BYTE] 		= 0;				//Network ID
			HES_Frame[ADD_LEN_BYTE]			= 0;

			HDLC_rec_data_size = (((((uint16_t) (HDLC_STRUCT->RX_Buffer[FRAME_FRMT_START_BYTE])) << 8) | ((uint16_t) (HDLC_STRUCT->RX_Buffer[FRAME_FRMT_START_BYTE+1]))) & 0x07FF) + 2;

			if(HDLC_STRUCT->RX_Buffer[HDLC_rec_data_size-1] != 0x7E)
			{
				printf("Meter2GW_Frame_Convertor: Error - End flag is wrong. \n");
				return -7;
			}

			for(int i=3; i<HDLC_rec_data_size; i++)				//Finding end of destination address
			{
				if((HDLC_STRUCT->RX_Buffer[i] & 0x1) == 0x1)
				{
					HES_Frame[HES_DST_ADD_START_BYTE+1] = (HDLC_STRUCT->RX_Buffer[i] >> 1);		//Destination Address
					last_byte = i;
					break;
				}
			}

			for(int i=last_byte+1; i<HDLC_rec_data_size; i++)	//Finding end of source address
			{
				if((HDLC_STRUCT->RX_Buffer[i] & 0x1) == 0x1)
				{
					if(i-last_byte == 4)		//Detecting size of source address
					{
						Log_Add = (uint16_t) ((HDLC_STRUCT->RX_Buffer[i-2] >> 1) | ((uint8_t) (HDLC_STRUCT->RX_Buffer[i-3] << 6)));
						Log_Add = Log_Add | ((uint16_t) ((HDLC_STRUCT->RX_Buffer[i-3] & 0xFC)<<6));

						HES_Frame[HES_SRC_ADD_START_BYTE] 	= (uint8_t) ((Log_Add >> 8) & 0x00FF);
						HES_Frame[HES_SRC_ADD_START_BYTE+1] = (uint8_t) ((Log_Add) & 0x00FF);			//Source address

						Phy_Add = (uint16_t) ((HDLC_STRUCT->RX_Buffer[i] >> 1) | ((uint8_t) (HDLC_STRUCT->RX_Buffer[i-1] << 6)));
						Phy_Add = Phy_Add | ((uint16_t) ((HDLC_STRUCT->RX_Buffer[i-1] & 0xFC)<<6));

						uint8_t Phy_Add_len = 0;

						if(Phy_Add>=0 && Phy_Add<256)
						{
							HES_Frame[ADD_LEN_BYTE] 			= 1;
							HES_Frame[HES_PHY_ADD_START_BYTE] 	= (uint8_t) (Phy_Add & 0x00FF);
							APDU_Len_for_Last_Segment ++;
						}
						else if(Phy_Add>=256 && Phy_Add<=65535)
						{
							HES_Frame[ADD_LEN_BYTE] 			= 2;
							HES_Frame[HES_PHY_ADD_START_BYTE] 	= (uint8_t) ((Phy_Add >> 8) & 0x00FF);
							HES_Frame[HES_PHY_ADD_START_BYTE+1] = (uint8_t) (Phy_Add & 0x00FF);
							APDU_Len_for_Last_Segment += 2;
						}
						else
						{
							printf("Meter2GW_Frame_Convertor: Error - Physical address must be between 0 and 65535. - %d\n", Phy_Add);
							return -1;
						}

						Phy_Add_len 	= HES_Frame[ADD_LEN_BYTE];
						HES_Frame_Size 	+= Phy_Add_len;
					}
					else if(i-last_byte == 2)
					{
						Log_Add = ((HDLC_STRUCT->RX_Buffer[i-1] >> 1));
						Phy_Add = (HDLC_STRUCT->RX_Buffer[i] >> 1);

						uint8_t Phy_Add_len = 0;

						if(Phy_Add>=0 && Phy_Add<256)
						{
							HES_Frame[HES_SRC_ADD_START_BYTE+1] = (uint8_t) (Log_Add);
							HES_Frame[ADD_LEN_BYTE] 			= 1;
							HES_Frame[HES_PHY_ADD_START_BYTE] 	= (uint8_t) (Phy_Add & 0x00FF);
							APDU_Len_for_Last_Segment ++;
						}
						else
						{
							printf("Meter2GW_Frame_Convertor: Error - Physical address must be between 0 and 256. - %d\n", Phy_Add);
							return -2;
						}

						Phy_Add_len = HES_Frame[ADD_LEN_BYTE];
						HES_Frame_Size += Phy_Add_len;

					}
					else
					{
						printf("Meter2GW_Frame_Convertor: Error - Source Address is wrong. \n");
						return -3;
					}
					uint16_t HES_APDU_Byte = ADD_LEN_BYTE + HES_Frame[ADD_LEN_BYTE] + 1;
					uint16_t HES_APDU_Byte_for_Last_segment = HES_APDU_Byte;

					last_byte = i;

					if(Segment_bit == SINGLE_FRAME_IN_SEGMENTATION)
					{
						Ctrl_Byte_Last = HDLC_STRUCT->RX_Buffer[last_byte+1];
					}

					uint16_t HCS_Frame 				= 0	;
					uint16_t HCS_Cal 				= 0	;
					HCS_Frame 						= ((uint16_t) (HDLC_STRUCT->RX_Buffer[last_byte+2]) << 8) | (HDLC_STRUCT->RX_Buffer[last_byte+3]);
					HCS_Cal 						= countCRC(HDLC_STRUCT->RX_Buffer, 1, last_byte + 1);
					last_byte 						= last_byte +3; 						//len(CRC)+len(ControlByte)
					last_byte_for_last_segment 		= last_byte;

					uint16_t APDU_len 				= 0;
					int32_t APDU_len_real 			= 0;
					APDU_len 						= HDLC_rec_data_size - last_byte -4 + (3 + HES_Frame[ADD_LEN_BYTE]);		//4 = len(FCS) + len(end flag) + (index is less in value one) / last (prefix len) due to gurux application
					APDU_len_real 					= HDLC_rec_data_size - last_byte -4;										//4 = len(FCS) + len(end flag) + (index is less in value one) / last (prefix len) due to gurux application
					HES_Frame_Size_for_Last_Segment = HES_Frame_Size;
					HES_Frame_Size					+= APDU_len_real;

					if(APDU_len_real < 0)
					{
						printf("Meter2GW_Frame_Convertor: NO INFORMATION \n");
						return -4;
					}

					HES_Frame[APDU_LEN_START_BYTE] 			= (uint8_t) (APDU_len >> 8);
					HES_Frame[APDU_LEN_START_BYTE+1] 		= (uint8_t) (APDU_len & 0x00FF);


					if(HCS_Frame == HCS_Cal)
					{
						for(int j = last_byte+1; j < HDLC_rec_data_size-3; j++)
						{
							Buffer_Data_Meter2GW_Frame_Convertor[Last_Byte_Buffer_Meter2GW] = HDLC_STRUCT->RX_Buffer[j];
							Last_Byte_Buffer_Meter2GW ++;

							HES_Frame[HES_APDU_Byte] = HDLC_STRUCT->RX_Buffer[j];
							HES_APDU_Byte ++;
							last_byte ++;
						}

						uint16_t FCS_Frame 	= 0;
						uint16_t FCS_Cal 	= 0;
						FCS_Frame 	= ((uint16_t) (HDLC_STRUCT->RX_Buffer[last_byte + 1]) << 8) | (HDLC_STRUCT->RX_Buffer[last_byte + 2]);
						FCS_Cal 	= countCRC(HDLC_STRUCT->RX_Buffer, 1, last_byte);

						if(FCS_Frame == FCS_Cal)
						{
							Gate_HES_size = HES_Frame_Size;
							memcpy(GW_STRUCT->TX_Buffer, HES_Frame, sizeof(HES_Frame));

							if(Control_Byte_Struct.RRR >= 7)
							{
								Control_Byte_Struct.RRR = 0;
							}
							else
							{
								Control_Byte_Struct.RRR ++;
							}

							if(Segment_bit == LAST_FRAME_IN_SEGMENTATION)
							{
								HES_Frame_Size_for_Last_Segment	+= Last_Byte_Buffer_Meter2GW;

								for(int n=0; n<Last_Byte_Buffer_Meter2GW; n++)
								{
									HES_Frame[HES_APDU_Byte_for_Last_segment] = Buffer_Data_Meter2GW_Frame_Convertor[n];
									HES_APDU_Byte_for_Last_segment ++;
									last_byte_for_last_segment ++;
								}

								APDU_Len_for_Last_Segment = HES_Frame_Size_for_Last_Segment - 8;
								HES_Frame[APDU_LEN_START_BYTE] 			= (uint8_t) (APDU_Len_for_Last_Segment >> 8);
								HES_Frame[APDU_LEN_START_BYTE+1] 		= (uint8_t) (APDU_Len_for_Last_Segment & 0x00FF);

								memcpy(GW_STRUCT->TX_Buffer, HES_Frame, sizeof(HES_Frame));

								printf("--------------------------------------------------------\n");
								printf("Meter2GW_Frame_Convertor:%d\n", Last_Byte_Buffer_Meter2GW);
								for(int i=0; i<Last_Byte_Buffer_Meter2GW+1; i++)
								{
									printf("0x%x-", GW_STRUCT->TX_Buffer[i]);
								}
								printf("\n--------------------------------------------------------\n");

								memset(Buffer_Data_Meter2GW_Frame_Convertor, 0, sizeof(Buffer_Data_Meter2GW_Frame_Convertor));
								Last_Byte_Buffer_Meter2GW = 0;

								return HES_Frame_Size_for_Last_Segment;
							}
							else if(Segment_bit == SINGLE_FRAME_IN_SEGMENTATION)
							{
								memset(Buffer_Data_Meter2GW_Frame_Convertor, 0, sizeof(Buffer_Data_Meter2GW_Frame_Convertor));
								Last_Byte_Buffer_Meter2GW = 0;
							}

							return HES_Frame_Size;
						}
						else
						{
							printf("Meter2GW_Frame_Convertor: Error - FCS is wrong. \n");
							return -5;
						}
					}
					else
					{
						printf("Meter2GW_Frame_Convertor: Error - HCS is wrong. \n");
						return -6;
					}

					break;
				}
			}
		}
		else
		{
			printf("Meter2GW_Frame_Convertor: Error - Frame format field is wrong.-0x%x \n", HDLC_STRUCT->RX_Buffer[1]);
			return -8;
		}
	}
	else
	{
		printf("Start flag is wrong. \n");
		return -9;
	}
	return 0;
}

/***********************************************************************************************
 * GW2HDLC_SNRM_Generator
 ***********************************************************************************************/
uint8_t GW2HDLC_SNRM_Generator (GW_STRUCT_TYPEDEF* GW_STRUCT, HDLC_STRUCT_TYPEDEF* HDLC_STRUCT)		//Generating SNRM frame (HDLC frame)
{
	printf("-----------------------------------------------------------------------\n");
	printf("RECEIVED FRAME FOR GENERATING SNRM - LEN:%d\n", GW_STRUCT->RX_Count);
	for (int i=0; i < GW_STRUCT->RX_Count; i++)
	{
		printf("0x%x-",GW_STRUCT->RX_Buffer[i]);
	}
	printf("\n");
	printf("-----------------------------------------------------------------------\n");

	uint8_t		Control_Byte	= SNRM_CONTROL_BYTE;
	uint8_t		APDU[65536]			;
	uint8_t 	Add_Len 		= 0	;
	uint8_t		last_byte 		= 0	;
	uint8_t 	APDU_start_add	= 0	;
	uint8_t 	dst_size 		= 0	;
	uint16_t	Logical_Address = 1	;
	uint16_t 	phy_add 		= 0	;
	uint16_t 	HCS 			= 0	;
	uint16_t 	APDU_size 		= 0	;
	uint16_t 	MAC_frame_Size 	= 0	;
	uint16_t 	dst_add_log 	= 0	;
	uint16_t 	dst_add_phy 	= 0	;
	uint16_t 	Frame_Format	= 0	;

	memset(APDU, 0, sizeof(APDU));

	/* Detecting APDU in GW frame */
	Add_Len 		= GW_STRUCT->RX_Buffer[ADD_LEN_BYTE];																			//Reading Physical Address length from 10th array of received format
	APDU_start_add 	= HEADER_PREFIX_MIN_SIZE_GW_PRTCL + Add_Len;															//Calculating start point to read APDU information
	APDU_size 		= (((uint16_t) (GW_STRUCT->RX_Buffer[APDU_LEN_START_BYTE])) << 8) | GW_STRUCT->RX_Buffer[APDU_LEN_START_BYTE+1];		//Reading APDU bytes size from received frame
	memcpy(APDU, GW_STRUCT->RX_Buffer + APDU_start_add, APDU_size);


	MAC_frame_Size = MIN_SNRM_SIZE;												//Min MAC frame size

	Logical_Address = (((uint16_t) (GW_STRUCT->RX_Buffer[HES_DST_ADD_START_BYTE])) << 8) | ((uint16_t) (GW_STRUCT->RX_Buffer[HES_DST_ADD_START_BYTE + 1]));

	if(Add_Len == 2) 					//Max physical address(2-byte): 0b 0011 1111 1111 1111 = 16383 => 0b 1111 1110 1111 1111
	{
		dst_size 		= 4;
		MAC_frame_Size += 4;
		phy_add = (((uint16_t) (GW_STRUCT->RX_Buffer[PHY_ADD_START_BYTE])) << 8) | ((uint16_t) (GW_STRUCT->RX_Buffer[PHY_ADD_START_BYTE + 1]));
	}
	else if(Add_Len == 1)						//Max physical address (1-byte): 0b 0111 1111 = 127 => 0b 1111 1111
	{
		dst_size 		= 2;
		MAC_frame_Size += 2;
		phy_add 		= ((uint16_t) (GW_STRUCT->RX_Buffer[PHY_ADD_START_BYTE]));
	}
	else
	{
		printf("GW2HDLC_SNRM_Generator: Error - Physical Address size must be 1 or 2 - it is %d\n", Add_Len);
		return -1;
	}

	uint8_t MAC_frame[MAC_frame_Size];
	memset(MAC_frame, 0, sizeof(MAC_frame));

	MAC_frame[START_FLAG_BYTE] 	= 0x7E;		//Start flag
	MAC_frame[MAC_frame_Size-1] = 0x7E;		//End flag

	Frame_Format = ((MAC_frame_Size - 2) & 0x07FF) | (0xA000)	;
	MAC_frame[FRAME_FRMT_START_BYTE] 	= (uint8_t) (Frame_Format >> 8)				;
	MAC_frame[FRAME_FRMT_START_BYTE+1] 	= (uint8_t) (Frame_Format & 0xFF)			;

	dst_add_log = ((Logical_Address << 2) & (0xFE00)) | ((Logical_Address << 1) & (0x00FE));
	dst_add_phy = ((phy_add << 2) & (0xFE00)) | ((phy_add << 1) & (0x00FE)) | 0x1;

	if(dst_size == 4)			//Combining logical address with physical address and turning to source address
	{
		MAC_frame[DST_ADD_START_BYTE] 	= (uint8_t) (dst_add_log >> 8);
		MAC_frame[DST_ADD_START_BYTE+1] = (uint8_t) (dst_add_log & 0xFF);
		MAC_frame[DST_ADD_START_BYTE+2] = (uint8_t) (dst_add_phy >> 8);
		MAC_frame[DST_ADD_START_BYTE+3] = (uint8_t) (dst_add_phy & 0xFF);

		last_byte = DST_ADD_START_BYTE+3;
	}
	else if(dst_size == 2)
	{
		MAC_frame[DST_ADD_START_BYTE] 	= (uint8_t) (dst_add_log & 0xFF);
		MAC_frame[DST_ADD_START_BYTE+1] = (uint8_t) (dst_add_phy & 0xFF);

		last_byte = DST_ADD_START_BYTE+1;
	}

	MAC_frame[last_byte + 1] = ((GW_STRUCT->RX_Buffer[HES_SRC_ADD_START_BYTE+1] << 1) & (0xFE)) | 0x1;			//Source Address

	MAC_frame[last_byte + 2] = Control_Byte;

	HCS = countCRC(MAC_frame, 1, last_byte+2);
	MAC_frame[last_byte + 3] = (uint8_t) (HCS >> 8);
	MAC_frame[last_byte + 4] = (uint8_t) (HCS & 0x00FF);

	last_byte += 5;

	memcpy(HDLC_STRUCT->TX_Buffer, MAC_frame, last_byte+1);

	GW_State = WAITING_FOR_SNRM_RESPONSE;

	printf("-----------------------------------------------------------------------\n");
	printf("GENERATED SNRM (HDLC FRAME) FRAME FOR SENDING GW2HDLC - LEN:%d\n", last_byte+1);
	for (int i=0; i<last_byte+1; i++)
	{
		printf("0x%x-",HDLC_STRUCT->TX_Buffer[i]);
	}
	printf("\n");
	printf("-----------------------------------------------------------------------\n");

	return (last_byte+1);
}

/***********************************************************************************************
 * Check_GW_Frame_Type
 ***********************************************************************************************/
uint8_t Check_GW_Frame_Type (GW_STRUCT_TYPEDEF* GW_STRUCT)				//Checking first APDU byte in GW frame
{
	return GW_STRUCT->RX_Buffer[HES_PHY_ADD_START_BYTE + GW_STRUCT->RX_Buffer[ADD_LEN_BYTE]];
}

/***********************************************************************************************
 * HDLC_Send_SNRM
 ***********************************************************************************************/
uint16_t HDLC_Send_SNRM (GW_STRUCT_TYPEDEF* GW_STRUCT, HDLC_STRUCT_TYPEDEF* HDLC_STRUCT)
{
	HDLC_STRUCT->TX_Count = GW2HDLC_SNRM_Generator (GW_STRUCT, HDLC_STRUCT);

	return HDLC_STRUCT->RX_Count;
}

/***********************************************************************************************
 * GW2HDLC_Poll_For_Remained_Data Function
 ***********************************************************************************************/
uint8_t GW2HDLC_Poll_For_Remained_Data (GW_STRUCT_TYPEDEF* GW_STRUCT, HDLC_STRUCT_TYPEDEF* HDLC_STRUCT, uint8_t Control_Byte)		//Generating receive ready frame
{
	uint8_t		APDU[65536]			;
	uint8_t 	Add_Len 		= 0	;
	uint8_t		last_byte 		= 0	;
	uint8_t 	APDU_start_add	= 0	;
	uint8_t 	dst_size 		= 0	;
	uint16_t	Logical_Address = 1	;
	uint16_t 	phy_add 		= 0	;
	uint16_t 	HCS 			= 0	;
	uint16_t 	APDU_size 		= 0	;
	uint16_t 	MAC_frame_Size 	= 0	;
	uint16_t 	dst_add_log 	= 0	;
	uint16_t 	dst_add_phy 	= 0	;
	uint16_t 	Frame_Format	= 0	;

	memset(APDU, 0, sizeof(APDU));

	/* Detecting APDU in GW frame */
	Add_Len 		= GW_STRUCT->RX_Buffer[10];											//Reading Physical Address length from 10th array of received format
	APDU_start_add 	= HEADER_PREFIX_MIN_SIZE_GW_PRTCL + Add_Len;						//Calculating start point to read APDU information
	APDU_size 		= (((uint16_t) (GW_STRUCT->RX_Buffer[6])) << 8) | GW_STRUCT->RX_Buffer[7];		//Reading APDU bytes size from received frame
	memcpy(APDU, GW_STRUCT->RX_Buffer + APDU_start_add, APDU_size);

	MAC_frame_Size = MIN_SNRM_SIZE;												//Min MAC frame size

	Logical_Address = (((uint16_t) (GW_STRUCT->RX_Buffer[HES_DST_ADD_START_BYTE])) << 8) | ((uint16_t) (GW_STRUCT->RX_Buffer[HES_DST_ADD_START_BYTE + 1]));

	if(Add_Len == 1)						//Max physical address (1-byte): 0b 0111 1111 = 127 => 0b 1111 1111
	{
		dst_size 		= 2;
		MAC_frame_Size += 2;
		phy_add 		= ((uint16_t) (GW_STRUCT->RX_Buffer[PHY_ADD_START_BYTE]));
	}
	else if(Add_Len == 2) 					//Max physical address(2-byte): 0b 0011 1111 1111 1111 = 16383 => 0b 1111 1110 1111 1111
	{
		dst_size 		= 4;
		MAC_frame_Size += 4;
		phy_add = (((uint16_t) (GW_STRUCT->RX_Buffer[PHY_ADD_START_BYTE])) << 8) | ((uint16_t) (GW_STRUCT->RX_Buffer[PHY_ADD_START_BYTE + 1]));
	}
	else
	{
		printf("GW2HDLC_Poll_For_Remained_Data: Error - Physical Address size must be 1 or 2\n");
		return -1;
	}

	uint8_t MAC_frame[MAC_frame_Size];
	memset(MAC_frame, 0, sizeof(MAC_frame));

	MAC_frame[0] 				= 0x7E;		//Start flag
	MAC_frame[MAC_frame_Size-1] = 0x7E;		//End flag

	Frame_Format = ((MAC_frame_Size - 2) & 0x07FF) | (0xA000)	;
	MAC_frame[1] = (uint8_t) (Frame_Format >> 8)				;
	MAC_frame[2] = (uint8_t) (Frame_Format & 0xFF)				;

	dst_add_log = ((Logical_Address << 2) & (0xFE00)) | ((Logical_Address << 1) & (0x00FE));

	dst_add_phy = ((phy_add << 2) & (0xFE00)) | ((phy_add << 1) & (0x00FE)) | 0x1;

	if(dst_size == 4)			//Combining logical address with physical address and turning to source address
	{
		MAC_frame[DST_ADD_START_BYTE] 	= (uint8_t) (dst_add_log >> 8);
		MAC_frame[DST_ADD_START_BYTE+1] = (uint8_t) (dst_add_log & 0xFF);
		MAC_frame[DST_ADD_START_BYTE+2] = (uint8_t) (dst_add_phy >> 8);
		MAC_frame[DST_ADD_START_BYTE+3] = (uint8_t) (dst_add_phy & 0xFF);

		last_byte = DST_ADD_START_BYTE+3;
	}
	else if(dst_size == 2)
	{
		MAC_frame[DST_ADD_START_BYTE] 	= (uint8_t) (dst_add_log & 0xFF);
		MAC_frame[DST_ADD_START_BYTE+1] = (uint8_t) (dst_add_phy & 0xFF);

		last_byte = DST_ADD_START_BYTE+1;
	}

	MAC_frame[last_byte + 1] = ((GW_STRUCT->RX_Buffer[HES_SRC_ADD_START_BYTE+1] << 1) & (0xFE)) | 0x1;			//Source Address

	MAC_frame[last_byte + 2] = Control_Byte;

	HCS = countCRC(MAC_frame, 1, last_byte+2);
	MAC_frame[last_byte + 3] = (uint8_t) (HCS >> 8);
	MAC_frame[last_byte + 4] = (uint8_t) (HCS & 0x00FF);

	last_byte += 5;

	memcpy(HDLC_STRUCT->TX_Buffer, MAC_frame, last_byte+1);

	printf("\n-----------------------------------------------------------------\n");
	printf("||PREPARED RECEIVE READY FRAME FOR SENDING FROM GW2DLMS - LEN:%d||\n", last_byte+1);
	for(int v=0; v<last_byte+1; v++)
	{
		printf("0x%x-", HDLC_STRUCT->TX_Buffer[v]);
	}
	printf("\n-----------------------------------------------------------------\n");

	return (last_byte+1);
}

/***********************************************************************************************
 * GW2HDLC_DISC_Generator
 ***********************************************************************************************/
int8_t GW2HDLC_DISC_Generator (GW_STRUCT_TYPEDEF* GW_STRUCT, HDLC_STRUCT_TYPEDEF* HDLC_STRUCT)		//Generating DISC frame
{
	uint8_t		Control_Byte	= 0x53;
	uint8_t		Logical_Address = 1	;
	uint8_t		APDU[65536]			;
	uint8_t 	Add_Len 		= 0	;
	uint8_t		last_byte 		= 0	;
	uint8_t 	APDU_start_add	= 0	;
	uint8_t 	dst_size 		= 0	;
	uint16_t 	phy_add 		= 0	;
	uint16_t 	HCS 			= 0	;
	uint16_t 	APDU_size 		= 0	;
	uint16_t 	MAC_frame_Size 	= 0	;
	uint16_t 	dst_add_log 	= 0	;
	uint16_t 	dst_add_phy 	= 0	;
	uint16_t 	Frame_Format	= 0	;

	printf("\n-----------------------------------------------------------------\n");
	printf("||RECEIVED FRAM FOR GENERATING DISC FRAME - LEN:%d||\n", GW_STRUCT->RX_Count);
	for(int v=0; v<last_byte+1; v++)
	{
		printf("0x%x-", GW_STRUCT->RX_Buffer[v]);
	}
	printf("\n-----------------------------------------------------------------\n");


	memset(APDU, 0, sizeof(APDU));

	/* Detecting APDU in GW frame */
	Add_Len 		= GW_STRUCT->RX_Buffer[10];														//Reading Physical Address length from 10th array of received format
	APDU_start_add 	= HEADER_PREFIX_MIN_SIZE_GW_PRTCL + Add_Len;									//Calculating start point to read APDU information
	APDU_size 		= (((uint16_t) (GW_STRUCT->RX_Buffer[6])) << 8) | GW_STRUCT->RX_Buffer[7];		//Reading APDU bytes size from received frame
	memcpy(APDU, GW_STRUCT->RX_Buffer + APDU_start_add, APDU_size);


	MAC_frame_Size = MIN_SNRM_SIZE;												//Min MAC frame size

	Logical_Address = (((uint16_t) (GW_STRUCT->RX_Buffer[HES_DST_ADD_START_BYTE])) << 8) | ((uint16_t) (GW_STRUCT->RX_Buffer[HES_DST_ADD_START_BYTE + 1]));

	if(Add_Len == 1)						//Max physical address (1-byte): 0b 0111 1111 = 127 => 0b 1111 1111
	{
		dst_size 		= 2;
		MAC_frame_Size += 2;
		phy_add 		= ((uint16_t) (GW_STRUCT->RX_Buffer[PHY_ADD_START_BYTE]));
	}
	else if(Add_Len == 2) 					//Max physical address(2-byte): 0b 0011 1111 1111 1111 = 16383 => 0b 1111 1110 1111 1111
	{
		dst_size 		= 4;
		MAC_frame_Size += 4;
		phy_add = (((uint16_t) (GW_STRUCT->RX_Buffer[PHY_ADD_START_BYTE])) << 8) | ((uint16_t) (GW_STRUCT->RX_Buffer[PHY_ADD_START_BYTE + 1]));
	}
	else
	{
		printf("GW2HDLC_DISC_Generator: Error - Physical Address size must be 1 or 2\n");
		return -1;
	}

	uint8_t MAC_frame[MAC_frame_Size];
	memset(MAC_frame, 0, sizeof(MAC_frame));

	MAC_frame[0] 				= 0x7E;		//Start flag
	MAC_frame[MAC_frame_Size-1] = 0x7E;		//End flag

	Frame_Format = ((MAC_frame_Size - 2) & 0x07FF) | (0xA000)	;
	MAC_frame[1] = (uint8_t) (Frame_Format >> 8)				;
	MAC_frame[2] = (uint8_t) (Frame_Format & 0xFF)				;

	dst_add_log = ((Logical_Address << 2) & (0xFE00)) | ((Logical_Address << 1) & (0x00FE));

	dst_add_phy = ((phy_add << 2) & (0xFE00)) | ((phy_add << 1) & (0x00FE)) | 0x1;

	if(dst_size == 4)			//Combining logical address with physical address and turning to source address
	{
		MAC_frame[DST_ADD_START_BYTE] 	= (uint8_t) (dst_add_log >> 8);
		MAC_frame[DST_ADD_START_BYTE+1] = (uint8_t) (dst_add_log & 0xFF);
		MAC_frame[DST_ADD_START_BYTE+2] = (uint8_t) (dst_add_phy >> 8);
		MAC_frame[DST_ADD_START_BYTE+3] = (uint8_t) (dst_add_phy & 0xFF);

		last_byte = DST_ADD_START_BYTE+3;
	}
	else if(dst_size == 2)
	{
		MAC_frame[DST_ADD_START_BYTE] 	= (uint8_t) (dst_add_log & 0xFF);
		MAC_frame[DST_ADD_START_BYTE+1] = (uint8_t) (dst_add_phy & 0xFF);

		last_byte = DST_ADD_START_BYTE+1;
	}

	MAC_frame[last_byte + 1] = ((GW_STRUCT->RX_Buffer[HES_SRC_ADD_START_BYTE+1] << 1) & (0xFE)) | 0x1;			//Source Address

	MAC_frame[last_byte + 2] = Control_Byte;

	HCS = countCRC(MAC_frame, 1, last_byte+2);
	MAC_frame[last_byte + 3] = (uint8_t) (HCS >> 8);
	MAC_frame[last_byte + 4] = (uint8_t) (HCS & 0x00FF);

	last_byte += 5;

	memcpy(HDLC_STRUCT->TX_Buffer, MAC_frame, last_byte+1);

	printf("\n-----------------------------------------------------------------\n");
	printf("||PREPARED DISC FRAME FOR SENDING GW2HDLC - LEN:%d||\n", last_byte+1);
	for(int v=0; v<last_byte+1; v++)
	{
		printf("0x%x-", HDLC_STRUCT->TX_Buffer[v]);
	}
	printf("\n-----------------------------------------------------------------\n");

	return (last_byte+1);
}

/***********************************************************************************************
 * HDLC_Send_DISC
 ***********************************************************************************************/
uint8_t HDLC_Send_DISC (GW_STRUCT_TYPEDEF* GW_STRUCT, HDLC_STRUCT_TYPEDEF* HDLC_STRUCT)		//Generating DISC frame and set 0 for control byte
{
	int8_t DISC_Gen_Ret = GW2HDLC_DISC_Generator (GW_STRUCT, HDLC_STRUCT);

	if(DISC_Gen_Ret>0)
	{
		HDLC_STRUCT->TX_Count 	= DISC_Gen_Ret;
		Control_Byte_Struct.RRR = 0;
		Control_Byte_Struct.SSS = 0;
	}

	return DISC_Gen_Ret;
}

/***********************************************************************************************
 * Control_Byte
 ***********************************************************************************************/
uint8_t Control_Byte (uint8_t RRR, uint8_t SSS, FRAME_TYPE frame_type)		//Managing control byte in information frame and receive ready frame
{
	uint8_t CB = 0;

	if(frame_type == INFORMATION)
	{
		CB = (((RRR << 5) & 0xE0) | ((SSS << 1) & 0x0E)) | 0x10;
	}
	else if(frame_type == RECEIVE_READY)
	{
		CB = ((RRR << 5) & 0xE0) | (0x11);
	}

	printf("|| CONTROL BYTE = 0x%x - RRR = 0x%x - SSS = 0x%x - FRMAE TYPE = %d ||\n", CB, RRR, SSS, frame_type);

	return CB;
}

/***********************************************************************************************
 * GW2HDLC_Frame_Convertor
 ***********************************************************************************************/
int8_t Check_GW_Frame_Valid (GW_STRUCT_TYPEDEF* GW_STRUCT)		//Checking version, header and network ID in GW frame
{
	if(GW_STRUCT->RX_Buffer[VERSION_START_BYTE] == 0 && GW_STRUCT->RX_Buffer[VERSION_START_BYTE+1] == 1) 			//Checking version 0x0001
	{
		if(GW_STRUCT->RX_Buffer[HEADER_BYTE] == 0xE6)														//Checking header - must be 0xE6
		{
			if(GW_STRUCT->RX_Buffer[NETWORK_ID_BYTE] == 0x00)												//Checking Network ID - shall be 0x00
			{
				printf("Check_GW_Frame_Valid: GW Frame is Valid\n");
				return GW_FRAME_IS_VALID;
			}
			else
			{
				printf("Check_GW_Frame_Valid: Error - Network ID must be 0x00.\n");
				return GW_FRAME_NETWORK_ID_ERROR;
			}
		}
		else
		{
			printf("Check_GW_Frame_Valid: Error - Header must be 0xE6.\n");
			return GW_FRAME_HEADER_ERROR;
		}
	}
	else
	{
		printf("Check_GW_Frame_Valid: Error - Version must be 0x0001.\n");
		return GW_FRAME_VERSION_ERROR;
	}
}

/***********************************************************************************************
 * countCRC
 ***********************************************************************************************/
static uint16_t countCRC(char* Buff, uint32_t index, uint32_t count)
{
   uint16_t tmp;
   uint16_t FCS16 = 0xFFFF;
   uint16_t pos;
   for (pos = 0; pos < count; ++pos)
   {

	   FCS16 = (FCS16 >> 8) ^ FCS16Table[(FCS16 ^ ((unsigned char*)Buff)[index + pos]) & 0xFF];
   }
   FCS16 = ~FCS16;
   //CRC is in big endian byte order.
   tmp = FCS16;
   FCS16 = tmp >> 8;
   FCS16 |= tmp << 8;
   return FCS16;
}


