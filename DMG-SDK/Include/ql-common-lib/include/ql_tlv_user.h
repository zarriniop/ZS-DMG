/*******************************************************************************
 *
 *  Filename: ql_tlv_user.h
 *
 *  Description: flash tlv format param config header file.
 *
 *  When        Who         Why
 *  2019/09/28  juson       Creation of file
 *
 *  Notes:
 *
 ******************************************************************************/
#ifndef __QL_TLV_USER_H__
#define __QL_TLV_USER_H__

typedef enum {
    SYS_CONFIG_START = 0,
    SYS_CONFIG_AP_RSTLEVEL_ID,
    SYS_CONFIG_MODEM_RSTLEVEL_ID,
    SYS_CONFIG_RECOVERY_ID,
    SYS_CONFIG_CONSOLE_LOG_ID,
    SYS_CONFIG_END,
}SYS_CFG_ID_E;

/* 128bytes */
typedef struct ql_tlv_struct {
    SYS_CFG_ID_E type;
    unsigned int len;
    char value[120];
}ql_tlv_t;

int quectel_tlv_write(SYS_CFG_ID_E param_id, ql_tlv_t tlv_data);
int quectel_tlv_read(SYS_CFG_ID_E param_id, ql_tlv_t *tlv_data);
int ql_onoff_console_log(unsigned char onoff_flag);

#endif /* __QL_TLV_USER_H__ */


