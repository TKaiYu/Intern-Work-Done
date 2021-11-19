################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LD_SRCS += \
../src/lscript.ld 

C_SRCS += \
../src/freertos_lwip_example_web_utils.c \
../src/freertos_lwip_example_webserver.c \
../src/freertos_lwip_example_ws_http_response.c \
../src/freertos_lwip_example_ws_platform_fs.c \
../src/lwip_example_iic_phyreset.c \
../src/main.c 

OBJS += \
./src/freertos_lwip_example_web_utils.o \
./src/freertos_lwip_example_webserver.o \
./src/freertos_lwip_example_ws_http_response.o \
./src/freertos_lwip_example_ws_platform_fs.o \
./src/lwip_example_iic_phyreset.o \
./src/main.o 

C_DEPS += \
./src/freertos_lwip_example_web_utils.d \
./src/freertos_lwip_example_webserver.d \
./src/freertos_lwip_example_ws_http_response.d \
./src/freertos_lwip_example_ws_platform_fs.d \
./src/lwip_example_iic_phyreset.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v8 gcc compiler'
	aarch64-none-elf-gcc -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -IC:/Users/dso_intern/workspace/avnet/export/avnet/sw/avnet/freertos10_xilinx_psu_cortexa53_0/bspinclude/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


