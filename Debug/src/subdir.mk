################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/connection.c \
../src/exampleserver.c \
../src/main.c 

OBJS += \
./src/connection.o \
./src/exampleserver.o \
./src/main.o 

C_DEPS += \
./src/connection.d \
./src/exampleserver.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-openwrt-linux-gcc -I"/home/mhn/ZS-DMG/DMG-SDK/Include/ql-common-lib/include" -I"/home/mhn/ZS-DMG/development/include" -I"/home/mhn/ZS-DMG/include" -O0 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


