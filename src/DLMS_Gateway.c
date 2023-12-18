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

	GW_State = RESPONSE_FOR_MDM;

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


				break;

			case SNRM_RESPONSE:

				if(diff_time_ms(&timeout_start) > HDLC_tmp.Timeout_ms)
				{
					printf("TIME OUT IN SNRM RESPONSE\n");
					GW_State = WAIT_FOR_GET_FRAME;
				}
				if(HDLC_STRUCT->RX_Count > 0)
				{
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
					printf("ERROR in INFORMATION FRAME\n");
					GW_State = WAIT_FOR_GET_FRAME;
				}

				break;

			case RESPONSE:

				if(diff_time_ms(&timeout_start) > HDLC_tmp.Timeout_ms)
				{
					printf("TIME OUT IN RESPONSE\n");
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

					printf("**GW_State:%d\n", GW_State);

				}

				break;

			case SEGMENT:

				HDLC_STRUCT->RX_Count = 0;
				Control_Byte_Struct.Ctrl_Byte = Control_Byte (Control_Byte_Struct.RRR, Control_Byte_Struct.SSS, RECEIVE_READY);
				ret = GW2HDLC_Poll_For_Remained_Data (&GW_tmp, HDLC_STRUCT, Control_Byte_Struct.Ctrl_Byte);
				GW_State = RESPONSE;

				break;

			case RESPONSE_FOR_MDM:
				if(HDLC_STRUCT->RX_Count > 0)
				{
					memset(HDLC_tmp.RX, 0, sizeof(HDLC_tmp.RX));
					memcpy(HDLC_tmp.RX, HDLC_STRUCT->RX, HDLC_STRUCT->RX_Count);
					HDLC_tmp.RX_Count = HDLC_STRUCT->RX_Count;

					HDLC_STRUCT->RX_Count = 0;

					ret = Meter2GW_Frame_Convertor(&HDLC_tmp, GW_STRUCT, &Control_Byte_Struct);
					printf("ret : %d\n", ret);
				}

//				ret = Meter2GW_Frame_Convertor(&HDLC_tmp, GW_STRUCT, &Control_Byte_Struct);

//				printf("ret : %d , GW_State: %d \n", ret, GW_State);

//				GW_State = WAIT_FOR_GET_FRAME;

//				if(ret > 0)
//				{
////					GW_STRUCT->TX_Count = ret;
//					GW_State = WAIT_FOR_GET_FRAME;
//				}
//				else
//				{
//					GW_State = WAIT_FOR_GET_FRAME;
//				}

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
	printf("GW_Run_Init\n");
	GW_State = WAIT_FOR_GET_FRAME;
}

/***********************************************************************************************
 * GW2HDLC_Frame_Convertor
 ***********************************************************************************************/
int16_t GW2HDLC_Frame_Convertor (Buffer* GW_STRUCT, Buffer* HDLC_STRUCT, CTRL_BYTE_STR_TD* Control_Byte_Struct)		//Converting GW frame to HDLC frame
{
//	printf("-----------------------------------------------------------------------\n");
//	printf("RECEIVED FRAME FOR CONVERTING FROM GW2HDLC - LEN:%d\n", GW_STRUCT->RX_Count);
//	for (int i=0; i< GW_STRUCT->RX_Count; i++)
//	{
//		printf("0x%x-",GW_STRUCT->RX[i]);
//	}
//	printf("\n");
//	printf("-----------------------------------------------------------------------\n");

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
		printf("GW2HDLC_Frame_Convertor: Error - Physical Address size must be 1 or 2 - it is %d\n", Add_Len);
		return -1;
	}

//	MAC_frame_Size = MAC_frame_Size + APDU_size + LLC_SUB_LAYER_SIZE;
//
//	uint8_t MAC_frame[MAC_frame_Size];

	uint8_t MAC_frame[2048];
	memset(MAC_frame, 0, sizeof(MAC_frame));

	MAC_frame[0] 				= 0x7E;		//Start flag
