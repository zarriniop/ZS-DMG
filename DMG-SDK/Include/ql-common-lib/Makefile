########################################################
#  Copyright (C) 2018-2099 Quectel Wireless Co.,Ltd. 
#  Quectel Wireless Proprietary and Confidential.
#
#  Compile ql-common-api.
#  Created by Nebula.Li 2019-07-24
#
########################################################

include $(TOPDIR)/rules.mk

PKG_NAME:=ql-common-lib
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/ql-ol-extsdk/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Ql-ol-extsdk
  TITLE:=Print ql-common-lib
  DEPENDS:=+ubus +uci +libubox +libjson-script +ril +curl +libopencore-amr +libffmpeg-4.2.2
endef


define Build/Prepare
endef

define Build/Compile
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include
	cp -rf ./include/* $(1)/usr/include/
	$(INSTALL_BIN) ./libql_common_api.so $(1)/usr/lib/
#	$(INSTALL_BIN) ./libql_common_api.a $(1)/usr/lib
	$(INSTALL_BIN) ./libql_peripheral.so $(1)/usr/lib/
#	$(INSTALL_BIN) ./libql_peripheral.a $(1)/usr/lib/
	$(INSTALL_BIN) ./libql_lib_audio.so $(1)/usr/lib/
#	$(INSTALL_BIN) ./libql_lib_audio.a $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/lib
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/../lib
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/../include
	cp -rf ./include/* $(PKG_BUILD_DIR)/../include
	$(INSTALL_BIN) ./libql_common_api.so $(1)/usr/lib/
	$(INSTALL_BIN) ./libql_common_api.so $(PKG_BUILD_DIR)/lib
#	$(INSTALL_BIN) ./libql_common_api.a $(PKG_BUILD_DIR)/lib
	$(INSTALL_BIN) ./libql_common_api.so $(PKG_BUILD_DIR)/../lib
#	$(INSTALL_BIN) ./libql_common_api.a $(PKG_BUILD_DIR)/../lib

	$(INSTALL_BIN) ./libql_peripheral.so $(1)/usr/lib/
	$(INSTALL_BIN) ./libql_peripheral.so $(PKG_BUILD_DIR)/lib
#	$(INSTALL_BIN) ./libql_peripheral.a $(PKG_BUILD_DIR)/lib
	$(INSTALL_BIN) ./libql_peripheral.so $(PKG_BUILD_DIR)/../lib
#	$(INSTALL_BIN) ./libql_peripheral.a $(PKG_BUILD_DIR)/../lib

	$(INSTALL_BIN) ./libql_lib_audio.so $(1)/usr/lib/
	$(INSTALL_BIN) ./libql_lib_audio.so $(PKG_BUILD_DIR)/lib
#	$(INSTALL_BIN) ./libql_lib_audio.a $(PKG_BUILD_DIR)/lib
	$(INSTALL_BIN) ./libql_lib_audio.so $(PKG_BUILD_DIR)/../lib
#	$(INSTALL_BIN) ./libql_lib_audio.a $(PKG_BUILD_DIR)/../lib

endef

$(eval $(call BuildPackage,$(PKG_NAME)))

