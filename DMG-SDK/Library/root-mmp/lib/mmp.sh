#!/bin/sh
#
# Copyright (C) 2014 OpenWrt.org
#

MMP_BOARD_NAME=
MMP_MODEL=

mmp_board_detect() {
	local machine
	local name

	machine=$(cat /etc/mversion)

	case "$machine" in
	*"pxa1826spinor"*)
		name="pxa1826-spinor"
		;;
	*"pxa1826spinand"*)
		name="pxa1826-spinand"
		;;
	*"pxa1826"*)
		name="pxa1826"
		;;
	*"pxa1826pcie"*)
		name="pxa1826-pcie"
		;;
	*"pxa1826p305"*)
		name="pxa1826-p305"
		;;
	*"asr1802s"*)
		name="asr1802s"
		;;
	esac

	[ -z "$name" ] && name="unknown"

	[ -z "$MMP_BOARD_NAME" ] && MMP_BOARD_NAME="$name"
	[ -z "$MMP_MODEL" ] && MMP_MODEL="$machine"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$MMP_BOARD_NAME" > /tmp/sysinfo/board_name
	echo "$MMP_MODEL" > /tmp/sysinfo/model
}

mmp_board_name() {
	local name

	[ -f /tmp/sysinfo/board_name ] && name=$(cat /tmp/sysinfo/board_name)
	[ -z "$name" ] && name="unknown"

	echo "$name"
}