//	MAC_frame[MAC_frame_Size-1] = 0x7E;		//End flag

//	Frame_Format 						= ((MAC_frame_Size - 2) & 0x07FF) | (0xA000);
//	MAC_frame[FRAME_FRMT_START_BYTE] 	= (uint8_t) (Frame_Format >> 8)				;
//	MAC_frame[FRAME_FRMT_START_BYTE+1] 	= (uint8_t) (Frame_Format & 0xFF)			;

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

	MAC_frame[last_byte + 1] = ((GW_STRUCT->RX[HES_SRC_ADD_START_BYTE+1] << 1) & (0xFE)) | 0x1;			//Source Address

	if(Control_Byte_Struct->Ctrl_Byte >= 0 && Control_Byte_Struct->Ctrl_Byte <= 255)
	{
		MAC_frame[last_byte + 2] = Control_Byte_Struct->Ctrl_Byte;
	}
	else
	{
		printf("GW2HDLC_Frame_Convertor: Error - Physical Address must be between 0 and 255.\n");
		return -3;
	}


	HCS_Count = last_byte+2;
//	HCS = countCRC(MAC_frame, 1, last_byte+2);
//	MAC_frame[last_byte + 3] = (uint8_t) (HCS >> 8);
//	MAC_frame[last_byte + 4] = (uint8_t) (HCS & 0x00FF);

	MAC_frame[last_byte + 5] = 0xE6;	//LLC sub-layer
	MAC_frame[last_byte + 6] = 0xE6;
	MAC_frame[last_byte + 7] = 0x00;

	APDU_start_byte = last_byte + MAC_MIN_APDU_START_BYTE;

//	for(int k=0; k<APDU_size; k++)
//	{
//		APDU_size ++;
//		MAC_frame[APDU_start_byte + k] = APDU[k];
//	}

	APDU_size = 0;
	for(int k=APDU_start_add; k<GW_STRUCT->RX_Count; k++)
	{
		MAC_frame[APDU_start_byte + APDU_size] = GW_STRUCT->RX[k];
		APDU_size ++;
	}
	last_byte = APDU_start_byte + APDU_size - 1;

	FCS_Count = last_byte;
//	FCS = countCRC(&MAC_frame, 1, last_byte);
//	MAC_frame[last_byte + 1] = (uint8_t) (FCS >> 8);
//	MAC_frame[last_byte + 2] = (uint8_t) (FCS & 0x00FF);

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
	printf("G2M - NR:%d ,NS:%d\n", Control_Byte_Struct->RRR, Control_Byte_Struct->SSS);

//	printf("-----------------------------------------------------------------------\n");
//	printf("PREPARED DLMS FRAME FOR SENDING FROM GW2HDLC - LEN:%d\n", MAC_frame_Size);
//	for (int i=0; i<MAC_frame_Size; i++)
//	{
//		printf("0x%x-",HDLC_STRUCT->TX[i]);
//	}
//	printf("\n");
//	printf("-----------------------------------------------------------------------\n");

	return 1;
}

/***********************************************************************************************
 * Meter2GW_Frame_Convertor
 ***********************************************************************************************/
