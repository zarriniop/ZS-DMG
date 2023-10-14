append DRIVERS "realtek"

set_open() {
	echo "0" > $CONFIG_ROOT_DIR/$1/macclone_enable

	echo "0" > $CONFIG_ROOT_DIR/$1/encrypt
	echo "0" > $CONFIG_ROOT_DIR/$1/wep
	echo "0" > $CONFIG_ROOT_DIR/$1/wep_default_key
	echo "1" > $CONFIG_ROOT_DIR/$1/wep_key_type
	echo "2" > $CONFIG_ROOT_DIR/$1/auth_type

	echo "0" > $CONFIG_ROOT_DIR/$1/wsc_configured
	echo "1" > $CONFIG_ROOT_DIR/$1/wsc_auth
	echo "1" > $CONFIG_ROOT_DIR/$1/wsc_enc    

	if [ "$RP" = "0" ]; then
		echo 0 > $CONFIG_ROOT_DIR/repeater_enabled
	fi	
}

set_wep() {
	# wep_key_type - 0: hex mode 1: ascii
	# wep : 1: wep-64 2: wep-128
	echo "1" > $CONFIG_ROOT_DIR/$1/encrypt
	echo $wep_type > $CONFIG_ROOT_DIR/$1/wep
	echo $wep_key_idx > $CONFIG_ROOT_DIR/$1/wep_default_key
	echo $wep_key_type > $CONFIG_ROOT_DIR/$1/wep_key_type
	echo "2" > $CONFIG_ROOT_DIR/$1/auth_type

	echo $wep64_h_key1 > $CONFIG_ROOT_DIR/$1/wepkey1_64_hex
	echo $wep64_h_key2 > $CONFIG_ROOT_DIR/$1/wepkey2_64_hex
	echo $wep64_h_key3 > $CONFIG_ROOT_DIR/$1/wepkey2_64_hex
	echo $wep64_h_key4 > $CONFIG_ROOT_DIR/$1/wepkey3_64_hex

	echo $wep64_a_key1 > $CONFIG_ROOT_DIR/$1/wepkey1_64_asc
	echo $wep64_a_key2 > $CONFIG_ROOT_DIR/$1/wepkey2_64_asc
	echo $wep64_a_key3 > $CONFIG_ROOT_DIR/$1/wepkey2_64_asc
	echo $wep64_a_key4 > $CONFIG_ROOT_DIR/$1/wepkey3_64_asc

	echo $wep128_h_key1 > $CONFIG_ROOT_DIR/$1/wepkey1_128_hex
	echo $wep128_h_key2 > $CONFIG_ROOT_DIR/$1/wepkey2_128_hex
	echo $wep128_h_key3 > $CONFIG_ROOT_DIR/$1/wepkey2_128_hex
	echo $wep128_h_key4 > $CONFIG_ROOT_DIR/$1/wepkey3_128_hex

	echo $wep128_a_key1 > $CONFIG_ROOT_DIR/$1/wepkey1_128_asc
	echo $wep128_a_key2 > $CONFIG_ROOT_DIR/$1/wepkey2_128_asc
	echo $wep128_a_key3 > $CONFIG_ROOT_DIR/$1/wepkey2_128_asc
	echo $wep128_a_key4 > $CONFIG_ROOT_DIR/$1/wepkey3_128_asc

	echo "1" > $CONFIG_ROOT_DIR/$1/wsc_configured
	echo "1" > $CONFIG_ROOT_DIR/$1/wsc_auth
	echo "2" > $CONFIG_ROOT_DIR/$1/wsc_enc
	echo "0" > $CONFIG_ROOT_DIR/$1/wsc_configbyextreg
}

set_wpa_tkip() {
	echo "2" > $CONFIG_ROOT_DIR/$1/encrypt
	echo "0" > $CONFIG_ROOT_DIR/$1/enable_1x
	echo "2" > $CONFIG_ROOT_DIR/$1/wpa_auth
	echo "1" > $CONFIG_ROOT_DIR/$1/wpa_cipher
	echo $wpa_psk > $CONFIG_ROOT_DIR/$1/wpa_psk

	echo "1" > $CONFIG_ROOT_DIR/$1/wsc_configured
	echo "2" > $CONFIG_ROOT_DIR/$1/wsc_auth
	echo "4" > $CONFIG_ROOT_DIR/$1/wsc_enc
	echo "0" > $CONFIG_ROOT_DIR/$1/wsc_configbyextreg
	echo $wpa_psk > $CONFIG_ROOT_DIR/$1/wsc_psk
}

