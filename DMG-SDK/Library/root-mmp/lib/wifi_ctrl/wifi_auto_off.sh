#!/bin/sh
. /lib/functions.sh
. /lib/config/uci.sh
CRONTABS_PATH=/etc/crontabs/
CRONTABS_CONFIG_FILE=/etc/crontabs/root
CRONTABS_TMP=/var/crontmp

remove_wifi_task()
{
	if [ ! -f "$CRONTABS_TMP" ] ; then
		touch $CRONTABS_TMP
		chmod 666 $CRONTABS_TMP
  	fi
	echo "" > $CRONTABS_TMP
	
	if [  -f "$CRONTABS_CONFIG_FILE" ] ; then
		sed "/wifi down/d" $CRONTABS_CONFIG_FILE > $CRONTABS_TMP
		cp $CRONTABS_TMP $CRONTABS_CONFIG_FILE
	fi
	rm $CRONTABS_TMP
}
add_wifi_task()
{
	if [ ! -f "$CRONTABS_CONFIG_FILE" ] ; then
                if [ ! -d "$CRONTABS_PATH" ]; then
                        mkdir -p $CRONTAB_PATH
                fi
                touch $CRONTABS_CONFIG_FILE
        fi
	echo "$MIN $HOUR $DAY $MON * wifi down" >> $CRONTABS_CONFIG_FILE 
}

COMMAND=$1
MON=$2
DAY=$3
HOUR=$4
MIN=$5
SEC=$6


case "$COMMAND" in
	"RESET_AUTO_OFF" )
	remove_wifi_task
	add_wifi_task
	;;
	"REMOVE_AUTO_OFF" )
	remove_wifi_task
	;;
esac
