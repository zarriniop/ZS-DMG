#!/bin/sh

do_sysinfo_mmp() {
	. /lib/mmp.sh
	#mmp_board_detect
}

boot_hook_add preinit_main do_sysinfo_mmp