set_wpa_aes() {
	echo "2" > $CONFIG_ROOT_DIR/$1/encrypt
	echo "0" > $CONFIG_ROOT_DIR/$1/enable_1x
	echo "2" > $CONFIG_ROOT_DIR/$1/wpa_auth
	echo "2" > $CONFIG_ROOT_DIR/$1/wpa_cipher
	echo $wpa_psk > $CONFIG_ROOT_DIR/$1/wpa_psk

	echo "1" > $CONFIG_ROOT_DIR/$1/wsc_configured
	echo "2" > $CONFIG_ROOT_DIR/$1/wsc_auth
	echo "8" > $CONFIG_ROOT_DIR/$1/wsc_enc
	echo "0" > $CONFIG_ROOT_DIR/$1/wsc_configbyextreg
	echo $wpa_psk > $CONFIG_ROOT_DIR/$1/wsc_psk
}

set_wpa2_tkip () {
	echo "4" > $CONFIG_ROOT_DIR/$1/encrypt
	echo "0" > $CONFIG_ROOT_DIR/$1/enable_1x
	echo "2" > $CONFIG_ROOT_DIR/$1/wpa_auth
	echo "1" > $CONFIG_ROOT_DIR/$1/wpa2_cipher
	echo $wpa_psk > $CONFIG_ROOT_DIR/$1/wpa_psk

	echo "1" > $CONFIG_ROOT_DIR/$1/wsc_configured
	echo "32" > $CONFIG_ROOT_DIR/$1/wsc_auth
	echo "4" > $CONFIG_ROOT_DIR/$1/wsc_enc
	echo "0" > $CONFIG_ROOT_DIR/$1/wsc_configbyextreg
	echo $wpa_psk > $CONFIG_ROOT_DIR/$1/wsc_psk
}

set_wpa2_aes () {
	echo "4" > $CONFIG_ROOT_DIR/$1/encrypt
	echo "0" > $CONFIG_ROOT_DIR/$1/enable_1x
	echo "2" > $CONFIG_ROOT_DIR/$1/wpa_auth
	echo "2" > $CONFIG_ROOT_DIR/$1/wpa2_cipher
	echo $wpa_psk > $CONFIG_ROOT_DIR/$1/wpa_psk

	echo "1" > $CONFIG_ROOT_DIR/$1/wsc_configured
	echo "32" > $CONFIG_ROOT_DIR/$1/wsc_auth
	echo "8" > $CONFIG_ROOT_DIR/$1/wsc_enc
	echo "0" > $CONFIG_ROOT_DIR/$1/wsc_configbyextreg
	echo $wpa_psk > $CONFIG_ROOT_DIR/$1/wsc_psk
}

set_mixed() {
	echo "6" > $CONFIG_ROOT_DIR/$1/encrypt
	echo "0" > $CONFIG_ROOT_DIR/$1/enable_1x
	echo "2" > $CONFIG_ROOT_DIR/$1/wpa_auth
	echo "3" > $CONFIG_ROOT_DIR/$1/wpa_cipher
	echo "3" > $CONFIG_ROOT_DIR/$1/wpa2_cipher
	echo $wpa_psk > $CONFIG_ROOT_DIR/$1/wpa_psk

	echo "1" > $CONFIG_ROOT_DIR/$1/wsc_configured
	echo "34" > $CONFIG_ROOT_DIR/$1/wsc_auth
	echo "12" > $CONFIG_ROOT_DIR/$1/wsc_enc
	echo "0" > $CONFIG_ROOT_DIR/$1/wsc_configbyextreg
	echo $wpa_psk > $CONFIG_ROOT_DIR/$1/wsc_psk
}

set_sae() {
	echo "8" > $CONFIG_ROOT_DIR/$1/encrypt
	echo "2" > $CONFIG_ROOT_DIR/$1/authtype
	echo "2" > $CONFIG_ROOT_DIR/$1/encmode
	echo "0" > $CONFIG_ROOT_DIR/$1/wpa_cipher
	echo "8" > $CONFIG_ROOT_DIR/$1/wpa2_cipher
	echo $wpa_psk > $CONFIG_ROOT_DIR/$1/wpa_psk
}

