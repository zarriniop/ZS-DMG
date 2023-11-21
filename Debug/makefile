################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := ZS-DMG
BUILD_ARTIFACT_EXTENSION :=
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: main-build

# Main-build Target
main-build: ZS-DMG

# Tool invocations
ZS-DMG: $(OBJS) $(USER_OBJS) makefile $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	arm-openwrt-linux-gcc -L"/home/zs/mhn/ZS-DMG/DMG-SDK/Library/ql-ol-extsdk/lib" -L"/home/zs/mhn/ZS-DMG/DMG-SDK/Library/uci-2021-04-14-4b3db117" -L"/home/zs/mhn/ZS-DMG/DMG-SDK/Library/usr/lib" -L"/home/zs/mhn/ZS-DMG/DMG-SDK/Library/root-mmp/lib" -L"/home/zs/mhn/ZS-DMG/development/lib" -o "ZS-DMG" $(OBJS) $(USER_OBJS) $(LIBS) -lubus -lgurux_dlms_c -lm -lpthread -lnghttp2 -lwolfssl -lssl -lcrypto -lcurl -lql_common_api -lubox -luci -lblobmsg_json -ljson-c -ljson_script -lrilutil -llog -lprop2uci -lcutils -lavcodec -lavutil -lswresample -lql_peripheral
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) ZS-DMG
	-@echo ' '

.PHONY: all clean dependents main-build

-include ../makefile.targets
