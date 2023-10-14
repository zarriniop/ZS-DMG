/**
 * @file ql_wifi.h
 * @brief Quectel Openlinux WiFi public function declarations.
 * 
 * @note 
 *
 * @copyright Copyright (c) 2009-2017 @ Quectel Wireless Solutions Co., Ltd.
 */
/*================================================================
  Copyright (c) 2018 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/

/*=====================================================================
  
                         EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  WHEN             WHO         WHAT, WHERE, WHY
  ------------     -------     ----------------------------------------
  11/20/2011       Paddy        Initial creation.

=======================================================================*/

#ifndef __QL_WIFI_H__
#define __QL_WIFI_H__

#include <stdbool.h>
#include <netinet/in.h>

#define QL_WIFI_AP_AUTH_OPEN                  "none"
#define QL_WIFI_AP_AUTH_WPA_PSK               "psk"
#define QL_WIFI_AP_AUTH_WPA2_PSK              "psk2"
#define QL_WIFI_AP_AUTH_WPA_WPA2_PSK          "psk-mixed"

#define QL_WIFI_AP_AUTH_WPA_PAIRWISE_TKIP     "tkip"
#define QL_WIFI_AP_AUTH_WPA_PAIRWISE_AES      "aes"
#define QL_WIFI_AP_AUTH_WPA_PAIRWISE_BOTH     "tkip+aes"

#define QL_WIFI_AP_BANDWIDTH_20MHZ             "HT20"
#define QL_WIFI_AP_BANDWIDTH_40MHZ             "HT40-"
#define QL_WIFI_AP_BANDWIDTH_80MHZ             "HT40+"

#define QL_WIFI_AP_MODE_80211B                "11b"
#define QL_WIFI_AP_MODE_80211BG               "11bg"
#define QL_WIFI_AP_MODE_80211BGN              "11bgn"