set_sae_mixed() {
	echo "10" > $CONFIG_ROOT_DIR/$1/encrypt
	echo "2" > $CONFIG_ROOT_DIR/$1/authtype
	echo "2" > $CONFIG_ROOT_DIR/$1/encmode
	echo "0" > $CONFIG_ROOT_DIR/$1/wpa_cipher
	echo "8" > $CONFIG_ROOT_DIR/$1/wpa2_cipher
	echo $wpa_psk > $CONFIG_ROOT_DIR/$1/wpa_psk
}

set_enc() {
	# set_enc [enc] [intf]	
	set_open $2
	case $1 in
		open)
			echo "do nothing"
		;;
		wep)
			set_wep $2
		;;
		wpa_tkip)
			set_wpa_tkip $2
		;;
		wpa_aes)
			set_wpa_aes $2 
		;;
		wpa2_tkip)
			set_wpa2_tkip $2
		;;
		wpa2_aes)
			set_wpa2_aes $2 
		;;
		mixed)
			set_mixed $2
		;;
		sae-mixed)
			set_sae_mixed $2
		;;
		sae)
			set_sae $2
		;;
		802.1x)
			echo "not supported yet"     
		;;
		*)
			echo "usage: enc: open, wep, wpa_tkip, wpa2_aes, mixed"
		exit 1
	esac
}

start_nat() {
	EIF="eth0"
	IIF="br-lan"
	INNET="192.168.10.0/24"

	echo "1" > /proc/sys/net/ipv4/ip_forward
	iptables -F
	iptables -X
	iptables -t nat -F
	iptables -t nat -X 
	iptables -P INPUT DROP
	iptables -P OUTPUT ACCEPT
	iptables -P FORWARD ACCEPT
	iptables -A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT
	iptables -t nat -A POSTROUTING -o $EIF -s $INNET -j MASQUERADE
}

stop_nat() {
	iptables -F
	iptables -X
	iptables -t nat -F
	iptables -t nat -X
}

