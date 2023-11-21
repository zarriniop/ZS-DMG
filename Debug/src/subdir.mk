################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/DLMS_Gateway.c \
../src/Tools.c \
../src/connection.c \
../src/exampleserver.c \
../src/main.c 

C_DEPS += \
./src/DLMS_Gateway.d \
./src/Tools.d \
./src/connection.d \
./src/exampleserver.d \
./src/main.d 

OBJS += \
./src/DLMS_Gateway.o \
./src/Tools.o \
./src/connection.o \
./src/exampleserver.o \
./src/main.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-openwrt-linux-gcc -I"/home/zs/mhn/ZS-DMG/DMG-SDK/Include/ql-common-lib/include" -I"/home/zs/mhn/ZS-DMG/development/include" -I"/home/zs/mhn/ZS-DMG/include" -O0 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/DLMS_Gateway.d ./src/DLMS_Gateway.o ./src/Tools.d ./src/Tools.o ./src/connection.d ./src/connection.o ./src/exampleserver.d ./src/exampleserver.o ./src/main.d ./src/main.o

.PHONY: clean-src