int Meter2GW_Frame_Convertor (Buffer* HDLC_STRUCT, Buffer* GW_STRUCT, CTRL_BYTE_STR_TD* Control_Byte_Struct)		//Converting HDLC frame to GW Frame
{

	uint32_t 	HDLC_Frame_Byte_Index;
	uint32_t 	APDU_First_Byte_Index_in_HDLC_Frame;
	uint32_t 	APDU_Last_Byte_Index_in_HDLC_Frame;
	uint32_t	GW_Frame_tmp_size;
	uint16_t 	HCS_in_HDLC_Frame;
	uint16_t 	HCS_Calculated;
	uint16_t 	FCS_in_HDLC_Frame;
	uint16_t 	FCS_Calculated;
	uint16_t	Log_Add;
	uint16_t	Phy_Add;
	uint8_t		Src_Add_First_Byte_Index_in_HDLC_Frame;
	uint8_t		GW_Frame_tmp[5000];
	uint8_t		Control_Byte_Index;

//	printf("*-*- Meter2GW_Frame_Convertor:%d -*-*\n", HDLC_STRUCT->RX_Count);
//
//	for(int i = 0; i< HDLC_STRUCT->RX_Count; i++)
//	{
//		printf("%.2X  ", HDLC_STRUCT->RX[i]);
//	}
//	printf("\n");

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

	/* End - FINDING AND CHECKING HCS IN HDLC FRAME *******************/

	/* FINDING AND CHECKING FCS IN HDLC FRAME - Start *******************/
	APDU_Last_Byte_Index_in_HDLC_Frame = HDLC_STRUCT->RX_Count - 4;		//4 bytes = 1 byte for End flag + 2 bytes for FCS size + 1 byte for index (index == size -1)
	FCS_Calculated = countCRC(HDLC_STRUCT->RX, 1, APDU_Last_Byte_Index_in_HDLC_Frame);
	FCS_in_HDLC_Frame = ((uint16_t) (HDLC_STRUCT->RX[APDU_Last_Byte_Index_in_HDLC_Frame+1]) << 8);
	FCS_in_HDLC_Frame = FCS_in_HDLC_Frame | (HDLC_STRUCT->RX[APDU_Last_Byte_Index_in_HDLC_Frame+2]);

	if(FCS_Calculated != FCS_in_HDLC_Frame)
		return -4;

	/* End - FINDING AND CHECKING FCS IN HDLC FRAME *******************/

	//increasing RRR in control byte
	if(Control_Byte_Struct->RRR >= 7)
		Control_Byte_Struct->RRR = 0;
	else
		Control_Byte_Struct->RRR ++;

	/* COPYING APDU FROM HDLC FRAME TO A APDU BUFFER - Start **********************/

	memcpy(APDU_tmp_buffer + APDU_tmp_buffer_index, HDLC_STRUCT->RX + APDU_First_Byte_Index_in_HDLC_Frame + 3, APDU_Last_Byte_Index_in_HDLC_Frame - APDU_First_Byte_Index_in_HDLC_Frame -2);
	//Explanation: RX + APDU_First_Byte_Index_in_HDLC_Frame + 3: because 3 bytes are LLC sub_layer
	APDU_tmp_buffer_index += (APDU_Last_Byte_Index_in_HDLC_Frame - APDU_First_Byte_Index_in_HDLC_Frame -2);		//3 bytes are LLC sub-layer

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



//
//
//	uint16_t	last_byte					= 0;
//	uint16_t	last_byte_for_last_segment	= 0;
//	uint16_t	Phy_Add						= 0;
//	uint16_t	Log_Add						= 0;
//	uint16_t	HDLC_rec_data_size			= 0;
//	uint16_t	APDU_Len_for_Last_Segment	= MIN_GW_APDU_LEN_SIZE;		//len (header + net id + add len)
//
//	uint8_t 	HES_Frame[5000];
//	uint32_t 	HES_Frame_Size=0;
//	uint32_t 	HES_Frame_Size_for_Last_Segment=0;
//
//	HES_Frame_Size = HES_MIN_FRAME_SIZE;
//
//	memset(HES_Frame, 0, sizeof(HES_Frame));
//
//	GW_State = WAIT_FOR_GET_FRAME;
//
//	if(HDLC_STRUCT->RX[0] == 0x7E )
//	{
//		if((HDLC_STRUCT->RX[FRAME_FRMT_START_BYTE] & 0xF0) == 0xA0 )
//		{
//			if((HDLC_STRUCT->RX[FRAME_FRMT_START_BYTE] & 0x08) == 0x08)
//			{
//				Segment_bit = MIDDLE_FRAME_IN_SEGMENTATION;
//				GW_State = SEGMENT;
//			}
//			else if(Segment_bit == MIDDLE_FRAME_IN_SEGMENTATION)
//			{
//				Segment_bit = LAST_FRAME_IN_SEGMENTATION;
//				GW_State = WAIT_FOR_GET_FRAME;
//			}
//			else
//			{
//				Segment_bit = SINGLE_FRAME_IN_SEGMENTATION;
//				memset(Buffer_Data_Meter2GW_Frame_Convertor, 0, sizeof(Buffer_Data_Meter2GW_Frame_Convertor));
//				Last_Byte_Buffer_Meter2GW = 0;
//				GW_State = WAIT_FOR_GET_FRAME;
//			}
//
//			HES_Frame[VERSION_START_BYTE] 	= 0;
//			HES_Frame[VERSION_START_BYTE+1] = 1;				//Version
//
//			HES_Frame[HEADER_BYTE] 			= 0xE7;				//Header
//			HES_Frame[NETWORK_ID_BYTE] 		= 0;				//Network ID
//			HES_Frame[ADD_LEN_BYTE]			= 0;
//
//			HDLC_rec_data_size = (((((uint16_t) (HDLC_STRUCT->RX[FRAME_FRMT_START_BYTE])) << 8) | ((uint16_t) (HDLC_STRUCT->RX[FRAME_FRMT_START_BYTE+1]))) & 0x07FF) + 2;
//
//			if(HDLC_STRUCT->RX[HDLC_rec_data_size-1] != 0x7E)
//			{
//				printf("Meter2GW_Frame_Convertor: Error - End flag is wrong. \n");
//				return -7;
//			}
//
//			for(int i=3; i<HDLC_rec_data_size; i++)				//Finding end of destination address
//			{
//				if((HDLC_STRUCT->RX[i] & 0x1) == 0x1)
//				{
//					HES_Frame[HES_DST_ADD_START_BYTE+1] = (HDLC_STRUCT->RX[i] >> 1);		//Destination Address
//					last_byte = i;
//					break;
//				}
//			}
//
//			for(int i=last_byte+1; i<HDLC_rec_data_size; i++)	//Finding end of source address
//			{
//				if((HDLC_STRUCT->RX[i] & 0x1) == 0x1)
//				{
//					if(i-last_byte == 4)		//Detecting size of source address
//					{
//						Log_Add = (uint16_t) ((HDLC_STRUCT->RX[i-2] >> 1) | ((uint8_t) (HDLC_STRUCT->RX[i-3] << 6)));
//						Log_Add = Log_Add | ((uint16_t) ((HDLC_STRUCT->RX[i-3] & 0xFC)<<6));
//
//						HES_Frame[HES_SRC_ADD_START_BYTE] 	= (uint8_t) ((Log_Add >> 8) & 0x00FF);
//						HES_Frame[HES_SRC_ADD_START_BYTE+1] = (uint8_t) ((Log_Add) & 0x00FF);			//Source address
//
//						Phy_Add = (uint16_t) ((HDLC_STRUCT->RX[i] >> 1) | ((uint8_t) (HDLC_STRUCT->RX[i-1] << 6)));
//						Phy_Add = Phy_Add | ((uint16_t) ((HDLC_STRUCT->RX[i-1] & 0xFC)<<6));
//
//						uint8_t Phy_Add_len = 0;
//
//						if(Phy_Add>=0 && Phy_Add<256)
//						{
//							HES_Frame[ADD_LEN_BYTE] 			= 1;
//							HES_Frame[HES_PHY_ADD_START_BYTE] 	= (uint8_t) (Phy_Add & 0x00FF);
//							APDU_Len_for_Last_Segment ++;
//						}
//						else if(Phy_Add>=256 && Phy_Add<=65535)
//						{
//							HES_Frame[ADD_LEN_BYTE] 			= 2;
//							HES_Frame[HES_PHY_ADD_START_BYTE] 	= (uint8_t) ((Phy_Add >> 8) & 0x00FF);
//							HES_Frame[HES_PHY_ADD_START_BYTE+1] = (uint8_t) (Phy_Add & 0x00FF);
//							APDU_Len_for_Last_Segment += 2;
//						}
//						else
//						{
//							printf("Meter2GW_Frame_Convertor: Error - Physical address must be between 0 and 65535. - %d\n", Phy_Add);
//							return -1;
//						}
//
//						Phy_Add_len 	= HES_Frame[ADD_LEN_BYTE];
//						HES_Frame_Size 	+= Phy_Add_len;
//					}
//					else if(i-last_byte == 2)
//					{
//						Log_Add = ((HDLC_STRUCT->RX[i-1] >> 1));
//						Phy_Add = (HDLC_STRUCT->RX[i] >> 1);
//
//						uint8_t Phy_Add_len = 0;
//
//						if(Phy_Add>=0 && Phy_Add<256)
//						{
//							HES_Frame[HES_SRC_ADD_START_BYTE+1] = (uint8_t) (Log_Add);
//							HES_Frame[ADD_LEN_BYTE] 			= 1;
//							HES_Frame[HES_PHY_ADD_START_BYTE] 	= (uint8_t) (Phy_Add & 0x00FF);
//							APDU_Len_for_Last_Segment ++;
//						}
//						else
//						{
//							printf("Meter2GW_Frame_Convertor: Error - Physical address must be between 0 and 256. - %d\n", Phy_Add);
//							return -2;
//						}
//
//						Phy_Add_len = HES_Frame[ADD_LEN_BYTE];
//						HES_Frame_Size += Phy_Add_len;
//
//					}
//					else
//					{
//						printf("Meter2GW_Frame_Convertor: Error - Source Address is wrong. \n");
//						return -3;
//					}
//					uint16_t HES_APDU_Byte = ADD_LEN_BYTE + HES_Frame[ADD_LEN_BYTE] + 1;	//determining APDU start byte in MDM's frame
//					uint16_t HES_APDU_Byte_for_Last_segment = HES_APDU_Byte;
//
//					last_byte = i;
//
//					if(Segment_bit == SINGLE_FRAME_IN_SEGMENTATION)
//					{
//						Ctrl_Byte_Last = HDLC_STRUCT->RX[last_byte+1];
//					}
//
//					uint16_t HCS_Frame 				= 0	;
//					uint16_t HCS_Cal 				= 0	;
//					HCS_Frame 						= ((uint16_t) (HDLC_STRUCT->RX[last_byte+2]) << 8) | (HDLC_STRUCT->RX[last_byte+3]); //HCS in meter's frame
//					HCS_Cal 						= countCRC(HDLC_STRUCT->RX, 1, last_byte + 1);
//					last_byte 						= last_byte +3; 						//len(CRC)+len(ControlByte)
//					last_byte_for_last_segment 		= last_byte;
//
//					uint16_t APDU_len 				= 0;
//					int32_t APDU_len_real 			= 0;
//					APDU_len 						= HDLC_rec_data_size - last_byte -4 + (3 + HES_Frame[ADD_LEN_BYTE]);		//4 = len(FCS) + len(end flag) + (index is less in value one) / last (prefix len) due to gurux application
//					APDU_len_real 					= HDLC_rec_data_size - last_byte -4;										//4 = len(FCS) + len(end flag) + (index is less in value one) / last (prefix len) due to gurux application
//					HES_Frame_Size_for_Last_Segment = HES_Frame_Size;
//					HES_Frame_Size					+= APDU_len_real;
//
//					if(APDU_len_real < 0)
//					{
//						printf("Meter2GW_Frame_Convertor: NO INFORMATION \n");
//						return -4;
//					}
//
//					APDU_len-=3; //najafi
//
//					HES_Frame[APDU_LEN_START_BYTE] 			= (uint8_t) (APDU_len >> 8);
//					HES_Frame[APDU_LEN_START_BYTE+1] 		= (uint8_t) (APDU_len & 0x00FF);
//
//
//					if(HCS_Frame == HCS_Cal)
//					{
//						for(int j = last_byte+4; j < HDLC_rec_data_size-3; j++) //najafi
//						{
//							Buffer_Data_Meter2GW_Frame_Convertor[Last_Byte_Buffer_Meter2GW] = HDLC_STRUCT->RX[j];
//							Last_Byte_Buffer_Meter2GW ++;
//
//							HES_Frame[HES_APDU_Byte] = HDLC_STRUCT->RX[j];
//							HES_APDU_Byte ++;
//							last_byte ++;
//						}
//
//						uint16_t FCS_Frame 	= 0;
//						uint16_t FCS_Cal 	= 0;
//						FCS_Frame 	= ((uint16_t) (HDLC_STRUCT->RX[last_byte + 1]) << 8) | (HDLC_STRUCT->RX[last_byte + 2]);
//						FCS_Cal 	= countCRC(HDLC_STRUCT->RX, 1, last_byte);
//
//						//if(FCS_Frame == FCS_Cal)   //najafi
//						if(1)
//						{
//							Gate_HES_size = HES_Frame_Size;
//							memcpy(GW_STRUCT->TX, HES_Frame, sizeof(HES_Frame));
//
//							if(Control_Byte_Struct->RRR >= 7)
//							{
//								Control_Byte_Struct->RRR = 0;
//							}
//							else
//							{
//								Control_Byte_Struct->RRR ++;
//							}
//							printf("M2G - NR:%d ,NS:%d\n", Control_Byte_Struct->RRR, Control_Byte_Struct->SSS);
//
//							if(Segment_bit == LAST_FRAME_IN_SEGMENTATION)
//							{
//								HES_Frame_Size_for_Last_Segment	+= Last_Byte_Buffer_Meter2GW;
//
//
//								for(int n=0; n<Last_Byte_Buffer_Meter2GW; n++)
//								{
//									HES_Frame[HES_APDU_Byte_for_Last_segment] = Buffer_Data_Meter2GW_Frame_Convertor[n];
//									HES_APDU_Byte_for_Last_segment ++;
//									last_byte_for_last_segment ++;
//								}
//
//								APDU_Len_for_Last_Segment = HES_Frame_Size_for_Last_Segment - 8;
//								HES_Frame[APDU_LEN_START_BYTE] 			= (uint8_t) (APDU_Len_for_Last_Segment >> 8);
//								HES_Frame[APDU_LEN_START_BYTE+1] 		= (uint8_t) (APDU_Len_for_Last_Segment & 0x00FF);
//
////								memcpy(GW_STRUCT->TX, HES_Frame, sizeof(HES_Frame));
//								memcpy(GW_STRUCT->TX, HES_Frame, HES_Frame_Size_for_Last_Segment-3); //najafi
//								GW_STRUCT->TX_Count = HES_Frame_Size_for_Last_Segment-3;			//najafi
////								printf("--------------------------------------------------------\n");
////								printf("Meter2GW_Frame_Convertor-Converted_SB:%d\n", Last_Byte_Buffer_Meter2GW);
////								for(int i=0; i<Last_Byte_Buffer_Meter2GW+1; i++)
////								{
////									printf("0x%x-", GW_STRUCT->TX[i]);
////								}
////								printf("\n--------------------------------------------------------\n");
//
//								memset(Buffer_Data_Meter2GW_Frame_Convertor, 0, sizeof(Buffer_Data_Meter2GW_Frame_Convertor));
//								Last_Byte_Buffer_Meter2GW = 0;
//
//								return HES_Frame_Size_for_Last_Segment;
////								return 1;
//							}
//							else if(Segment_bit == SINGLE_FRAME_IN_SEGMENTATION)
//							{
////								printf("--------------------------------------------------------\n");
////								printf("Meter2GW_Frame_Convertor-Converted-Single Frame:%d\n", Last_Byte_Buffer_Meter2GW);
////								for(int i=0; i<HES_Frame_Size; i++)
////								{
////									printf("0x%x-", GW_STRUCT->TX[i]);
////								}
////								printf("\n--------------------------------------------------------\n");
//
//								GW_STRUCT->TX_Count = HES_Frame_Size-3; //najafi
//
//								memset(Buffer_Data_Meter2GW_Frame_Convertor, 0, sizeof(Buffer_Data_Meter2GW_Frame_Convertor));
//								Last_Byte_Buffer_Meter2GW = 0;
//
//								return HES_Frame_Size;
////								return 2;
//							}
//
//							return 0;
//						}
//						else
//						{
//							printf("Meter2GW_Frame_Convertor: Error - FCS is wrong 0x%.2X. \n", FCS_Cal);
//							return -5;
//						}
//					}
//					else
//					{
//						printf("Meter2GW_Frame_Convertor: Error - HCS is wrong. \n");
//						return -6;
//					}
//
//					break;
//				}
//			}
//		}
//		else
//		{
//			printf("Meter2GW_Frame_Convertor: Error - Frame format field is wrong.-0x%x \n", HDLC_STRUCT->RX[1]);
//			return -8;
//		}
//	}
//	else
//	{
//		printf("Start flag is wrong. \n");
//		return -9;
//	}
//	return 0;
}

