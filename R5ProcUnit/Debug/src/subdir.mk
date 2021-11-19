################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LD_SRCS += \
../src/lscript.ld 

C_SRCS += \
../src/R5Code.c \
../src/platform.c 

OBJS += \
./src/R5Code.o \
./src/platform.o 

C_DEPS += \
./src/R5Code.d \
./src/platform.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM R5 gcc compiler'
	armr5-none-eabi-gcc -DARMR5 -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-r5 -mfloat-abi=hard  -mfpu=vfpv3-d16 -IC:/Users/dso_intern/workspace/MailMutex/export/MailMutex/sw/MailMutex/standalone_psu_cortexr5_1/bspinclude/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


