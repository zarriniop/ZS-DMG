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
uint8_t					GW_State_tmp			= 1000;
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
uint8_t APDU_tmp_buffer[8152] = {0}					;
uint32_t APDU_tmp_buffer_index 	= 0					;	//keep last data index which is wrote to the APDU_tmp_buffer

void Print_GW_State (void)
{
	if(GW_State != GW_State_tmp)
	{
		printf("-------->>>>>>>>> GW State:%d <<<<<<<<<-------\n", GW_State);
		GW_State_tmp = GW_State;
	}
}
/***********************************************************************************************
 * Gateway Runnig Function - Managing and converting GW2HDLC and HDLC2GW
 ***********************************************************************************************/
void GW_Run (Buffer* GW_STRUCT, Buffer* HDLC_STRUCT)
{
	CTRL_BYTE_STR_TD Control_Byte_Struct;
	uint8_t	Frame_Type;
	uint8_t	RLRQ_flag;
	int ret;
	Buffer GW_tmp, HDLC_tmp;
	struct timeval timeout_start;
	GW_State = WAIT_FOR_GET_FRAME;

	while(1)
	{
//		Print_GW_State();
		switch(GW_State)
		{

			case WAIT_FOR_GET_FRAME:

				if(GW_STRUCT->RX_Count > 0)
				{
					memset(GW_tmp.RX, 0, 2048);
					memcpy(GW_tmp.RX, GW_STRUCT->RX, GW_STRUCT->RX_Count);
					GW_tmp.RX_Count = GW_STRUCT->RX_Count;

					HDLC_tmp.Timeout_ms = HDLC_STRUCT->Timeout_ms;

					GW_STRUCT->RX_Count = 0;

					Frame_Type = Check_GW_Frame_Type(&GW_tmp);

					RLRQ_flag = 0;

					if		(Frame_Type == AARQ_TAG)
						GW_State = SNRM_REQUEST;

					else if	(Frame_Type == RLRQ_TAG)
					{
						RLRQ_flag = 1;
						GW_State = INFORMATION_FRAME;
					}

					else
						GW_State = INFORMATION_FRAME;

					printf("GW_State:%d\n", GW_State);
				}

				break;

			case SNRM_REQUEST:
				printf("SNRM GENERATE\n");
				HDLC_STRUCT->RX_Count = 0;
				ret = GW2HDLC_SNRM_Generator (&GW_tmp, HDLC_STRUCT);

				printf("HDLC_STRUCT:%d\n", HDLC_STRUCT->TX_Count);

				if (ret == 1)
				{
					Control_Byte_Struct.Ctrl_Byte 	= 0;
					Control_Byte_Struct.RRR 		= 0;
					Control_Byte_Struct.SSS 		= 0;

					gettimeofday(&timeout_start, NULL);
					GW_State = SNRM_RESPONSE;
				}
				else
					GW_State = WAIT_FOR_GET_FRAME;

				sleep(1);	//sleep for Afzar Azma meter before sending AARQ and waiting for receiving full SNRM request

				break;

			case SNRM_RESPONSE:

				if(diff_time_ms(&timeout_start) > HDLC_tmp.Timeout_ms)
				{
					report(GATEWAY, RX, "TIME OUT IN SNRM RESPONSE");
					GW_State = WAIT_FOR_GET_FRAME;
				}
				if((HDLC_STRUCT->RX_Count > 0) && (HDLC_STRUCT->RX[HDLC_STRUCT->RX_Count - 1] == FLAG_VALUE_IN_HDLC_FRAME))  //We ensure that we received whole UA frame from meter
				{
					printf("<---- WHOLE SNRM UA RECEIVED ---->\n");
					HDLC_STRUCT->RX_Count = 0;
					GW_State = INFORMATION_FRAME;
				}

				break;

			case INFORMATION_FRAME:

				HDLC_STRUCT->RX_Count = 0;
				Control_Byte_Struct.Ctrl_Byte = Control_Byte (Control_Byte_Struct.RRR, Control_Byte_Struct.SSS, INFORMATION);
				ret = GW2HDLC_Frame_Convertor(&GW_tmp, HDLC_STRUCT, &Control_Byte_Struct);

				if(ret > 0)
				{
					gettimeofday(&timeout_start, NULL);
					GW_State = RESPONSE;
				}
				else
				{
					report(GATEWAY, RX, "ERROR in INFORMATION FRAME");
					GW_State = WAIT_FOR_GET_FRAME;
				}

				break;

			case RESPONSE:

				if(diff_time_ms(&timeout_start) > HDLC_tmp.Timeout_ms)
				{
					report(GATEWAY, RX, "TIME OUT IN RESPONSE");
					GW_State = WAIT_FOR_GET_FRAME;
				}

				if(HDLC_STRUCT->RX_Count > 0)
				{
					memset(HDLC_tmp.RX, 0, sizeof(HDLC_tmp.RX));
					memcpy(HDLC_tmp.RX, HDLC_STRUCT->RX, HDLC_STRUCT->RX_Count);
					HDLC_tmp.RX_Count = HDLC_STRUCT->RX_Count;

					HDLC_STRUCT->RX_Count = 0;

					if	(RLRQ_flag == 1)
						GW_State = DISC_REQUEST;

					else
						GW_State = RESPONSE_FOR_MDM;

				}

				break;

			case SEGMENT:

				HDLC_STRUCT->RX_Count = 0;
				Control_Byte_Struct.Ctrl_Byte = Control_Byte (Control_Byte_Struct.RRR, Control_Byte_Struct.SSS, RECEIVE_READY);
				ret = GW2HDLC_Poll_For_Remained_Data (&GW_tmp, HDLC_STRUCT, Control_Byte_Struct.Ctrl_Byte);
				GW_State = RESPONSE;

				break;

			case RESPONSE_FOR_MDM:

				ret = Meter2GW_Frame_Convertor(&HDLC_tmp, GW_STRUCT, &Control_Byte_Struct);
//				printf("ret : %d , GW_State: %d \n", ret, GW_State);

				if(ret == 1)
					GW_State = SEGMENT;

				else
					GW_State = WAIT_FOR_GET_FRAME;

				break;

			case DISC_REQUEST:
				HDLC_STRUCT->RX_Count = 0;
				ret = GW2HDLC_DISC_Generator(&GW_tmp, HDLC_STRUCT)	;

				if(ret == 1)
				{
					gettimeofday(&timeout_start, NULL);
					GW_State = DISC_RESPONSE;
				}
				else
					GW_State = RESPONSE_FOR_MDM;

				break;

			case DISC_RESPONSE:

				if(diff_time_ms(&timeout_start) > HDLC_tmp.Timeout_ms)
				{
					printf("TIME OUT IN DISC RESPONSE\n");
					GW_State = WAIT_FOR_GET_FRAME;
				}

				if(HDLC_STRUCT->RX_Count > 0)
				{
					GW_State = RESPONSE_FOR_MDM;
				}

				break;

			default:

				break;

		}

		usleep(1000);
	}

}


