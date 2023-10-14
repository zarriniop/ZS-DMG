/**  
 *@file     ql_audio.h
 *@date     2019-09-04
 *@author   juson.zhang
 *@brief    Quecte Lib Audio
 */
#ifndef __QL_AUDIO_H__
#define __QL_AUDIO_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/****************************************************************************
*  Audio Format
***************************************************************************/
typedef enum {
	AUD_STREAM_FORMAT_MP3 = 0,
	AUD_STREAM_FORMAT_AMR = 1,
	AUD_STREAM_FORMAT_PCM = 2,
	AUD_STREAM_FORMAT_AMRNB,
	AUD_STREAM_FORMAT_AMRWB,
	AUD_STREAM_FORMAT_END
} Enum_AudStreamFormat;

/****************************************************************************
*  Audio Volume Level Definition
***************************************************************************/
typedef enum {
	AUD_VOLUME_LEVEL1 = 0,
	AUD_VOLUME_LEVEL2,
	AUD_VOLUME_LEVEL3,
	AUD_VOLUME_LEVEL4,
	AUD_VOLUME_LEVEL5,
	AUD_VOLUME_LEVEL_END
}Enum_AudVolumeLevel;

/****************************************************************************
*  Audio Direct
***************************************************************************/
typedef enum {
	AUD_LINK_REVERSE = 0,
	AUD_LINK_FORWARD = 1,
	AUD_LINK_BOTH = 2,
	AUD_LINK_INVALID
}Enum_AudStreamDirection;

struct ST_MediaParams {
	Enum_AudStreamFormat     format;
	Enum_AudVolumeLevel      volume;
	Enum_AudStreamDirection     direct;
};

/**
 * @brief   Player state in user callback
 */
typedef enum {
    AUD_PLAYER_ERROR = -1,
    AUD_PLAYER_START = 0,
    AUD_PLAYER_PAUSE,
    AUD_PLAYER_RESUME,      ///<not used
    AUD_PLAYER_NODATA,      ///<not used
    AUD_PLAYER_LESSDATA,    ///<not used
    AUD_PLAYER_FINISHED,
} Enum_AudPlayer_State;

/**
 * @brief   Recorder state in user callback
 */
typedef enum {
    AUD_RECORDER_ERROR = -1,
    AUD_RECORDER_START = 0,
    AUD_RECORDER_PAUSE,     ///<not used
    AUD_RECORDER_RESUME,    ///<not used
    AUD_RECORDER_FINISHED,
} Enum_AudRecorder_State;

/*
    audmod set.
     -value   : the value of audmod (0~2) 0:handset  1:headset   2:speaker
	return:
		0 : success
		-1: error
*/
int ql_audmod_set(int value);
//end gunner

/**
 * @brief This callback function handles the result of audio player.
 *
 * @param hdl    Player handle, received from Ql_AudPlayer_Open().
 * @param result Reference to Enum_AudPlayer_State.
 *
 * @return Not care.
 */
typedef int(*_cb_onPlayer)(int hdl, int result);

/**
 * @brief This callback function handles the result of audio recorder.
 *
 * @param result
 *      Reference to Enum_AudRecorder_State.
 * @param pBuf
 *      Data buffer.
 * @param length
 *      Data length.
 *      If encode operation is failed, length is -1;
 *
 * @return
 *      Not care
 */
typedef int(*_cb_onRecorder)(int result, unsigned char* pBuf, unsigned int length);

/**
 * @brief Open player device.
 * @details
 *      Just call function Ql_AudPlayer_OpenExt(device, cb_func,
 *                               PCM_NMMAP, 1, 8000, SNDRV_PCM_FORMAT_S16_LE); \n
 *      Set default parameters MONO, 8khz samplerate, 16bit little endian.
 *
 * @param device
 *      Pcm device, eg: "hw:0,0"
 * @param cb_func
 *      User callback functions.
 *
 * @return 
 *      Succeed: player device handle. \n
 *      Fail:    -1.
 */
int  Ql_AudPlayer_Open(char* device, _cb_onPlayer cb_func);

/**
 * @brief Writes pcm data to pcm device to play.
 * @details
 *      If player thread is not started, this function will start play
 *      thread first.
 *
 * @param dev
 *      A string that specifies the PCM device. \n
 *      NULL, means the audio will be played on the default PCM device. \n
 *      \n
 *      If you want to mixedly play audio sources, you can call this
 *      API twice with specifying different PCM device. \n
 *      The string devices available: \n
 *         "hw:0,0"  (the default play device) \n
 *         "hw:0,13" (this device can mix audio and TTS) \n
 * @param cb_fun
 *      Callback function for audio player. \n
 *      The results of all operations on audio player
 *      are informed in callback function.
 * @param hdl
 *      Device handle
 * @param pData
 *      Pcm data
 * @param length
 *      Pcm data length
 *
 * @return
 *      Succeed: length has written. \n
 *      Fail: -1.
 */
int  Ql_AudPlayer_Play(int hdl, unsigned char* pData, unsigned int length);

