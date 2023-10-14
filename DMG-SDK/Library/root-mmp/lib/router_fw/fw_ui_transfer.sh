#!/bin/sh
. /lib/functions.sh
. /lib/config/uci.sh

FW="firewall"
UI_FW_FILE="firewall"
UI_DHCP_FILE="dhcp"
DN_FILTER_ADDN_FILE="/etc/dns_blacklist"
DN_REDIRECT_URL="192.168.1.1"
IP_FILTER_DISABLE_TYPE="FW"
IP_FILTER_ENABLE_TYPE="FW"
IP_FILTER_DISABLE="1"
DN_FILTER_DISABLE="1"
FIND_ADDNHOSTS="0"
CRONTABS_PATH="/etc/crontabs/"
CRONTABS_CONFIG_FILE="/etc/crontabs/root"
CRONTABS_TMP="/var/crontmp"
CHECK_DNFILTER_CRON="/lib/router_fw/fw_ui_transfer.sh DN_RESET"

foreach_ip_disable()
{
   echo "enter $FUNCNAME"
   local config="$1"
   local fw_type;
   local after_enabled;
   config_get fw_type "$config" "filter_type" "unset"
   echo "filter type: $fw_type"
   if [ "$fw_type" == "ip_filter" ] ;then
       uci_set "$FW" "$config" "enabled" "0"
       if [ "$IP_FILTER_DISABLE_TYPE" == "UI" ];then
           echo "set ui_enabled 0"
           uci_set "$FW" "$config" "ui_enabled" "0"
       fi
   fi
   config_get after_enabled "$config" "enabled" "unset"
   echo "after $after_enabled"

}
set_ip_filter_disable()
{
   IP_FILTER_DISABLE_TYPE=$1;
   config_foreach foreach_ip_disable rule
}
foreach_ip_enable()
{
   local config="$1"
   local fw_type;
   local ui_setting;
   config_get fw_type "$config" "filter_type" "unset"
   if [ "$fw_type" == "ip_filter" ];then
      if [ "$IP_FILTER_ENABLE_TYPE" == "FW" ] ;then
        config_get ui_setting "$config" "ui_enabled"
        if [ -n ui_setting ];then
          uci_set "$FW" "$config" "enabled" $ui_setting
        fi
      elif [ "$IP_FILTER_ENABLE_TYPE" == "UI" ];then
        uci_set "$FW" "$config" "enabled" "1"
        uci_set "$FW" "$config" "ui_enabled" "1"
      fi
   fi
}
set_ip_filter_enable()
{
   IP_FILTER_ENABLE_TYPE=$1;
   config_foreach foreach_ip_enable rule
}
foreach_defaults()                                  
{                                                      
  local config="$1"                                    
  config_get IP_FILTER_DISABLE "$config" "ip_filter_disable"
  config_get DN_FILTER_DISABLE "$config" "dn_filter_disable"
}                                
                                                            
handle_addnhosts()
{
  local value="$1"

  if [ "$value" == "$DN_FILTER_ADDN_FILE" ]; then
	FIND_ADDNHOSTS=1
  fi 
}
foreach_handle_dnsmasq()
{
   local config="$1"
   config_list_foreach "$config" "addnhosts" handle_addnhosts
}
foreach_add_addnhosts()
{
	echo "enter add addnhosts"
	local config="$1"
	config_add_list "$config" "addnhosts" "$DN_FILTER_ADDN_FILE"
}
lan_down_and_up()
{

	#down and up to clean client side dns cache	
	#/etc/init.d/dnsmasq restart
	ifconfig usbnet0 down
	ifconfig usbnet0 up
	#ifconfig wlan0 down
	#ifconfig wlan0 up
}
remove_dns_from_cron()
{
	if [ ! -f "$CRONTABS_TMP" ] ;then
		touch $CRONTABS_TMP
		chmod 666 $CRONTABS_TMP
	fi 
	echo "" > $CRONTABS_TMP

	if [ -f "$CRONTABS_CONFIG_FILE" ] ; then
		sed "/fw_ui_transfer.sh/d" $CRONTABS_CONFIG_FILE > $CRONTABS_TMP
	        cp $CRONTABS_TMP $CRONTABS_CONFIG_FILE
	fi
	rm $CRONTABS_TMP	
}


add_dhcp_address_list()
{
	local config="$1"
	#check if /etc/config/dhcp include "addnhosts"
	config_load "$UI_DHCP_FILE"

	uci export dhcp
	uci add_list dhcp.@dnsmasq[0].address="/${config/www./}/$DN_REDIRECT_URL"
	uci commit
}

handle_remove_address()
{
	sed -i "/list address/d" "/etc/config/dhcp"
}

foreach_handle_remove_dnsmasq()
{
   local config="$1"
   config_list_foreach "$config" "address" handle_remove_address
}

remove_dhcp_address_list()
{
	local config="$1"
	config_load "$UI_DHCP_FILE"
	config_foreach foreach_handle_remove_dnsmasq "dnsmasq"
	uci_commit "$UI_DHCP_FILE"
}