start_ap() {
	local __ssid

	if [ "$WPS" = "0" ]; then	
		for intf in $@; do
			echo "0" > $CONFIG_ROOT_DIR/$intf/wlan_mode
			set_enc $ENC $intf # [enc] [intf]

			NUM=${intf#w*va};
			echo "start my $NUM"
			#echo "$AP_SSID-$intf" > $CONFIG_ROOT_DIR/$intf/ssid
			eval "__ssid=\$ssid_${intf}"
			echo $__ssid > $CONFIG_ROOT_DIR/$intf/ssid
			echo $ap_channel > $CONFIG_ROOT_DIR/$intf/channel
		done
		 #$IFCONFIG $INTF hw ether $MACID
	fi

	rtl_act_wlan_if start $@ # all if

	if [ "$RP" = "0" ]; then
		rtl_br_lan_if start $@ $LAN_INTF #all if
		#$IFCONFIG $INTF hw ether $MACID
		if [ "$ADD_BR" = "1" ]; then
			$IFCONFIG $BR_INTF $LOCAL_IP up
		fi
		$START_WLAN_APP start $@ $BR_INTF 
		#start_nat
	fi
}
 
start_cli() {
	if [ "$WPS" = "0" ]; then	
		set_enc $1 $2

		# auto channel
		channel=0

		#wlan_mode=0 : AP
		#wlan_mode=1 : Client | Ad-hoc
		#               network_type=0 : Clien
		#               network_type=1 : Ad-hoc
		wlan_mode=1
		network_type=0

		#1:B, 2:G, 4:A, 8:N, 64:AC
		#band=11 : BGN
		band=11

		#phyBandSelect=1 : 2G
		#phyBandSelect=2 : 5G
		phyBandSelect=1

		echo $channel > $CONFIG_ROOT_DIR/$2/channel
		echo $wlan_mode > $CONFIG_ROOT_DIR/$2/wlan_mode
		echo $network_type > $CONFIG_ROOT_DIR/$2/network_type
		echo $STA_SSID > $CONFIG_ROOT_DIR/repeater_ssid
		echo $STA_SSID > $CONFIG_ROOT_DIR/$2/ssid
		echo $band > $CONFIG_ROOT_DIR/$2/band
		echo $phyBandSelect > $CONFIG_ROOT_DIR/$2/phyBandSelect

		#WPS_DISABLE=0
		#echo $WPS_DISABLE > $CONFIG_ROOT_DIR/$2/wpa_auth    
		#echo 0 > $CONFIG_ROOT_DIR/$2/wsc_enc
	fi

	#$IFCONFIG $INTF down    
	#$START_WLAN_ORG $INTF
	#$IFCONFIG $INTF hw ether $MACID
	#$IFCONFIG $INTF $LOCAL_IP up

	rtl_act_wlan_if start $2
	#$IWPRIV $2 set_mib wsc_enable=1
	if [ "$RP" = "0" ]; then
		#$IFCONFIG $2 hw ether $MACID
		rtl_br_lan_if start $2 $BR_INTF
		#$IWRPIV $INTF set_mib nat25_disable=1
		#$IFCONFIG $INTF hw ether $MACID
		if [ "$ADD_BR" = "1" ]; then
			$IFCONFIG $BR_INTF $LOCAL_IP up
		fi
		$START_WLAN_APP start $2 $BR_INTF
	fi

	#echo 1 > $CONFIG_ROOT_DIR/$2/wsc_enc
	#$IWPRIV $2 set_mib wsc_enable=1
	cat /var/run/wscd-wlan0.pid
}

start_rp() {
	echo 1 > $CONFIG_ROOT_DIR/repeater_enabled
	echo $AP_SSID > $CONFIG_ROOT_DIR/repeater_ssid

	ENC=$1
	VAP=0
	RP=1

	start_ap $ALL_AP_INTF
	#$IFCONFIG $INTF up
	VXD_INTF="$INTF-vxd"
	start_cli $1 $VXD_INTF
	$IFCONFIG $VXD_INTF up
	$IFCONFIG $INTF up
	rtl_br_lan_if start $VXD_INTF $INTF
	if [ "$ADD_BR" = "1" ]; then
		$IFCONFIG $BR_INTF $LOCAL_IP up
	fi
}

start_rp11() {
	echo 1 > $CONFIG_ROOT_DIR/repeater_enabled
	echo $AP_SSID > $CONFIG_ROOT_DIR/repeater_ssid

	VXD_INTF="$INTF-vxd"	
	start_cli $1 $VXD_INTF
	set_enc $1 $INTF
	wlan_mode=0
	network_type=0
	repeater_enabled=1

	echo $wlan_mode > $CONFIG_ROOT_DIR/$INTF/wlan_mode
	echo $network_type > $CONFIG_ROOT_DIR/$INTF/network_type
	echo $repeater_enabled > $CONFIG_ROOT_DIR/repeater_enabled
	echo $AP_SSID > $CONFIG_ROOT_DIR/$INTF/ssid

	rtl_act_wlan_if start $INTF
	$IFCONFIG $INTF 0.0.0.0 up
	$BRCTL addif $BR_INTF $INTF
}

start_rp1() {
	start_cli $1 $INTF

	VXD_INTF="$INTF-vxd"
	set_enc $1 $VXD_INTF

	wlan_mode=0
	network_type=0
	repeater_enabled=1

	echo $wlan_mode > $CONFIG_ROOT_DIR/$VXD_INTF/wlan_mode
	echo $network_type > $CONFIG_ROOT_DIR/$VXD_INTF/network_type
	echo $repeater_enabled > $CONFIG_ROOT_DIR/repeater_enabled
	echo $AP_SSID > $CONFIG_ROOT_DIR/$VXD_INTF/ssid

	rtl_act_wlan_if start $VXD_INTF
	MACID1="00:01:73:01:FF:11"
	MACID2="00:22:88:77:33:27"
	#$IFCONFIG $VXD_INTF hw ether $MACID1
	#iwpriv wlan0-vxd set_mib ssid="cliff-test"
	$IWPRIV $VXD_INTF set_mib ssid=$AP_SSID
	#rtl_br_lan_if start $ALL_RP_INTF
	#$IFCONFIG $BR_INTF $LOCAL_IP up
	$IFCONFIG $VXD_INTF 0.0.0.0 up 
	$BRCTL addif $BR_INTF $VXD_INTF
}

rtl_br_lan_if() {
	# func args: cmd lan0 lan1 ...
	# shutdown LAN interface (ethernt, wlan)

	if [ "$1" = "start" ]; then
		if [ "$ADD_BR" = "1" ]; then
			$BRCTL addbr $BR_INTF
			$IFCONFIG $BR_INTF up
		fi
	fi
	for ARG in $@ ; do
		#INTF=`$IFCONFIG $ARG`
		#echo "$1 $INTF"
		if [ "$ARG" = "start" ] || [ "$ARG" = "stop" ]; then
			continue;
		fi
		
		if [ "$1" = "stop" ]; then
			$IFCONFIG $ARG down
			if [ $ARG != $1 ]; then
				$BRCTL delif $BR_INTF $ARG 2> /dev/null
			fi
		else    
			if [ $ARG != $1 ]; then
				$IFCONFIG $ARG 0.0.0.0 up
				$BRCTL addif $BR_INTF $ARG 2> /dev/null
			fi                    
		fi
	done
	if [ "$1" = "stop" ]; then
		if [ "$ADD_BR" = "1" ]; then
			$IFCONFIG $BR_INTF down
			$BRCTL delbr $BR_INTF
		fi
	fi
}

# Start WLAN interface
rtl_act_wlan_if() {
	VA_INTF="$INTF-va*"
	VXD_IF="$INTF-vxd*"

	#echo "VAP:$VAP"
	#$BIN_DIR/webs -x

	#echo "$WLAN_PREFIX $WLAN_PREFIX_LEN $WLAN_NAME"
	for WLAN in $@ ; do
		case $WLAN in
			$VA_INTF)
				#echo "va: $WLAN"
				#$IWPRIV $WLAN set_mib vap_enable=0
				EXT="va"
			;; 
			$VXD_IF)
				#echo "vxd: $WLAN"
				#$IWPRIV $WLAN set_mib vap_enable=0
				$IWPRIV $WLAN copy_mib
				EXT="vxd"
			;;
			$INTF)  
				#echo "wlan intf: $WLAN"
				$IWPRIV $WLAN set_mib vap_enable=$VAP
				EXT="root"
			;;	   
			*)
				#echo "cmd: $WLAN"
				continue
			;;
		esac
		$IFCONFIG $WLAN down
		if [ "$1" = "start" ]; then
			echo "Initialize $WLAN intf"
			$START_WLAN $WLAN $INTF $EXT
		fi
	done
}