/**
 * @brief Play pcm data from the specified file.
 * @details
 *      This functions just for compatibility with old example in example_audio.c. \n
 *      The player device was opened outside this function and pcm parameters (rate,
 *      channels, format) was already set which may be different with the audio file. \n
 *      eg: the player device is opened and set sample rate 8000 hz, 1 channel. But the 
 *          audio file test.wav is sample by 16000 hz, 1channel. Then the wav file will
 *          be played and extended.
 *      \n
 *      You are supposed to use this function when you want play the file with the
 *      special pcm format you want. Otherwise please use Ql_AudPlayer_PlayFile instead.
 *
 *
 * @param hdl
 *      Device handle.
 * @param fd
 *      Linux file discriper.
 * @param offset
 *      The position start to play in the file.
 *
 * @return
 *      Succeed: 0. \n 
 *      Fail:    -1.
 */
int  Ql_AudPlayer_PlayFrmFile(int hdl, int fd, int offset);

/**
 * @brief Pause playing
 *
 * @param hdl
 *      Device handle.
 *
 * @return 
 *      Succeed: 0. \n 
 *      Fail:    -1.
 */
int  Ql_AudPlayer_Pause(int hdl);

/**
 * @brief Resume from pause state
 *
 * @param hdl
 *      Device handle.
 *
 * @return 
 *      Succeed: 0. \n 
 *      Fail:    -1.
 */
int  Ql_AudPlayer_Resume(int hdl);

/**
 * @brief Stop playing
 * @details
 *      Just set stop flag, and playing progress will end itself.
 *
 * @param hdl
 *      Device handle.
 */
void Ql_AudPlayer_Stop(int hdl);

/**
 * @brief  Close player device.
 * @details
 *      This functions will close player device if it was opened. \n
 *      If the playing progress is still on, this function will call
 *      Ql_AudPlayer_Stop(hdl) first and block to wait playing progress
 *      end.
 *
 * @param hdl
 *      Device handle.
 */
void Ql_AudPlayer_Close(int hdl);

// int Ql_AudPlayer_set_LessDataThreshold(int hdl, unsigned short threshSize);

// int Ql_AudPlayer_get_freeSpace(int hdl);

/**
 * @brief Open recorder device.
 * @details
 *      Just call function Ql_AudRecorder_OpenExt(device, cb_func,
 *                               PCM_NMMAP, 1, 8000, SNDRV_PCM_FORMAT_S16_LE); \n
 *      Set default parameters MONO, 8khz samplerate, 16bit little endian.
 *
 * @param device
 *      Pcm device, eg: "hw:0,0"
 * @param cb_func
 *      User callback functions.
 *
 * @return 
 *      Succeed: recorder device handle. \n
 *      Fail:    -1.
 */
int  Ql_AudRecorder_Open(char* device, _cb_onRecorder cb_fun);

/**
 * @brief Start to record
 * @details
 *      Just call Ql_AudRecorder_HStartRecord(0);
 *
 * @return 
 *      Succeed: recorder device handle. \n
 *      Fail:    -1.
 */
int Ql_AudRecorder_StartRecord(void);

/**
 * @brief Pause recording (not support now)
 * @details
 *      Just call Ql_AudRecorder_HPause(0).
 *
 * @return 
 *      Succeed: 0. \n
 *      Fail:    -1.
 */
int Ql_AudRecorder_Pause(void);

/**
 * @brief Resume recording (not support now)
 * @details
 *      Just call Ql_AudRecorder_HResume(0).
 *
 * @return 
 *      Succeed: 0. \n
 *      Fail:    -1.
 */
int Ql_AudRecorder_Resume(void);

/**
 * @brief Stop recording
 * @details
 *      Just call Ql_AudRecorder_Stop(0).
 *
 */
void Ql_AudRecorder_Stop(void);

/**
 * @brief Close recorder device, and free the resources.
 * @details
 *      Just call Ql_AudRecorder_Close(0).
 */
void Ql_AudRecorder_Close(void);


typedef enum {
     HP_PATH_SET,    //hp out
     HP_LEFT_SET,   //  some codec have left hp and right hp
     HP_RIGHT_SET,
     SPK_PATH_SET,  //spk path
     LINEOUT_PATH_SET, //line out path
     AUX_PATH_SET,    // aux path
     LINEIN1_PATH,   //linein path
     LINEIN2_PATH_SET,
     LINEIN3_PATH_SET,
     MIC1_PATH_SET,    //mic path
     MIC2_PATH_SET,
     MIC3_PATH_SET,
     DIGHT_MIC_SET,  // dight path

}CODEC_PATH_E;
int ql_codec_path_set(CODEC_PATH_E path,int enable);

int ql_codec_volume(CODEC_PATH_E path,int volume);
int Ql_AudStream_Enable(int enable);

int Ql_Ext_Codec_Control(int enable);
int Ql_Record_Codec_Enable(void);
void Ql_Record_Codec_Disable(void);
int Ql_Playback_Codec_Enable(void);
void Ql_Playback_Codec_Disable(void);
int Ql_Handfree_Loopback_Enable(void);
int Ql_Hand_Shank_Loopback_Enable(void);
int Ql_Handfree_Loopback_Disable(void);
int Ql_Hand_Shank_Loopback_Disable(void);

int Ql_TTS_Play(int mode,char *str);
int Ql_TTS_Speed_Set(int speed);

/**
 * @brief Set RX DSP Gain
 * @details
 *      Gain support [-36,12] dB
 *
 * @param gain
 *      DSP gain
 */

int Ql_Rxgain_Set(int gain);

/**
 * @brief Set TX DSP Gain
 * @details
 *      Gain support [-36,12] dB
 *
 * @param gain
 *      DSP gain
 */

int Ql_Txgain_Set(int gain);


#endif/*__QL_AUDIO_H__*/