typedef enum {
	WIFI_WORK_MODE_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.	Do not change or use*/
	WIFI_WORK_MODE_AP_V01 = 0, 
	WIFI_WORK_MODE_STA_V01 = 1, 
	WIFI_WORK_MODE_AP_STA_V01 = 2, 
	WIFI_WORK_MODE_AP_AP_V01 = 3, 
	WIFI_WORK_MODE_MAX_V01 = 4, 
	WIFI_WORK_MODE_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wifi_work_mode_e_v01;
	
typedef enum {
	WIFI_IEEE80211_MODE_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
	WIFI_MODE_11A_V01 = 0, 
	WIFI_MODE_11AN_V01 = 1, 
	WIFI_MODE_11B_V01 = 2, 
	WIFI_MODE_11BG_V01 = 3, 
	WIFI_MODE_11BGN_V01 = 4, 
	WIFI_MODE_11AC_V01 = 5, 
	WIFI_IEEE80211_MODE_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wifi_ieee80211_mode_e_v01;;


	
typedef enum {
	WIFI_BANDWIDTH_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
	WIFI_BANDWIDTH_20MHZ_V01 = 0, 
	WIFI_BANDWIDTH_40MHZ_V01 = 1, 
	WIFI_BANDWIDTH_80MHZ_V01 = 2, 
	WIFI_BANDWIDTH_160MHZ_V01 = 3, 
	WIFI_BANDWIDTH_MAX_V01 = 4, 
	WIFI_BANDWIDTH_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wifi_bandwidth_e_v01;

typedef enum {
	WIFI_AUTH_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
	WIFI_AUTH_OPEN_V01 = 0, 
	WIFI_AUTH_WEP_V01 = 1, 
	WIFI_AUTH_WPA_PSK_V01 = 2, 
	WIFI_AUTH_WPA2_PSK_V01 = 3, 
	WIFI_AUTH_WPA_WPA2_PSK_BOTH_V01 = 4, 
	WIFI_AUTH_WPA_V01 = 5, 
	WIFI_AUTH_WPA2_V01 = 6, 
	WIFI_AUTH_WPA_WPA2_BOTH_V01 = 7, 
	WIFI_AUTH_WPS_V01 = 8, 
	WIFI_AUTH_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wifi_auth_e_v01;
/**
    @}
  */

/** @addtogroup ql_manager_access_msgr_wifi_qmi_enums
    @{
  */
typedef enum {
	WIFI_WPA_PAIRWISE_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
	WIFI_AUTH_WPA_PAIRWISE_TKIP_V01 = 0, 
	WIFI_AUTH_WPA_PAIRWISE_AES_V01 = 1, 
	WIFI_AUTH_WPA_PAIRWISE_BOTH_V01 = 2, 
	WIFI_WPA_PAIRWISE_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wifi_wpa_pairwise_e_v01;

typedef enum {
    QL_WIFI_EVENT_ENABLE_STATUS = 0,
    QL_WIFI_EVENT_DISABLE_STATUS,
    QL_WIFI_EVENT_AP_STATION,
    QL_WIFI_EVENT_STA_STATUS,
    QL_WIFI_EVENT_STA_SCAN_DONE,
} ql_wifi_event_type_e;

typedef enum {
    QL_WIFI_STATUS_ENABLE = 0,
    QL_WIFI_STATUS_DISABLE,
    QL_WIFI_STATUS_ERR_DRIVER,
    QL_WIFI_STATUS_ERR_SOFTWARE,
} ql_wifi_status_e;

typedef enum {
    QL_WIFI_STATION_DISABLE = 0,    /* WiFi station is not enable */
    QL_WIFI_STATION_CONNECTED,      /* WiFi station is connected hotspot */
    QL_WIFI_STATION_DISCONNECTED    /* WiFi station is disconnect hotspot */
} ql_wifi_station_status_e;

typedef struct {
    ql_wifi_event_type_e id;
    union {
        /* the QL_WIFI_ENABLE_STATUS or QL_WIFI_DISABLE_STATUS event */
        ql_wifi_status_e status;
        struct {
            /* 
             * If connected is true, the station is connected wifi hotspot.
             * If connected is false, the station is disconnected wifi hotspot.
             */
            bool connected;
            unsigned char mac[6];
        } ap_sta_info;
        ql_wifi_station_status_e sta_status;
    };
} ql_wifi_event_s;

typedef void (*wifi_event_handle)(ql_wifi_event_s *event, void *arg);

typedef enum {
    QL_WIFI_WORK_MODE_STA = 0,      /* WiFi is in STA Mode */
    QL_WIFI_WORK_MODE_AP0,          /* WiFi is in AP Mode */
    QL_WIFI_WORK_MODE_AP0_STA,      /* WiFi is in AP/STA Mode */
    QL_WIFI_WORK_MODE_AP0_AP1       /* WiFi is in AP/AP Mode */
} ql_wifi_work_mode_e;

typedef enum {
    QL_WIFI_AP_INDEX_AP0 = 0,       /* Index 0 of AP/AP-STA/AP-AP Mode */
    QL_WIFI_AP_INDEX_AP1            /* Index 1 of AP/AP-STA/AP-AP Mode */
} ql_wifi_ap_index_e;

typedef enum {
    QL_WIFI_MODE_80211B = 0,        /* IEEE 802.11b (2.4 GHz) */
    QL_WIFI_MODE_80211BG,           /* IEEE 802.11bg (2.4 GHz) */
    QL_WIFI_MODE_80211BGN,          /* IEEE 802.11bgn (2.4 GHz) */
    QL_WIFI_MODE_80211A,            /* IEEE 802.11a (5 GHz) */
    QL_WIFI_MODE_80211AN,           /* IEEE 802.11an (5 GHz) */
    QL_WIFI_MODE_80211AC            /* IEEE 802.11ac (5 GHz) */
} ql_wifi_mode_type_e;

typedef enum {
    QL_WIFI_BANDWIDTH_HT20 = 0,    
    QL_WIFI_BANDWIDTH_HT40,
    QL_WIFI_BANDWIDTH_HT80
} ql_wifi_bandwidth_type_e;

typedef enum {
    QL_WIFI_SSID_HIDE_DISABLE = 0,    
    QL_WIFI_SSID_HIDE_ENABLE,
} ql_wifi_ssidhide_type_e;

typedef enum {
    QL_WIFI_AUTH_OPEN = 0,             /* Open */
    QL_WIFI_AUTH_WPA_PSK,              /* WPA Personal */
    QL_WIFI_AUTH_WPA2_PSK,             /* WPA2 Personal */
    QL_WIFI_AUTH_WPA_WPA2_PSK_BOTH,    /* WPA&WPA2 Personal */
} ql_wifi_auth_e;

typedef enum {
    QL_WIFI_AUTH_WPA_PAIRWISE_AUTO = 0,    /* contain TKIP and AES */
    QL_WIFI_AUTH_WPA_PAIRWISE_TKIP,
    QL_WIFI_AUTH_WPA_PAIRWISE_AES
} ql_wifi_auth_wpa_psk_e;

typedef struct {
    int auth;
    union {
        struct {
            int default_index;
            char passwd[4][64];
        } wep;

        struct {
            short pairwise;
            char passwd[64];
            int group_rekey;
        } wpa_psk;
    };
} ql_wifi_ap_auth_s;


typedef enum {
	WIFI_IDX_AP_0 = 1,
	WIFI_IDX_AP_1,
	WIFI_IDX_STA_0
} wifi_index_e;

typedef enum {
	WIFI_WORK_MODE_AP = 0,
	WIFI_WORK_MODE_STA,
	WIFI_WORK_MODE_AP_STA,
	WIFI_WORK_MODE_AP_AP,
	WIFI_WORK_MODE_MAX
} wifi_work_mode_e;

typedef enum {
	WIFI_START = 0,
	WIFI_STOP,
	WIFI_RESTART,
} wifi_active_e;

typedef enum {
	WIFI_MODE_11A = 0,
	WIFI_MODE_11AN,
	WIFI_MODE_11B,
	WIFI_MODE_11BG,
	WIFI_MODE_11BGN,
	WIFI_MODE_11AC
} wifi_ieee80211_mode_e;

typedef enum {
	WIFI_BANDWIDTH_20MHZ = 0,
	WIFI_BANDWIDTH_40MHZ,
	WIFI_BANDWIDTH_80MHZ,
	WIFI_BANDWIDTH_160MHZ,
	WIFI_BANDWIDTH_MAX,
} wifi_bandwidth_e;

typedef enum {
	WIFI_AUTH_OPEN = 0,
	WIFI_AUTH_WEP,
	WIFI_AUTH_WPA_PSK,
	WIFI_AUTH_WPA2_PSK,
	WIFI_AUTH_WPA_WPA2_PSK_BOTH,
	WIFI_AUTH_WPA,
	WIFI_AUTH_WPA2,
	WIFI_AUTH_WPA_WPA2_BOTH,
	WIFI_AUTH_WPS
} wifi_auth_e;

typedef enum {
	WIFI_AUTH_WPA_PAIRWISE_TKIP = 0,
	WIFI_AUTH_WPA_PAIRWISE_AES,
	WIFI_AUTH_WPA_PAIRWISE_BOTH
} wifi_wpa_pairwise_e;

typedef enum {
	WIFI_SSID_HIDE_DISABLE = 0,
	WIFI_SSID_HIDE_ENABLE
} wifi_ssid_hide_state_e;


struct _auth {
	unsigned char auth;
	struct {
		unsigned char pairwise;
		unsigned char group_rekey;
		unsigned char passwd[64];
	} wpa_psk;
	
};


typedef enum {
	WIFI_INDEX_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
	WIFI_IDX_AP_0_V01 = 1, 
	WIFI_IDX_AP_1_V01 = 2, 
	WIFI_IDX_STA_0_V01 = 3, 
	WIFI_INDEX_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wifi_index_e_v01;

typedef enum {
	RTK_WIFI_IEEE80211_MODE_E_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
	RTK_WIFI_WORKMODE_11B_V01 = 1, 
	RTK_WIFI_WORKMODE_11BG_V01 = 3, 
	RTK_WIFI_WORKMODE_11BGN_V01 = 11, 
	RTK_WIFI_IEEE80211_MODE_E_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}rtk_wifi_ieee80211_mode_e_v01;

typedef enum {
    QL_WIFI_AUTH_OPEN_V01 = 0,             /* Open */
    QL_WIFI_SHARED_KEY,              /* Shared key */
    QL_WIFI_AUTO                   /* Auto */
} ql_wifi_auth_type_e;

typedef enum {
    QL_WIFI_ENCRYPT_TKIP = 2,           /* TKIP */
    QL_WIFI_ENCRYPT_AES = 8,        /* AES */
    QL_WIFI_ENCRYPT_TKIP_AES_MIXED = 10     /* TKIP & AES MIXED */
} ql_wifi_encrypt_mode_e;

typedef enum {
    QL_WIFI_PSK_DISABLE = 0,    /* Encrypt disable */
    QL_WIFI_PSK_WPA,            /* WPA */
    QL_WIFI_PSK_WPA2,           /* WPA2 */
    QL_WIFI_PSK_WPA_WPA2,       /* WPA-WPA2 */
} ql_wifi_psk_enable_e;



#define QL_WIFI_MAX_STA_NUM 16
	
	struct ql_mgmt_client_wifi_work_mode {
		/*
		 * WiFi work mode support AP, STA, AP+STA, AP-AP
		 */
		wifi_work_mode_e work_mode;
	};
	
	struct ql_mgmt_client_wifi_enable {
		bool state;
	};
	
	struct ql_mgmt_client_wifi_country_code {
		char country_code[2];
	};
	
	struct ql_mgmt_client_wifi_mode {
		wifi_index_e id;
		wifi_ieee80211_mode_e mode;
	};
	
	struct ql_mgmt_client_wifi_channel {
		wifi_index_e id;
		int channel;
	};
	
	struct ql_mgmt_client_wifi_essid {
		wifi_index_e id;
		char ssid[32];
	};
	
	struct ql_mgmt_client_wifi_ssid_hide {
		wifi_index_e id;
		wifi_ssid_hide_state_e state;
	};
	
	struct ql_mgmt_client_wifi_bandwidth {
		wifi_index_e id;
		wifi_bandwidth_e bandwidth;
	};
	
	struct ql_mgmt_client_wifi_max_sta {
		wifi_index_e id;
		int num;
	};

	
	struct ql_mgmt_client_wifi_active {
		wifi_index_e_v01 id;
		unsigned char action_type;
	};
	
	struct ql_mgmt_client_wifi_auth {
		wifi_index_e id;
		wifi_auth_e auth;
		union {
			struct {
				int default_index;
				char password[4][64];
			} wep;
			struct {
				wifi_wpa_pairwise_e pairwise;
				char password[64];
				int group_rekey;
			} wpa_psk;
		};
	};
	
	struct ql_mgmt_client_wifi_rssi {
		wifi_index_e id;
		int rssi;
	};
	
	struct ql_mgmt_client_wifi_sta_info {
		int count;
		struct {
			int ip;
			char mac[6];
			char hostname[32];
			char ssid[32];
			int uptime;
			long long tx_bytes;
			long long rx_bytes;
		} sta[QL_WIFI_MAX_STA_NUM];
	};
	typedef struct{
		struct ql_mgmt_client_wifi_enable enable;
	
	
	}ql_mgmt_client_wifi_s_test;
	
	typedef struct {
		union {
			struct ql_mgmt_client_wifi_work_mode work_mode;
			struct ql_mgmt_client_wifi_enable enable;
			struct ql_mgmt_client_wifi_country_code country_code;
			struct ql_mgmt_client_wifi_mode mode;
			struct ql_mgmt_client_wifi_channel channel;
			struct ql_mgmt_client_wifi_essid ssid;
			struct ql_mgmt_client_wifi_ssid_hide ssid_hide;
			struct ql_mgmt_client_wifi_bandwidth bandwidth;
			struct ql_mgmt_client_wifi_max_sta max_sta;
			struct ql_mgmt_client_wifi_active active;
			struct ql_mgmt_client_wifi_auth auth;
			struct ql_mgmt_client_wifi_rssi rssi;
			struct ql_mgmt_client_wifi_sta_info sta_info;
		};
	} ql_mgmt_client_wifi_s;


extern int ql_wifi_enable(void);

/**
 * Disables the WLAN. This function removes the kernel module for the Wi-Fi driver.
 *
 * @param None
 *
 * @return 
 *   On success, 0 is returned.  On error, -1 is returned.
 * 
 */
extern int ql_wifi_disable(void);

/**
 * Create thread to enables the WLAN. This function enables the WLAN based on the hostpad or wpa_supplicant 
 * configuration provided. This function inserts the WLAN kernel module and advertises the SSID.
 * 
 * @param None
 *
 * @return 
 *   On success, 0 is returned.  On error, -1 is returned.
 * 
 */


extern int ql_wifi_status_get(ql_wifi_status_e *status);

/**
 * Set the WiFi configuration: WiFi work mode configuration.
 *
 * @param [in]   WiFi work mode
 *
 * @return 
 *   On success, 0 is returned.  On error, -1 is returned.
 * 
 * @dependencies
 *   ql_wifi_disable is must be disable
 *
 */

extern int ql_wifi_ap_ssid_set(ql_wifi_ap_index_e idx, char *ssid);

/**
 * Get the WiFi configuration: the hotspot ssid mode configuration.
 *
 * @param [in]    Index of AP mode
 * @param [out]   the hotspot ssid name
 *
 * @return 
 *   On success, 0 is returned.  On error, -1 is returned.
 * 
 */
extern int ql_wifi_ap_ssid_get(ql_wifi_ap_index_e idx, char *ssid);

/**
 * Set the WiFi configuration: hidden hotspot configuration.
 *
 * @param [in]    Index of AP mode
 * @param [in]    If set true, hidden hotspot
 *                If set false, open hotspot
 *
 * @return 
 *   On success, 0 is returned.  On error, -1 is returned.
 * 
 */
extern int ql_wifi_ap_ssid_hide_set(ql_wifi_ap_index_e idx, bool hide);

/**
 * Get the WiFi configuration: hidden hotspot configuration.
 *
 * @param [in]    Index of AP mode
 * @param [in]    Get the hotspot hidden status
 *
 * @return 
 *   On success, 0 is returned.  On error, -1 is returned.
 * 
 */
extern int ql_wifi_ap_ssid_hide_get(ql_wifi_ap_index_e idx, bool *hide);

/**
 * Set the WiFi configuration: Operation mode configuration, such as 802.11a/b/n/ac.
 *
 * @param [in]   Index of AP mode
 * @param [in]   Operation mode
 *
 * @return 
 *   On success, 0 is returned.  On error, -1 is returned.
 * 
 */
extern int ql_wifi_ap_mode_set(ql_wifi_ap_index_e idx, ql_wifi_mode_type_e mode);

/**
 * Get the WiFi configuration: Operation mode configuration, such as 802.11a/b/n/ac.
 *
 * @param [in]    Index of AP mode
 * @param [out]   Operation mode
 *
 * @return 
 *   On success, 0 is returned.  On error, -1 is returned.
 * 
 */
extern int ql_wifi_ap_mode_get(ql_wifi_ap_index_e idx, ql_wifi_mode_type_e *mode);

/**
 * Set the WiFi configuration: HT or VHT capabilities (20MHz, 40MHz, 80MHz).
 *
 * @param [in]   Index of AP mode
 * @param [in]   Bandwidth
 *
 * @return 
 *   On success, 0 is returned.  On error, -1 is returned.
 * 
 * @dependencies
 *   The WiFi Operation mode must be set 802.11bgn/802.11an/802.11ac.
 */
extern int ql_wifi_ap_bandwidth_set(ql_wifi_ap_index_e idx, ql_wifi_bandwidth_type_e bandwidth);

/**
 * Get the WiFi configuration: HT or VHT capabilities (20MHz, 40MHz, 80MHz).
 *
 * @param [in]    Index of AP mode
 * @param [Out]   Bandwidth
 *
 * @return 
 *   On success, 0 is returned.  On error, -1 is returned.
 * 
 * @dependencies
 *   The WiFi Operation mode must be set 802.11bgn/802.11an/802.11ac.
 */
extern int ql_wifi_ap_bandwidth_get(ql_wifi_ap_index_e idx, ql_wifi_bandwidth_type_e *bandwidth);

/**
 * Set the WiFi configuration: channel number configuration.
 *
 * @param [in]   Index of AP mode
 * @param [in]   Channel number
 *
 * @return 
 *   On success, 0 is returned.  On error, -1 is returned.
 * 
 * @dependencies
 *   1. if WiFi set 2.4GHz, the channel list: 
 *   0/1/2/3/4/5/6/7/8/9/10/11/12/13/14
 *   2. if WiFi set 5GHz, the channel list:
 *   36/40/44/48/52/56/60/64/100/104/108/112/116/120/124/128/132/136/140/144/149/153/157/161/165/175/181 
 */
extern int ql_wifi_ap_channel_set(ql_wifi_ap_index_e idx, int channel);

/**
 * Get the WiFi configuration: channel number configuration.
 *
 * @param [in]    Index of AP mode
 * @param [out]   Channel number
 *
 * @return 
 *   On success, 0 is returned.  On error, -1 is returned.
 * 
 * @dependencies
 *   1. if WiFi set 2.4GHz, the channel list: 
 *   0/1/2/3/4/5/6/7/8/9/10/11/12/13/14
 *   2. if WiFi set 5GHz, the channel list:
 *   36/40/44/48/52/56/60/64/100/104/108/112/116/120/124/128/132/136/140/144/149/153/157/161/165/175/181 
 */
extern int ql_wifi_ap_channel_get(ql_wifi_ap_index_e idx, int *channel);

/**
 * Set the WiFi configuration: IEEE 802.11 specifies two authentication algorithms and WPA/IEEE 802.11i configuration.
 *
 * @param [in]   Index of AP mode
 * @param [in]   authentication configuration
 *
 * @return 
 *   On success, 0 is returned.  On error, -1 is returned.
 * 
 */
extern int ql_wifi_ap_auth_set(ql_wifi_ap_index_e idx, ql_wifi_ap_auth_s *auth);

/**
 * Get the WiFi configuration: IEEE 802.11 specifies two authentication algorithms and WPA/IEEE 802.11i configuration.
 *
 * @param [in]   Index of AP mode
 * @param [out]   authentication configuration
 *
 * @return 
 *   On success, 0 is returned.  On error, -1 is returned.
 * 
 */
extern int ql_wifi_ap_auth_get(ql_wifi_ap_index_e idx, ql_wifi_ap_auth_s *auth);

/**
 * Set the WiFi configuration: Maximum number of stations allowed in station table.
 *
 * @param [in]   Index of AP mode
 * @param [in]   Maximum number of stations
 *
 * @return 
 *   On success, 0 is returned.  On error, -1 is returned.
 * 
 */
extern int ql_wifi_ap_max_sta_set(ql_wifi_ap_index_e idx, int max_sta_num);

/**
 * Get the WiFi configuration: Maximum number of stations allowed in station table.
 *
 * @param [in]    Index of AP mode
 * @param [out]   Maximum number of stations
 *
 * @return 
 *   On success, 0 is returned.  On error, -1 is returned.
 * 
 */
extern int ql_wifi_ap_max_sta_get(ql_wifi_ap_index_e idx, int *max_sta_num);

extern int ql_wifi_set_to_ap(ql_wifi_ap_index_e idx);

extern int ql_wifi_set_to_sta(ql_wifi_ap_index_e idx);

#endif /* end of __QL_WIFI_H__ */

