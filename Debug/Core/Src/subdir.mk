################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/LCD.c \
../Core/Src/MAX31865.c \
../Core/Src/adc.c \
../Core/Src/ads1115.c \
../Core/Src/gpio.c \
../Core/Src/i2c.c \
../Core/Src/lwgps.c \
../Core/Src/main.c \
../Core/Src/my_code.c \
../Core/Src/spi.c \
../Core/Src/stm32f0xx_hal_msp.c \
../Core/Src/stm32f0xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f0xx.c \
../Core/Src/tim.c \
../Core/Src/tsc.c \
../Core/Src/usart.c \
../Core/Src/usb.c 

OBJS += \
./Core/Src/LCD.o \
./Core/Src/MAX31865.o \
./Core/Src/adc.o \
./Core/Src/ads1115.o \
./Core/Src/gpio.o \
./Core/Src/i2c.o \
./Core/Src/lwgps.o \
./Core/Src/main.o \
./Core/Src/my_code.o \
./Core/Src/spi.o \
./Core/Src/stm32f0xx_hal_msp.o \
./Core/Src/stm32f0xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f0xx.o \
./Core/Src/tim.o \
./Core/Src/tsc.o \
./Core/Src/usart.o \
./Core/Src/usb.o 

C_DEPS += \
./Core/Src/LCD.d \
./Core/Src/MAX31865.d \
./Core/Src/adc.d \
./Core/Src/ads1115.d \
./Core/Src/gpio.d \
./Core/Src/i2c.d \
./Core/Src/lwgps.d \
./Core/Src/main.d \
./Core/Src/my_code.d \
./Core/Src/spi.d \
./Core/Src/stm32f0xx_hal_msp.d \
./Core/Src/stm32f0xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f0xx.d \
./Core/Src/tim.d \
./Core/Src/tsc.d \
./Core/Src/usart.d \
./Core/Src/usb.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F072xB -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -u _printf_float -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/LCD.cyclo ./Core/Src/LCD.d ./Core/Src/LCD.o ./Core/Src/LCD.su ./Core/Src/MAX31865.cyclo ./Core/Src/MAX31865.d ./Core/Src/MAX31865.o ./Core/Src/MAX31865.su ./Core/Src/adc.cyclo ./Core/Src/adc.d ./Core/Src/adc.o ./Core/Src/adc.su ./Core/Src/ads1115.cyclo ./Core/Src/ads1115.d ./Core/Src/ads1115.o ./Core/Src/ads1115.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/i2c.cyclo ./Core/Src/i2c.d ./Core/Src/i2c.o ./Core/Src/i2c.su ./Core/Src/lwgps.cyclo ./Core/Src/lwgps.d ./Core/Src/lwgps.o ./Core/Src/lwgps.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/my_code.cyclo ./Core/Src/my_code.d ./Core/Src/my_code.o ./Core/Src/my_code.su ./Core/Src/spi.cyclo ./Core/Src/spi.d ./Core/Src/spi.o ./Core/Src/spi.su ./Core/Src/stm32f0xx_hal_msp.cyclo ./Core/Src/stm32f0xx_hal_msp.d ./Core/Src/stm32f0xx_hal_msp.o ./Core/Src/stm32f0xx_hal_msp.su ./Core/Src/stm32f0xx_it.cyclo ./Core/Src/stm32f0xx_it.d ./Core/Src/stm32f0xx_it.o ./Core/Src/stm32f0xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f0xx.cyclo ./Core/Src/system_stm32f0xx.d ./Core/Src/system_stm32f0xx.o ./Core/Src/system_stm32f0xx.su ./Core/Src/tim.cyclo ./Core/Src/tim.d ./Core/Src/tim.o ./Core/Src/tim.su ./Core/Src/tsc.cyclo ./Core/Src/tsc.d ./Core/Src/tsc.o ./Core/Src/tsc.su ./Core/Src/usart.cyclo ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su ./Core/Src/usb.cyclo ./Core/Src/usb.d ./Core/Src/usb.o ./Core/Src/usb.su

.PHONY: clean-Core-2f-Src

