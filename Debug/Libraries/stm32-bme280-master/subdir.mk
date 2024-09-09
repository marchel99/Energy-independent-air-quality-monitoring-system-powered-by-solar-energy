################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libraries/stm32-bme280-master/bme280.c \
../Libraries/stm32-bme280-master/bme280_support.c 

OBJS += \
./Libraries/stm32-bme280-master/bme280.o \
./Libraries/stm32-bme280-master/bme280_support.o 

C_DEPS += \
./Libraries/stm32-bme280-master/bme280.d \
./Libraries/stm32-bme280-master/bme280_support.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries/stm32-bme280-master/%.o Libraries/stm32-bme280-master/%.su Libraries/stm32-bme280-master/%.cyclo: ../Libraries/stm32-bme280-master/%.c Libraries/stm32-bme280-master/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L476xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"D:/ST/STM32CubeIDE_1.15.1/STM32CubeIDE/D/Users/mrchl/STM32CubeIDE/workspace_1.15.1/31_05_2024/Libraries/stm32-bme280-master" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Libraries-2f-stm32-2d-bme280-2d-master

clean-Libraries-2f-stm32-2d-bme280-2d-master:
	-$(RM) ./Libraries/stm32-bme280-master/bme280.cyclo ./Libraries/stm32-bme280-master/bme280.d ./Libraries/stm32-bme280-master/bme280.o ./Libraries/stm32-bme280-master/bme280.su ./Libraries/stm32-bme280-master/bme280_support.cyclo ./Libraries/stm32-bme280-master/bme280_support.d ./Libraries/stm32-bme280-master/bme280_support.o ./Libraries/stm32-bme280-master/bme280_support.su

.PHONY: clean-Libraries-2f-stm32-2d-bme280-2d-master