foreach_addinto_address()
{
	local config="$1"
	local start_time
	local stop_time
	local start_sec
	local stop_sec
	local cur_sec
	local cur_date
	local start_date_time
	local stop_date_time
	local enabled
	
	config_get enabled "$config"	"enabled"
	if [ $enabled -eq "0" ]; then
		return 0
	fi 
	
	config_get start_time "$config"	"start_time"
	config_get stop_time	"$config"	"stop_time"
	cur_date=$(date +%Y-%m-%d)

	start_date_time=$cur_date" "$start_time":00"
	stop_date_time=$cur_date" "$stop_time":00"	
	cur_sec=$(date +%s)

	start_sec=$(date -d "$start_date_time"	+%s)
	stop_sec=$(date -d "$stop_date_time"	+%s)
	
	if [ $cur_sec -ge $start_sec ]; then

		if [ $cur_sec -lt $stop_sec ]; then
			config_get domain_url "$config"	"domain_name"
			add_dhcp_address_list $domain_url
		fi
	fi			
}	

foreach_addinto_cron()
{
	local config="$1"
	local start_time;
	local stop_time;
	local start_hour;
	local start_min;
	local stop_hour;
	local stop_min;
	
	config_get start_time "$config"	"start_time"
	config_get stop_time	"$config"	"stop_time"
	
	start_hour=${start_time%:*}
	start_min=${start_time#*:}

	#unset hour
	#unset min
	stop_hour=${stop_time%:*}
    stop_min=${stop_time#*:}

    echo "$start_min,$stop_min $start_hour,$stop_hour * * * $CHECK_DNFILTER_CRON" >> $CRONTABS_CONFIG_FILE
	
}

add_dnfilter_to_dhcp_address()
{
	remove_dhcp_address_list
	config_load "$UI_FW_FILE" 
   	config_foreach foreach_addinto_address "dnsblacklist"
}
check_addinto_cron()
{
	remove_dns_from_cron;
    if [ ! -f "$CRONTABS_CONFIG_FILE" ] ; then
	 	if [ ! -d "$CRONTABS_PATH" ]; then
			mkdir -p $CRONTAB_PATH
		fi
		touch $CRONTABS_CONFIG_FILE		
	fi
	config_foreach foreach_addinto_cron "dnsblacklist"
}
set_dn_filter_enable()                               
{
   	check_addinto_cron
	add_dnfilter_to_dhcp_address
	crontab $CRONTABS_CONFIG_FILE
}
set_dn_filter_disable()                              
{
   	remove_dns_from_cron	
	remove_dhcp_address_list
	crontab $CRONTABS_CONFIG_FILE
}

set_fw_disable()                                            
{

  set_ip_filter_disable "FW"
  set_dn_filter_disable
}
set_fw_enable()                                     
{
  config_foreach foreach_defaults defaults

  if [ "$IP_FILTER_DISABLE" == "0" ] ;then
    set_ip_filter_enable "FW"
  fi
  if [ "$DN_FILTER_DISABLE" == "0" ];then
     set_dn_filter_enable "FW"
  fi
}

isnumber()
{
  ret=`expr match $1 "[0-9][0-9]*$"`
  if [ $ret -gt 0 ];then
        return 0
  else
        return 1
  fi
}

del_trigger_rule()
{
	local del_list;
	local keys;
	local revert_list;
   	del_list=`iptables -L --line-number | grep TRIGGER | cut -d ' ' -f 1 | tr "\n" " "`

	for keys in $del_list
	do
	  revert_list=$keys' '$revert_list
	done

	for keys in $revert_list
	do
	  iptables -D FORWARD $keys
	done
	
	unset del_list
	unset revert_list
	del_list=`iptables -t nat -L --line-number | grep TRIGGER | cut -d ' ' -f 1 | tr "\n" " "`
        for keys in $del_list                                                                    
        do                                                                                       
          revert_list=$keys' '$revert_list                                                       
        done                                                                                     
        for keys in $revert_list                                                                 
        do                                                                                       
          iptables -t nat -D PREROUTING $keys                                                              
        done     

}
foreach_trigger_rule()
{

	local config="$1"
	local enabled;
	local out_port;
	local out_proto;
	local in_port;
	local in_proto;

	config_get enabled "$config" "enabled" "unset"

	if [ $enabled == "0" ]; then
		return 0
	fi
	
	config_get out_port "$config" "out_port"
	config_get in_port "$config"	"in_port"
	config_get in_proto "$config"	"in_proto"
	config_get out_proto	"$config"	"out_proto"

	iptables -A FORWARD -p $out_proto --dport $out_port -j TRIGGER --trigger-type out --trigger-match $out_port --trigger-relate $in_port --trigger-proto $in_proto

}
set_trigger_rule()
{
	config_foreach foreach_trigger_rule "port_trigger_rule"
	`iptables -t nat -A PREROUTING -j TRIGGER --trigger-type dnat`
}

INPUT_TYPE="$1"                                             
echo "input : $INPUT_TYPE"                           
echo "$UI_FW_FILE"                                  
config_load "$UI_FW_FILE"        
case "$INPUT_TYPE" in                                       
   "FW_DISABLE" )                                           
    set_fw_disable ;;                               
   "FW_ENABLE" )                             
    set_fw_enable ;;                                 
   "IP_ENABLE" )                                     
    set_ip_filter_enable "UI" ;;                   
   "IP_DISABLE" )                                   
    set_ip_filter_disable "UI" ;;                           
   "DN_DISABLE" )                                            
    set_dn_filter_disable "UI" 
	lan_down_and_up
	;; 
    "DN_RESET" )
    set_dn_filter_enable
	lan_down_and_up
	;;                   
   "DN_ENABLE" )                                   
    set_dn_filter_enable
	lan_down_and_up
	;;                     
    "PORT_TRIGGER_RESET")
     echo "here"
     del_trigger_rule
     set_trigger_rule ;;
esac                                                 
uci_commit "$UI_FW_FILE" 


