################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sub-systemCode/ADCS/ADCS.c \
../src/sub-systemCode/ADCS/Stage_Table.c 

OBJS += \
./src/sub-systemCode/ADCS/ADCS.o \
./src/sub-systemCode/ADCS/Stage_Table.o 

C_DEPS += \
./src/sub-systemCode/ADCS/ADCS.d \
./src/sub-systemCode/ADCS/Stage_Table.d 


# Each subdirectory must supply rules for building sources it contributes
src/sub-systemCode/ADCS/%.o: ../src/sub-systemCode/ADCS/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=arm926ej-s -O0 -fmessage-length=0 -ffunction-sections -Wall -Wextra  -g -Dsdram -Dat91sam9g20 -DTRACE_LEVEL=5 -DDEBUG=1 -D'BASE_REVISION_NUMBER=$(REV)' -D'BASE_REVISION_HASH_SHORT=$(REVHASH_SHORT)' -D'BASE_REVISION_HASH=$(REVHASH)' -I"C:\GivatShmuel\Base-Aug-2019\satellite-subsystems\demo\src" -I"C:/GivatShmuel/Base-Aug-2019/satellite-subsystems/demo/../..//hal/at91/include" -I"C:/GivatShmuel/Base-Aug-2019/satellite-subsystems/demo/../..//hal/hal/include" -I"C:/GivatShmuel/Base-Aug-2019/satellite-subsystems/demo/../..//hal/freertos/include" -I"C:/GivatShmuel/Base-Aug-2019/satellite-subsystems/demo/../..//hal/hcc/include" -I"C:/GivatShmuel/Base-Aug-2019/satellite-subsystems/demo/..//satellite-subsystems/include" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