## kill 802.1x, autoconf and IAPP daemon ##
rtl_kill_iwcontrol_pid() {
	PIDFILE="$TOP_VAR_DIR/run/iwcontrol.pid"
	if [ -f $PIDFILE ] ; then
		PID=`cat $PIDFILE`
		echo "IWCONTROL_PID=$PID"
		if [ "$PID" != "0" ]; then
			kill -9 $PID 2>/dev/null
		fi
		rm -f $PIDFILE
	fi
}

rtl_kill_wlan_pid() {
	for WLAN in $@ ; do
		PIDFILE=$TOP_VAR_DIR/run/auth-$WLAN.pid
		if [ -f $PIDFILE ] ; then
			PID=`cat $PIDFILE`
			echo "AUTH_PID=$PID"
			if [ "$PID" != "0" ]; then
				kill -9 $PID 2>/dev/null
			fi
			rm -f $PIDFILE

			PIDFILE=$TOP_VAR_DIR/run/auth-$WLAN-vxd.pid
			if [ -f $PIDFILE ] ; then
				PID=`cat $PIDFILE`
				if [ "$PID" != "0" ]; then
					kill -9 $PID 2>/dev/null
				fi
				rm -f $PIDFILE
			fi
		fi

		# for WPS ---------------------------------->>
		PIDFILE=$TOP_VAR_DIR/run/wscd-$WLAN.pid
		if [ "$both_band_ap" = "1" ]; then
			PIDFILE=$TOP_VAR_DIR/run/wscd-wlan0-wlan1.pid
		fi

		if [ -f $PIDFILE ] ; then
			PID=`cat $PIDFILE`
			echo "WSCD_PID=$PID"
			if [ "$PID" != "0" ]; then
				kill -9 $PID 2>/dev/null
			fi
			rm -f $PIDFILE
		fi
	done
	#<<----------------------------------- for WPS
}

