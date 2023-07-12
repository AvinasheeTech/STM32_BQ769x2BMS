################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BQ76942_BMS/src/BMS_SPI.c \
../Drivers/BQ76942_BMS/src/BQ76942.c 

OBJS += \
./Drivers/BQ76942_BMS/src/BMS_SPI.o \
./Drivers/BQ76942_BMS/src/BQ76942.o 

C_DEPS += \
./Drivers/BQ76942_BMS/src/BMS_SPI.d \
./Drivers/BQ76942_BMS/src/BQ76942.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BQ76942_BMS/src/%.o Drivers/BQ76942_BMS/src/%.su: ../Drivers/BQ76942_BMS/src/%.c Drivers/BQ76942_BMS/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32F446xx -DUSE_FULL_LL_DRIVER -DHSE_VALUE=8000000 -DHSE_STARTUP_TIMEOUT=100 -DLSE_STARTUP_TIMEOUT=5000 -DLSE_VALUE=32768 -DEXTERNAL_CLOCK_VALUE=12288000 -DHSI_VALUE=16000000 -DLSI_VALUE=32000 -DVDD_VALUE=3300 -DPREFETCH_ENABLE=1 -DINSTRUCTION_CACHE_ENABLE=1 -DDATA_CACHE_ENABLE=1 -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"/home/shyam/STM32CubeIDE/workspace_1.11.0/STM32_BMS/Drivers/BQ76942_BMS/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BQ76942_BMS-2f-src

clean-Drivers-2f-BQ76942_BMS-2f-src:
	-$(RM) ./Drivers/BQ76942_BMS/src/BMS_SPI.d ./Drivers/BQ76942_BMS/src/BMS_SPI.o ./Drivers/BQ76942_BMS/src/BMS_SPI.su ./Drivers/BQ76942_BMS/src/BQ76942.d ./Drivers/BQ76942_BMS/src/BQ76942.o ./Drivers/BQ76942_BMS/src/BQ76942.su

.PHONY: clean-Drivers-2f-BQ76942_BMS-2f-src