/***********************************************************************************************
 * Gateway Initialize Function
 ***********************************************************************************************/
void GW_Run_Init(Buffer* GW_STRUCT,Buffer* HDLC_STRUCT)							//Initializing some variables
{
	report(GATEWAY, CONNECTION, "RUN INIT");
	GW_State = WAIT_FOR_GET_FRAME;
}

/***********************************************************************************************
 * GW2HDLC_Frame_Convertor
 ***********************************************************************************************/
int16_t GW2HDLC_Frame_Convertor (Buffer* GW_STRUCT, Buffer* HDLC_STRUCT, CTRL_BYTE_STR_TD* Control_Byte_Struct)		//Converting GW frame to HDLC frame
{
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

	uint16_t	HCS_Count		= 0	;
	uint16_t	FCS_Count		= 0	;


	memset(APDU, 0, sizeof(APDU));

	/* Detecting APDU in GW frame */
	Add_Len 		= GW_STRUCT->RX[10]						;									//Reading Physical Address length from 10th array of received format
	APDU_start_add 	= HEADER_PREFIX_MIN_SIZE_GW_PRTCL + Add_Len		;									//Calculating start point to read APDU information
//	APDU_size 		= (((uint16_t) (GW_STRUCT->RX[6])) << 8) | GW_STRUCT->RX[7]	;		//Reading APDU bytes size from received frame
//	memcpy(APDU, GW_STRUCT->RX + APDU_start_add, APDU_size)	;

	MAC_frame_Size = MIN_HDLC_SIZE;			//Min MAC frame size

	Logical_Address = (((uint16_t) (GW_STRUCT->RX[HES_DST_ADD_START_BYTE])) << 8) | ((uint16_t) (GW_STRUCT->RX[HES_DST_ADD_START_BYTE + 1]));

	if(Add_Len == 1)						//Max physical address (1-byte): 0b 0111 1111 = 127 => 0b 1111 1111
	{
		dst_size 		= 2;
		MAC_frame_Size += 2;
		phy_add 		= ((uint16_t) (GW_STRUCT->RX[PHY_ADD_START_BYTE]));
	}
	else if(Add_Len == 2) 					//Max physical address(2-byte): 0b 0011 1111 1111 1111 = 16383 => 0b 1111 1110 1111 1111
	{
		dst_size = 4;
		MAC_frame_Size += 4;
		phy_add = (((uint16_t) (GW_STRUCT->RX[PHY_ADD_START_BYTE])) << 8) | ((uint16_t) (GW_STRUCT->RX[PHY_ADD_START_BYTE + 1]));
	}
	else
	{
		report(GATEWAY, CONNECTION, "W2HDLC_Frame_Convertor: ERROR - PHYSICAL ADDRESS SIZE MUST BE 1 OR 2 - IT IS %d", Add_Len);
		return -1;
	}

	uint8_t MAC_frame[2048];
	memset(MAC_frame, 0, sizeof(MAC_frame));

	MAC_frame[0] 				= 0x7E;		//Start flag

	if(0 <= Logical_Address && Logical_Address <= 16383)
	{
		dst_add_log = ((Logical_Address << 2) & (0xFE00)) | ((Logical_Address << 1) & (0x00FE));
	}
	else
	{
		report(GATEWAY, CONNECTION, "GW2HDLC_Frame_Convertor: ERROR - LOGICAL ADDRESS MUST BE BETWEEN 0 AND 16383.");
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

	MAC_frame[last_byte + 1] = ((GW_STRUCT->RX[HES_SRC_ADD_START_BYTE+1] << 1) & (0xFE)) | 0x1;			//Source Address

	if(Control_Byte_Struct->Ctrl_Byte >= 0 && Control_Byte_Struct->Ctrl_Byte <= 255)
	{
		MAC_frame[last_byte + 2] = Control_Byte_Struct->Ctrl_Byte;
	}
	else
	{
		report(GATEWAY, CONNECTION, "GW2HDLC_Frame_Convertor: ERROR - PHYSICAL ADDRESS MUST BE BETWEEN 0 AND 255.");
		return -3;
	}


	HCS_Count = last_byte+2;

	MAC_frame[last_byte + 5] = 0xE6;	//LLC sub-layer
	MAC_frame[last_byte + 6] = 0xE6;
	MAC_frame[last_byte + 7] = 0x00;

	APDU_start_byte = last_byte + MAC_MIN_APDU_START_BYTE;

	APDU_size = 0;
	for(int k=APDU_start_add; k<GW_STRUCT->RX_Count; k++)
	{
		MAC_frame[APDU_start_byte + APDU_size] = GW_STRUCT->RX[k];
		APDU_size ++;
	}
	last_byte = APDU_start_byte + APDU_size - 1;

	FCS_Count = last_byte;

	MAC_frame_Size 						= MAC_frame_Size + APDU_size + LLC_SUB_LAYER_SIZE;
	MAC_frame[MAC_frame_Size-1] 		= 0x7E										;		//End flag
	Frame_Format 						= ((MAC_frame_Size - 2) & 0x07FF) | (0xA000);
	MAC_frame[FRAME_FRMT_START_BYTE] 	= (uint8_t) (Frame_Format >> 8)				;
	MAC_frame[FRAME_FRMT_START_BYTE+1] 	= (uint8_t) (Frame_Format & 0xFF)			;

	HCS = countCRC(MAC_frame, 1, HCS_Count);
	MAC_frame[HCS_Count + 1] = (uint8_t) (HCS >> 8);
	MAC_frame[HCS_Count + 2] = (uint8_t) (HCS & 0x00FF);

	FCS = countCRC(&MAC_frame, 1, FCS_Count);
	MAC_frame[FCS_Count + 1] = (uint8_t) (FCS >> 8);
	MAC_frame[FCS_Count + 2] = (uint8_t) (FCS & 0x00FF);

	memcpy(HDLC_STRUCT->TX, MAC_frame, sizeof(MAC_frame));

	HDLC_STRUCT->TX_Count = MAC_frame_Size;

	Ctrl_Byte_Last = Control_Byte;

	if(Control_Byte_Struct->SSS >= 7)
	{
		Control_Byte_Struct->SSS = 0;
	}
	else
	{
		Control_Byte_Struct->SSS ++;
	}

	return 1;
}

/***********************************************************************************************
 * Meter2GW_Frame_Convertor
 ***********************************************************************************************/
int Meter2GW_Frame_Convertor (Buffer* HDLC_STRUCT, Buffer* GW_STRUCT, CTRL_BYTE_STR_TD* Control_Byte_Struct)		//Converting HDLC frame to GW Frame
{

	uint32_t 	HDLC_Frame_Byte_Index = 0;
	uint32_t 	APDU_First_Byte_Index_in_HDLC_Frame = 0;
	uint32_t 	APDU_Last_Byte_Index_in_HDLC_Frame = 0;
	uint32_t	GW_Frame_tmp_size = 0;
	uint32_t	APDU_len = 0;
	uint16_t 	HCS_in_HDLC_Frame = 0;
	uint16_t 	HCS_Calculated = 0;
	uint16_t 	FCS_in_HDLC_Frame = 0;
	uint16_t 	FCS_Calculated = 0;
	uint16_t	Log_Add = 0;
	uint16_t	Phy_Add = 0;
	uint8_t		Src_Add_First_Byte_Index_in_HDLC_Frame = 0;
	uint8_t		GW_Frame_tmp[5000];
	uint8_t		Control_Byte_Index = 0;
	uint8_t		LLC_sublayer_size = 0;


	if(HDLC_STRUCT->RX[START_FLAG_BYTE] != FLAG_VALUE_IN_HDLC_FRAME)
		return -1;

	if(HDLC_STRUCT->RX[HDLC_STRUCT->RX_Count-1] != FLAG_VALUE_IN_HDLC_FRAME)
		return -2;

	/* FINDING AND CHECKING HCS IN HDLC FRAME - Start *******************/

	HDLC_Frame_Byte_Index = DST_ADD_START_BYTE;		//Indicates destination address first byte index

	while(HDLC_Frame_Byte_Index < HDLC_STRUCT->RX_Count)
	{
		if((HDLC_STRUCT->RX[HDLC_Frame_Byte_Index] & 0x01) == 0x01)			//destination address end byte index
		{
			HDLC_Frame_Byte_Index ++;	//Indicates source address first byte index
			Src_Add_First_Byte_Index_in_HDLC_Frame = HDLC_Frame_Byte_Index;
			break;
		}
		HDLC_Frame_Byte_Index ++;
	}

	while(HDLC_Frame_Byte_Index < HDLC_STRUCT->RX_Count)
	{
		if((HDLC_STRUCT->RX[HDLC_Frame_Byte_Index] & 0x01) == 0x01)			//source address end byte index
		{
			HDLC_Frame_Byte_Index ++;	//Indicates control byte index
			Control_Byte_Index = HDLC_Frame_Byte_Index;
			break;
		}
		HDLC_Frame_Byte_Index ++;
	}

	HCS_Calculated = countCRC(HDLC_STRUCT->RX, 1, HDLC_Frame_Byte_Index);		// second argument (1) : start index : frame format field first byte /-/ third argument : HDLC_Frame_Byte_Index : the number of bytes that we are going to calculate their CRC
	HDLC_Frame_Byte_Index ++;	//Indicates HCS first byte index
	HCS_in_HDLC_Frame = ((uint16_t) (HDLC_STRUCT->RX[HDLC_Frame_Byte_Index]) << 8);
	HDLC_Frame_Byte_Index ++;															//Indicates HCS second byte index
	HCS_in_HDLC_Frame = HCS_in_HDLC_Frame | (HDLC_STRUCT->RX[HDLC_Frame_Byte_Index]); 	//HCS in meter's frame
	HDLC_Frame_Byte_Index ++;															//Indicates APDU first byte index
	APDU_First_Byte_Index_in_HDLC_Frame = HDLC_Frame_Byte_Index ;

	if(HCS_Calculated != HCS_in_HDLC_Frame)
		return -3;

	if((HDLC_STRUCT->RX_Count - HDLC_Frame_Byte_Index - 1) <= 3)				//Indicates that the received frame has no APDU - 3:2byte for FCS + 1 byte for end flag
	{
		report(GATEWAY, CONNECTION, "Meter2GW_Frame_Convertor: ERROR - ERROR FRAME IN RECEIVED FRAME FROM METER");
		return -4;
	}
	/* End - FINDING AND CHECKING HCS IN HDLC FRAME *******************/

	/* FINDING AND CHECKING FCS IN HDLC FRAME - Start *******************/

	APDU_Last_Byte_Index_in_HDLC_Frame = HDLC_STRUCT->RX_Count - 4;		//4 bytes = 1 byte for End flag + 2 bytes for FCS size + 1 byte for index (index == size -1)
	FCS_Calculated = countCRC(HDLC_STRUCT->RX, 1, APDU_Last_Byte_Index_in_HDLC_Frame);
	FCS_in_HDLC_Frame = ((uint16_t) (HDLC_STRUCT->RX[APDU_Last_Byte_Index_in_HDLC_Frame+1]) << 8);
	FCS_in_HDLC_Frame = FCS_in_HDLC_Frame | (HDLC_STRUCT->RX[APDU_Last_Byte_Index_in_HDLC_Frame+2]);

	if(FCS_Calculated != FCS_in_HDLC_Frame)
		return -5;

	/* End - FINDING AND CHECKING FCS IN HDLC FRAME *******************/

	//increasing RRR in control byte
	if(Control_Byte_Struct->RRR >= 7)
		Control_Byte_Struct->RRR = 0;
	else
		Control_Byte_Struct->RRR ++;

	/* COPYING APDU FROM HDLC FRAME TO A APDU BUFFER - Start **********************/

	//Explanation: If APDU has LLC sublayer (E6 E7 00), we have to remove it
	if((HDLC_STRUCT->RX[APDU_First_Byte_Index_in_HDLC_Frame] == 0xE6) && (HDLC_STRUCT->RX[APDU_First_Byte_Index_in_HDLC_Frame + 1] == 0xE7) && (HDLC_STRUCT->RX[APDU_First_Byte_Index_in_HDLC_Frame + 2] == 0x0))	//indicates there is LLC sublayer in frame
		LLC_sublayer_size = 3;
	else
		LLC_sublayer_size = 0;

	memcpy(APDU_tmp_buffer + APDU_tmp_buffer_index, HDLC_STRUCT->RX + APDU_First_Byte_Index_in_HDLC_Frame + LLC_sublayer_size, APDU_Last_Byte_Index_in_HDLC_Frame - APDU_First_Byte_Index_in_HDLC_Frame +1 - LLC_sublayer_size);
	//Explanation: RX + APDU_First_Byte_Index_in_HDLC_Frame + 3: because 3 bytes are LLC sub_layer
	APDU_tmp_buffer_index += (APDU_Last_Byte_Index_in_HDLC_Frame - APDU_First_Byte_Index_in_HDLC_Frame + 1 - LLC_sublayer_size);		//3 bytes are LLC sub-layer

	/* END - COPYING APDU FROM HDLC FRAME TO A APDU BUFFER **********************/


	/* CHECKING SEGMENTATION BIT EXISTANCE - Start **********************/

	if((HDLC_STRUCT->RX[FRAME_FRMT_START_BYTE] & SEGMENT_BIT_FRAME_FORMAT_BYTE) == SEGMENT_BIT_FRAME_FORMAT_BYTE)
		return 1;

	/* END - CHECKING SEGMENTATION BIT EXISTANCE **********************/

	/* FORMING GW FRAME - Start **********************/
	memset(GW_Frame_tmp, 0, sizeof(GW_Frame_tmp));
	GW_Frame_tmp_size = 0;

	//Version bytes (0,1)
	GW_Frame_tmp[VERSION_START_BYTE] 	= 0;
	GW_Frame_tmp[VERSION_START_BYTE+1] 	= 1;

	//Source (logical) address (2,3) and physical address
	if(Control_Byte_Index - Src_Add_First_Byte_Index_in_HDLC_Frame == 4)
	{
		Log_Add = (uint16_t) ((HDLC_STRUCT->RX[Src_Add_First_Byte_Index_in_HDLC_Frame+1] >> 1) | ((uint8_t) (HDLC_STRUCT->RX[Src_Add_First_Byte_Index_in_HDLC_Frame] << 6)));
		Log_Add = Log_Add | ((uint16_t) ((HDLC_STRUCT->RX[Src_Add_First_Byte_Index_in_HDLC_Frame] & 0xFC)<<6));

		GW_Frame_tmp[HES_SRC_ADD_START_BYTE] 	= (uint8_t) ((Log_Add >> 8) & 0x00FF);
		GW_Frame_tmp[HES_SRC_ADD_START_BYTE+1] 	= (uint8_t) ((Log_Add) & 0x00FF);			//Source address

		Phy_Add = (uint16_t) ((HDLC_STRUCT->RX[Src_Add_First_Byte_Index_in_HDLC_Frame+3] >> 1) | ((uint8_t) (HDLC_STRUCT->RX[Src_Add_First_Byte_Index_in_HDLC_Frame+2] << 6)));
		Phy_Add = Phy_Add | ((uint16_t) ((HDLC_STRUCT->RX[Src_Add_First_Byte_Index_in_HDLC_Frame+2] & 0xFC)<<6));

		if(Phy_Add>=256)
		{
			GW_Frame_tmp[ADD_LEN_BYTE] = 2;		//physical address length: 2 Bytes
			GW_Frame_tmp[HES_PHY_ADD_START_BYTE] 	= (uint8_t) ((Phy_Add >> 8) & 0x00FF);
			GW_Frame_tmp[HES_PHY_ADD_START_BYTE+1] 	= (uint8_t) (Phy_Add & 0x00FF);
		}
		else
		{
			GW_Frame_tmp[ADD_LEN_BYTE] = 1;		//physical address length: 1 Bytes
			GW_Frame_tmp[HES_PHY_ADD_START_BYTE] 	= (uint8_t) (Phy_Add & 0x00FF);
		}
	}
	else if(Control_Byte_Index - Src_Add_First_Byte_Index_in_HDLC_Frame == 2)
	{
		Log_Add = ((HDLC_STRUCT->RX[Src_Add_First_Byte_Index_in_HDLC_Frame] >> 1));
		Phy_Add = (HDLC_STRUCT->RX[Src_Add_First_Byte_Index_in_HDLC_Frame+1] >> 1);

		GW_Frame_tmp[HES_SRC_ADD_START_BYTE + 1]= (uint8_t) (Log_Add);

		GW_Frame_tmp[ADD_LEN_BYTE] 			= 1; 	//physical address length
		GW_Frame_tmp[HES_PHY_ADD_START_BYTE]= (uint8_t) (Phy_Add & 0x00FF);
	}

	//destination address
	GW_Frame_tmp[HES_DST_ADD_START_BYTE] 	= 0;
	GW_Frame_tmp[HES_DST_ADD_START_BYTE+1] 	= HDLC_STRUCT->RX[Src_Add_First_Byte_Index_in_HDLC_Frame-1] >> 1;

	//APDU length
	APDU_len=3+GW_Frame_tmp[ADD_LEN_BYTE]+APDU_tmp_buffer_index; 	//3 : Header + Network ID + Physical address
	GW_Frame_tmp[APDU_LEN_START_BYTE] 	= (uint8_t) (APDU_len >> 8);
	GW_Frame_tmp[APDU_LEN_START_BYTE+1] = (uint8_t) (APDU_len & 0x00FF);

	//Header byte
	GW_Frame_tmp[HEADER_BYTE] = HEADER_VALUE_FOR_GW_FRAME;

	//Network ID
	GW_Frame_tmp[NETWORK_ID_BYTE]	= 0;

	memcpy(GW_Frame_tmp+(ADD_LEN_BYTE+GW_Frame_tmp[ADD_LEN_BYTE]+1), APDU_tmp_buffer, APDU_tmp_buffer_index);
	//Explanation: ADD_LEN_BYTE+GW_Frame_tmp[ADD_LEN_BYTE]+1: First byte in APDU section in GW frame


	/* END - FORMING GW FRAME **********************/

	memcpy(GW_STRUCT->TX, GW_Frame_tmp, ADD_LEN_BYTE+GW_Frame_tmp[ADD_LEN_BYTE]+1+APDU_tmp_buffer_index);
	GW_STRUCT->TX_Count = 1+ADD_LEN_BYTE+GW_Frame_tmp[ADD_LEN_BYTE]+APDU_tmp_buffer_index;

	//Reset APDU_tmp_buffer
	memset(APDU_tmp_buffer, 0, sizeof(APDU_tmp_buffer));
	APDU_tmp_buffer_index = 0;

	return 0;
}

/***********************************************************************************************
 * GW2HDLC_SNRM_Generator
 ***********************************************************************************************/
uint8_t GW2HDLC_SNRM_Generator (Buffer* GW_STRUCT, Buffer* HDLC_STRUCT)		//Generating SNRM frame (HDLC frame)
{
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
	Add_Len 		= GW_STRUCT->RX[ADD_LEN_BYTE];																			//Reading Physical Address length from 10th array of received format
	APDU_start_add 	= HEADER_PREFIX_MIN_SIZE_GW_PRTCL + Add_Len;															//Calculating start point to read APDU information
	APDU_size 		= (((uint16_t) (GW_STRUCT->RX[APDU_LEN_START_BYTE])) << 8) | GW_STRUCT->RX[APDU_LEN_START_BYTE+1];		//Reading APDU bytes size from received frame
	memcpy(APDU, GW_STRUCT->RX + APDU_start_add, APDU_size);

	MAC_frame_Size = MIN_SNRM_SIZE;												//Min MAC frame size

	Logical_Address = (((uint16_t) (GW_STRUCT->RX[HES_DST_ADD_START_BYTE])) << 8) | ((uint16_t) (GW_STRUCT->RX[HES_DST_ADD_START_BYTE + 1]));

	if(Add_Len == 2) 					//Max physical address(2-byte): 0b 0011 1111 1111 1111 = 16383 => 0b 1111 1110 1111 1111
	{
		dst_size 		= 4;
		MAC_frame_Size += 4;
		phy_add = (((uint16_t) (GW_STRUCT->RX[PHY_ADD_START_BYTE])) << 8) | ((uint16_t) (GW_STRUCT->RX[PHY_ADD_START_BYTE + 1]));
	}
	else if(Add_Len == 1)						//Max physical address (1-byte): 0b 0111 1111 = 127 => 0b 1111 1111
	{
		dst_size 		= 2;
		MAC_frame_Size += 2;
		phy_add 		= ((uint16_t) (GW_STRUCT->RX[PHY_ADD_START_BYTE]));
	}
	else
	{
		printf("GW2HDLC_SNRM_Generator: Error - Physical Address size must be 1 or 2 - it is %d\n", Add_Len);
		return -1;
	}

	unsigned char MAC_frame[MAC_frame_Size];
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

	MAC_frame[last_byte + 1] = ((GW_STRUCT->RX[HES_SRC_ADD_START_BYTE+1] << 1) & (0xFE)) | 0x1;			//Source Address

	MAC_frame[last_byte + 2] = Control_Byte;

	HCS = countCRC(MAC_frame, 1, last_byte+2);
	MAC_frame[last_byte + 3] = (uint8_t) (HCS >> 8);
	MAC_frame[last_byte + 4] = (uint8_t) (HCS & 0x00FF);

	last_byte += 5;

	printf("mac frame size:%d , last byte:%d \n", MAC_frame_Size, last_byte+1);
	memcpy(HDLC_STRUCT->TX, MAC_frame, last_byte+1);

	HDLC_STRUCT->TX_Count = last_byte+1;

	return 1;
}

/***********************************************************************************************
 * Check_GW_Frame_Type
 ***********************************************************************************************/
uint8_t Check_GW_Frame_Type (Buffer* GW_STRUCT)				//Checking first APDU byte in GW frame
{
	printf("add len:%d , rx_start_apdu:0x%x\n", GW_STRUCT->RX[ADD_LEN_BYTE], GW_STRUCT->RX[HES_PHY_ADD_START_BYTE + GW_STRUCT->RX[ADD_LEN_BYTE]]);
	return GW_STRUCT->RX[HES_PHY_ADD_START_BYTE + GW_STRUCT->RX[ADD_LEN_BYTE]];
}


/***********************************************************************************************
 * GW2HDLC_Poll_For_Remained_Data Function
 ***********************************************************************************************/
uint8_t GW2HDLC_Poll_For_Remained_Data (Buffer* GW_STRUCT, Buffer* HDLC_STRUCT, uint8_t Control_Byte)		//Generating receive ready frame
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
	Add_Len 		= GW_STRUCT->RX[10];											//Reading Physical Address length from 10th array of received format
	APDU_start_add 	= HEADER_PREFIX_MIN_SIZE_GW_PRTCL + Add_Len;						//Calculating start point to read APDU information
	APDU_size 		= (((uint16_t) (GW_STRUCT->RX[6])) << 8) | GW_STRUCT->RX[7];		//Reading APDU bytes size from received frame
	memcpy(APDU, GW_STRUCT->RX + APDU_start_add, APDU_size);

	MAC_frame_Size = MIN_SNRM_SIZE;												//Min MAC frame size

	Logical_Address = (((uint16_t) (GW_STRUCT->RX[HES_DST_ADD_START_BYTE])) << 8) | ((uint16_t) (GW_STRUCT->RX[HES_DST_ADD_START_BYTE + 1]));

	if(Add_Len == 1)						//Max physical address (1-byte): 0b 0111 1111 = 127 => 0b 1111 1111
	{
		dst_size 		= 2;
		MAC_frame_Size += 2;
		phy_add 		= ((uint16_t) (GW_STRUCT->RX[PHY_ADD_START_BYTE]));
	}
	else if(Add_Len == 2) 					//Max physical address(2-byte): 0b 0011 1111 1111 1111 = 16383 => 0b 1111 1110 1111 1111
	{
		dst_size 		= 4;
		MAC_frame_Size += 4;
		phy_add = (((uint16_t) (GW_STRUCT->RX[PHY_ADD_START_BYTE])) << 8) | ((uint16_t) (GW_STRUCT->RX[PHY_ADD_START_BYTE + 1]));
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

	MAC_frame[last_byte + 1] = ((GW_STRUCT->RX[HES_SRC_ADD_START_BYTE+1] << 1) & (0xFE)) | 0x1;			//Source Address

	MAC_frame[last_byte + 2] = Control_Byte;

	HCS = countCRC(MAC_frame, 1, last_byte+2);
	MAC_frame[last_byte + 3] = (uint8_t) (HCS >> 8);
	MAC_frame[last_byte + 4] = (uint8_t) (HCS & 0x00FF);

	last_byte += 5;

	memcpy(HDLC_STRUCT->TX, MAC_frame, last_byte+1);
	HDLC_STRUCT->TX_Count = last_byte+1;

	return (1);
}

/***********************************************************************************************
 * GW2HDLC_DISC_Generator
 ***********************************************************************************************/
int8_t GW2HDLC_DISC_Generator (Buffer* GW_STRUCT, Buffer* HDLC_STRUCT)		//Generating DISC frame
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

	memset(APDU, 0, sizeof(APDU));

	/* Detecting APDU in GW frame */
	Add_Len 		= GW_STRUCT->RX[10];														//Reading Physical Address length from 10th array of received format
	APDU_start_add 	= HEADER_PREFIX_MIN_SIZE_GW_PRTCL + Add_Len;									//Calculating start point to read APDU information
	APDU_size 		= (((uint16_t) (GW_STRUCT->RX[6])) << 8) | GW_STRUCT->RX[7];		//Reading APDU bytes size from received frame
	memcpy(APDU, GW_STRUCT->RX + APDU_start_add, APDU_size);


	MAC_frame_Size = MIN_SNRM_SIZE;												//Min MAC frame size

	Logical_Address = (((uint16_t) (GW_STRUCT->RX[HES_DST_ADD_START_BYTE])) << 8) | ((uint16_t) (GW_STRUCT->RX[HES_DST_ADD_START_BYTE + 1]));

	if(Add_Len == 1)						//Max physical address (1-byte): 0b 0111 1111 = 127 => 0b 1111 1111
	{
		dst_size 		= 2;
		MAC_frame_Size += 2;
		phy_add 		= ((uint16_t) (GW_STRUCT->RX[PHY_ADD_START_BYTE]));
	}
	else if(Add_Len == 2) 					//Max physical address(2-byte): 0b 0011 1111 1111 1111 = 16383 => 0b 1111 1110 1111 1111
	{
		dst_size 		= 4;
		MAC_frame_Size += 4;
		phy_add = (((uint16_t) (GW_STRUCT->RX[PHY_ADD_START_BYTE])) << 8) | ((uint16_t) (GW_STRUCT->RX[PHY_ADD_START_BYTE + 1]));
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

	MAC_frame[last_byte + 1] = ((GW_STRUCT->RX[HES_SRC_ADD_START_BYTE+1] << 1) & (0xFE)) | 0x1;			//Source Address

	MAC_frame[last_byte + 2] = Control_Byte;

	HCS = countCRC(MAC_frame, 1, last_byte+2);
	MAC_frame[last_byte + 3] = (uint8_t) (HCS >> 8);
	MAC_frame[last_byte + 4] = (uint8_t) (HCS & 0x00FF);

	last_byte += 5;

	memcpy(HDLC_STRUCT->TX, MAC_frame, last_byte+1);
	HDLC_STRUCT->TX_Count = last_byte+1;

	printf("||GENERATED DISC FRAME FOR SENDING GW2HDLC - LEN:%d||\n", last_byte+1);

	return (1);
}

/***********************************************************************************************
 * HDLC_Send_DISC
 ***********************************************************************************************/
uint8_t HDLC_Send_DISC (Buffer* GW_STRUCT, Buffer* HDLC_STRUCT)		//Generating DISC frame and set 0 for control byte
{
	int8_t DISC_Gen_Ret = GW2HDLC_DISC_Generator (GW_STRUCT, HDLC_STRUCT);

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
int8_t Check_GW_Frame_Valid (Buffer* GW_STRUCT)		//Checking version, header and network ID in GW frame
{
	if(GW_STRUCT->RX[VERSION_START_BYTE] == 0 && GW_STRUCT->RX[VERSION_START_BYTE+1] == 1) 			//Checking version 0x0001
	{
		if(GW_STRUCT->RX[HEADER_BYTE] == 0xE6)														//Checking header - must be 0xE6
		{
			if(GW_STRUCT->RX[NETWORK_ID_BYTE] == 0x00)												//Checking Network ID - shall be 0x00
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