/***********************************************************************************************
 * GW2HDLC_SNRM_Generator
 ***********************************************************************************************/
uint8_t GW2HDLC_SNRM_Generator (Buffer* GW_STRUCT, Buffer* HDLC_STRUCT)		//Generating SNRM frame (HDLC frame)
{
//	printf("-----------------------------------------------------------------------\n");
//	printf("RECEIVED FRAME FOR GENERATING SNRM - LEN:%d\n", GW_STRUCT->RX_Count);
//	for (int i=0; i < GW_STRUCT->RX_Count; i++)
//	{
//		printf("0x%x-",GW_STRUCT->RX[i]);
//	}
//	printf("\n");
//	printf("-----------------------------------------------------------------------\n");

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


//	printf("-----------------------------------------------------------------------\n");
//	printf("GENERATED SNRM (HDLC FRAME) FRAME FOR SENDING GW2HDLC - LEN:%d\n", last_byte+1);
//	for (int i=0; i<last_byte+1; i++)
//	{
//		printf("0x%x-",HDLC_STRUCT->TX[i]);
//	}
//	printf("\n");
//	printf("-----------------------------------------------------------------------\n");

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

//	printf("\n-----------------------------------------------------------------\n");
//	printf("||PREPARED RECEIVE READY FRAME FOR SENDING FROM GW2DLMS - LEN:%d||\n", last_byte+1);
//	for(int v=0; v<last_byte+1; v++)
//	{
//		printf("0x%x-", HDLC_STRUCT->TX[v]);
//	}
//	printf("\n-----------------------------------------------------------------\n");

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

//	printf("\n-----------------------------------------------------------------\n");
//	printf("||RECEIVED FRAM FOR GENERATING DISC FRAME - LEN:%d||\n", GW_STRUCT->RX_Count);
//	for(int v=0; v<last_byte+1; v++)
//	{
//		printf("0x%x-", GW_STRUCT->RX[v]);
//	}
//	printf("\n-----------------------------------------------------------------\n");


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

//	printf("\n-----------------------------------------------------------------\n");
	printf("||PREPARED DISC FRAME FOR SENDING GW2HDLC - LEN:%d||\n", last_byte+1);
//	for(int v=0; v<last_byte+1; v++)
//	{
//		printf("0x%x-", HDLC_STRUCT->TX[v]);
//	}
//	printf("\n-----------------------------------------------------------------\n");

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