stop_all() {
	HAS_APP=1
	if [ "$1" = "m-ap" ];then
		ALL_INTF=$ALL_M_AP_INTF    
	elif [ "$1" = "ap" ];then
		ALL_INTF=$INTF	    
		#stop_nat
	elif [ "$1" = "rp" ]; then
		ALL_INTF=$ALL_RP_INTF
	elif [ "$1" = "wds" ]; then	
		ALL_INTF=$ALL_WDS_INTF    
		HAS_APP=0
	else
		ALL_INTF=$INTF
		HAS_APP=0	
	fi	    

	$IWPRIV $INTF set_mib wsc_enable=0
	rtl_act_wlan_if stop $ALL_INTF
	rtl_br_lan_if stop $ALL_INTF $LAN_INTF
	$START_WLAN_APP kill $ALL_INTF $BR_INTF
	#killall webs 2> /dev/null
	rtl_kill_iwcontrol_pid $ALL_INTF
	rtl_kill_wlan_pid $ALL_INTF
	rm -f $TOP_VAR_DIR/*.fifo
}

start_wds() {
	if [ "$1" = "open"]; then
		ENC_AUTH="0"
	elif [ "$1" = "wep" ]; then
		ENC_AUTH="1"
	elif [ "$1" = "wpa-tkip" ]; then
		ENC_AUTH="2"      	
	elif [ "$1" = "wpa2_aes" ]; then
		ENC_AUTH="4"	
	fi

	$IWPRIV $INTF set_mib wds_enable=1
	$IWPRIV $INTF set_mib wds_pure=0
	$IWPRIV $INTF set_mib wds_priority=1
	$IWPRIV $INTF set_mib wds_num=0
	$IWPRIV $INTF set_mib wds_encrypt=$ENC_AUTH  ## 0:none 1:wep40 2:tkip 4:aes 5:wep104
	$IWPRIV $INTF set_mib wds_wepkey=$wep_key
	$IWPRIV $INTF set_mib wds_passphrase=$wpa_psk
	$IWPRIV $INTF set_mib wds_add=$WDS_PEER_MAC,0  ## peer mac address, rate

	#$IWPRIV $INTF set_mib wsc_enable=0
	rtl_act_wlan_if start $INTF
	$IFCONFIG $WDS_INTF up
	rtl_br_lan_if start $ALL_WDS_INTF
	echo 1 > /proc/sys/net/ipv6/conf/$WDS_INTF/disable_ipv6
	if [ "$ADD_BR" = "1" ]; then
		$IFCONFIG $BR_INTF $WDS_IP
	fi  
	#$IFCONFIG $INTF-wds0 up
}

wps_restart() {
	MODE=`cat $CONFIG_ROOT_DIR/$2/wlan_mode`

	if [ "$MODE" = "0" ];then
		stop_all ap
		start_ap $ALL_AP_INTF
	else
		stop_all cli
		start_cli $INTF
	fi
}

scan_realtek() {
	local device="$1"
	local vif vifs channel
	local sta_if ap_if

	IFCONFIG=ifconfig

	if [ "$device" = "AP0_2G" ]; then
		INTF="wlan0"
		CONFIG_ROOT_DIR="/var/rtl8192c"
	elif [ "$device" = "AP1_2G" ]; then
		INTF="wlan1"
		CONFIG_ROOT_DIR="/var/rtl8192cd"
	elif [ "$device" = "AP1_5G" ]; then
		INTF="wlan1"
		CONFIG_ROOT_DIR="/var/rtl8192cd"
	fi
	ADD_BR=0
	BR_INTF="br-lan"
	#LAN_INTF="eth0"
	LAN_INTF=

	TOP_VAR_DIR="/var"

	BRCTL="brctl"
	IWPRIV="iwpriv"

	LOCAL_IP="192.168.1.254"
	#MACID="00:01:73:01:FF:AB"
	#LOCAL=00:01:73:01:ff:10    
	WDS_PEER_MAC="002288773327"
	WDS_IP="192.168.99.100"
	ENC="wpa2_aes"
	VAP=0
	RP=0
	WPS=0

	ALL_AP_INTF=
	ALL_RP_INTF=
	ALL_WDS_INTF=
	ALL_WLAN_INTF=
	ALL_M_AP_INTF=
	local _c=0

	config_get channel "$device" channel
	config_get vifs "$device" vifs
	for vif in $vifs; do
		config_get_bool disabled "$vif" disabled 0
		[ $disabled -eq 0 ] || continue

		local mode

		config_get mode "$vif" mode
		config_get ifname "$vif" ifname
		config_get ssid "$vif" ssid

		INTF=$ifname
		WLAN_INTF=$INTF
#		VA_INTF="$INTF-va0"
		VXD_INTF="$INTF-vxd"
		WDS_INTF="$INTF-wds0"

		ALL_AP_INTF="$ALL_AP_INTF $INTF"
		ALL_M_AP_INTF="$ALL_M_AP_INTF $INTF $VA_INTF"
		ALL_RP_INTF="$ALL_RP_INTF $INTF $VXD_INTF"
		ALL_WDS_INTF="$ALL_WDS_INTF $INTF $WDS_INTF"
		ALL_WLAN_INTF="$ALL_WLAN_INTF $WLAN_INTF $VXD_INTF"
		case "$mode" in
			sta)
				sta=1
				STA_SSID=$ssid
				sta_if="$vif"
			;;
			ap)
				apmode=1
				ap_if="${ap_if:+$ap_if }$vif"
				ap_channel=$channel
				eval "ssid_${ifname}=\"\$ssid\""
				ALL_M_AP_INTF="$ALL_M_AP_INTF $ifname"
				_c=$((${_c:-0} + 1))
			;;
			wds)
				wdsmode=1
			;;
			*) echo "$device($vif): Invalid mode, ignored."; continue;;
		esac
	done

	if [ "$_c" -gt 1 ]; then
		VAP=1
	else
		VAP=0
	fi

	if [ -d /proc/wlan0 ] || [ -d /proc/wlan1 ]; then
		if [ ! -d "$CONFIG_ROOT_DIR" ]; then
			mkdir $CONFIG_ROOT_DIR
		fi

		if [ -f "$CONFIG_ROOT_DIR/wifi_script_dir" ]; then
			SCRIPT_DIR=`cat $CONFIG_ROOT_DIR/wifi_script_dir`
		else
			SCRIPT_DIR=/root/wifi/script
			echo $SCRIPT_DIR > $CONFIG_ROOT_DIR/wifi_script_dir
		fi

		if [ -f "$CONFIG_ROOT_DIR/wifi_bin_dir" ]; then
			BIN_DIR=`cat $CONFIG_ROOT_DIR/wifi_bin_dir`
		else
			BIN_DIR="/bin"
			echo $BIN_DIR > $CONFIG_ROOT_DIR/wifi_bin_dir
		fi
	fi

	START_WLAN_ORG=$SCRIPT_DIR/wlan_8192c.sh
	START_WLAN=$SCRIPT_DIR/mywlan_8192c.sh
	START_WLAN_APP=$SCRIPT_DIR/wlanapp_8192c.sh

	for intf in $ALL_WLAN_INTF; do
		if [ -d /proc/wlan0 ] || [ -d /proc/wlan1 ]; then
			if [ "$intf" == "wlan0" ] || [ "$intf" == "wlan0-vxd" ]; then
				$SCRIPT_DIR/default_setting.sh $intf
			fi
			if [ "$intf" == "wlan1" ] || [ "$intf" == "wlan1-vxd" ]; then
				grep -l "RTL8192F" /proc/wlan1/mib_rf 2> /dev/null
				if [ $? -eq 0 ]; then
					$SCRIPT_DIR/default_setting_rtl8192fr.sh $intf
				fi
				grep -l "RTL8812" /proc/wlan1/mib_rf 2> /dev/null
				if [ $? -eq 0 ]; then
					$SCRIPT_DIR/default_setting_ac.sh $intf
				fi
			fi
		fi
	done
}

disable_realtek() {
	local device="$1"

	if [ -d /proc/wlan0 ] || [ -d /proc/wlan1 ]; then
		set_wifi_down "$device"

		if [ "$apmode" == "1" ]; then
			if [ "VAP" == "1" ]; then
				stop_all m_ap
			else
				stop_all ap
			fi
		else
			stop_all cli
		fi
	fi
}

enable_realtek() {
	local device="$1"
	local country vifs macaddr hidden
	local hwmode htmode
	local if_up
	
	if [ -d /proc/wlan0 ] || [ -d /proc/wlan1 ]; then

	hidden=0
	config_get country "$device" country
	config_get vifs "$device" vifs
	config_get hwmode "$device" hwmode
	config_get htmode "$device" htmode
	config_get hidden "$device" hidden

	echo $hidden > $CONFIG_ROOT_DIR/$INTF/hidden_ssid

	hwmode="${hwmode##11}"
	case "$hwmode" in
		ac)	BAND=76;;
		n)	BAND=11;;
		ng|na)	BAND=8;;
		a)	BAND=4;;
		g)	BAND=2;;
		b)	BAND=1;;
		*)	BAND=11;;
	esac
	echo $BAND > $CONFIG_ROOT_DIR/$INTF/band

	case "$htmode" in
		HT80-)	USE_40M=2; UPPER_SIDE=0;;
		HT80+)	USE_40M=2; UPPER_SIDE=1;;
		HT40-)	USE_40M=1; UPPER_SIDE=0;;
		HT40+)	USE_40M=1; UPPER_SIDE=1;;
		HT20)	USE_40M=0; UPPER_SIDE=0;;
		*)	USE_40M=0; UPPER_SIDE=0;;
	esac
	echo $USE_40M > $CONFIG_ROOT_DIR/$INTF/channel_bonding
	echo $UPPER_SIDE > $CONFIG_ROOT_DIR/$INTF/control_sideband

	[ -n "$country" ] && {
		echo "1" > $CONFIG_ROOT_DIR/$INTF/countrycode_enable
		echo $country > $CONFIG_ROOT_DIR/$INTF/countrycode
	}

	for vif in $vifs; do
		config_get_bool disabled "$vif" disabled 0
		[ $disabled -eq 0 ] || continue

		config_get ifname "$vif" ifname
		config_get macaddr "$vif" macaddr
		[ -n "$macaddr" ] && {
			echo $macaddr > $CONFIG_ROOT_DIR/$ifname/wlan0_addr
		}

		config_get enc "$vif" encryption
		case "$enc" in
			*wep*)
				local enc_bits key_type

				config_get wep_type "$vif" encmode
				# 0:wep disabled 1:wep-64bit 2:wep-128bit
				[ -n $wep_type ] && wep_type=1
				case "$wep_type" in
					1) bits=64;;
					2) bits=128;;
					*) bits=64;;
				esac

				# wep_key_type - 1: hex mode 0: ascii
				config_get wep_key_type "$vif" key_type
				[ -n $wep_key_type ] && wep_key_type=0
				case "$wep_key_type" in
					0) key_type=a;;
					1) key_type=h;;
					*) key_type=a;;
				esac

				config_get wep_key_type "$vif" key_type
				config_get key "$vif" key
				case "$key" in
					[1234])
						local idx
						for idx in 1 2 3 4; do
							config_get ikey "$vif" key$idx
							[ -n $ikey ] && {
								eval "wep${bits}_${key_type}_key${key}=\"\$ikey\""
							}
						done

						wep_key_idx=$key
					;;
				esac

				ENC=wep
			;;
			*psk*)
				case "$enc" in
					*mixed*|*psk+psk2*) ENC=mixed;;
					*psk2*) ENC=wpa2_aes;;
					*) ENC=wpa_tkip;;
				esac
				case "$enc" in
					*tkip+aes*|*tkip+ccmp*|*aes+tkip*|*ccmp+tkip*) ENC=mixed;;
					*aes*|*ccmp*) ENC=wpa_aes;;
					*tkip*) ENC=wpa_tkip;;
				esac
				config_get key "$vif" key
				wpa_psk=$key
			;;
			*sae*)
				case "$enc" in
					*sae-mixed*) ENC=sae-mixed;;
					*) ENC=sae;;
				esac
				config_get key "$vif" key
				wpa_psk=$key
			;;
			*wpa*)
				case "$enc" in
					*mixed*|*wpa+wpa2*) ENC=mixed;;
					*wpa2*) ENC=wpa2_aes;;
					*) ENC=wpa_tkip;;
				esac
				case "$enc" in
					*tkip+aes*|*tkip+ccmp*|*aes+tkip*|*ccmp+tkip*) ENC=mixed;;
					*aes*|*ccmp*) ENC=wpa_aes;;
					*tkip*) ENC=wpa_tkip;;
				esac
				config_get key "$vif" key
				wpa_psk=$key
			;;
			*)
				ENC=open
			;;
		esac

		local net_cfg="$(find_net_config "$vif")"
		[ -z "$net_cfg" ] || {
			append if_up "set_wifi_up '$vif' '$ifname'" ";$N"
			append if_up "start_net '$ifname' '$net_cfg'" ";$N"
		}
	done

	ubus -t 5 wait_for network.interface.lan
	if [ "$apmode" == "1" ]; then
		start_ap $ALL_M_AP_INTF
	fi

	if [ "$sta" == "1" ]; then
		start_cli $ENC $INTF
	fi

	eval "$if_up"

	fi
}

detect_realtek() {
	local i=-1

	while grep -qs "^ *wl$((++i)):" /proc/net/dev; do
		local channel type

		config_get type wl${i} type
		[ "$type" = realtek ] && continue
		channel=1
		cat <<EOF
config wifi-device  rtl${i}
	option type     realtek
	option channel  ${channel:-11}
	# REMOVE THIS LINE TO ENABLE WIFI:
	option disabled 1

config wifi-iface
	option device   rtl${i}
	option network	lan
	option mode     ap
	option ssid     ASR-OWRT${i#0}
	option encryption none

EOF
	done
}
