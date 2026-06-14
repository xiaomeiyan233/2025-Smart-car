################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/AppSw/Tricore/APP/LQ_ADC_7Mic.c \
../src/AppSw/Tricore/APP/LQ_Atom_Motor.c \
../src/AppSw/Tricore/APP/LQ_CCU6_Timer.c \
../src/AppSw/Tricore/APP/LQ_GPIO_ExInt.c \
../src/AppSw/Tricore/APP/LQ_GPIO_KEY.c \
../src/AppSw/Tricore/APP/LQ_GPIO_LED.c \
../src/AppSw/Tricore/APP/LQ_GPT_mini512.c \
../src/AppSw/Tricore/APP/LQ_I2C_MPU6050.c \
../src/AppSw/Tricore/APP/LQ_ICM20602.c \
../src/AppSw/Tricore/APP/LQ_STM_Timer.c \
../src/AppSw/Tricore/APP/LQ_TFT18.c \
../src/AppSw/Tricore/APP/LQ_Tim_InputCature.c \
../src/AppSw/Tricore/APP/LQ_Tom_Servo.c \
../src/AppSw/Tricore/APP/LQ_UART_Bluetooth.c 

COMPILED_SRCS += \
src/AppSw/Tricore/APP/LQ_ADC_7Mic.src \
src/AppSw/Tricore/APP/LQ_Atom_Motor.src \
src/AppSw/Tricore/APP/LQ_CCU6_Timer.src \
src/AppSw/Tricore/APP/LQ_GPIO_ExInt.src \
src/AppSw/Tricore/APP/LQ_GPIO_KEY.src \
src/AppSw/Tricore/APP/LQ_GPIO_LED.src \
src/AppSw/Tricore/APP/LQ_GPT_mini512.src \
src/AppSw/Tricore/APP/LQ_I2C_MPU6050.src \
src/AppSw/Tricore/APP/LQ_ICM20602.src \
src/AppSw/Tricore/APP/LQ_STM_Timer.src \
src/AppSw/Tricore/APP/LQ_TFT18.src \
src/AppSw/Tricore/APP/LQ_Tim_InputCature.src \
src/AppSw/Tricore/APP/LQ_Tom_Servo.src \
src/AppSw/Tricore/APP/LQ_UART_Bluetooth.src 

C_DEPS += \
src/AppSw/Tricore/APP/LQ_ADC_7Mic.d \
src/AppSw/Tricore/APP/LQ_Atom_Motor.d \
src/AppSw/Tricore/APP/LQ_CCU6_Timer.d \
src/AppSw/Tricore/APP/LQ_GPIO_ExInt.d \
src/AppSw/Tricore/APP/LQ_GPIO_KEY.d \
src/AppSw/Tricore/APP/LQ_GPIO_LED.d \
src/AppSw/Tricore/APP/LQ_GPT_mini512.d \
src/AppSw/Tricore/APP/LQ_I2C_MPU6050.d \
src/AppSw/Tricore/APP/LQ_ICM20602.d \
src/AppSw/Tricore/APP/LQ_STM_Timer.d \
src/AppSw/Tricore/APP/LQ_TFT18.d \
src/AppSw/Tricore/APP/LQ_Tim_InputCature.d \
src/AppSw/Tricore/APP/LQ_Tom_Servo.d \
src/AppSw/Tricore/APP/LQ_UART_Bluetooth.d 

OBJS += \
src/AppSw/Tricore/APP/LQ_ADC_7Mic.o \
src/AppSw/Tricore/APP/LQ_Atom_Motor.o \
src/AppSw/Tricore/APP/LQ_CCU6_Timer.o \
src/AppSw/Tricore/APP/LQ_GPIO_ExInt.o \
src/AppSw/Tricore/APP/LQ_GPIO_KEY.o \
src/AppSw/Tricore/APP/LQ_GPIO_LED.o \
src/AppSw/Tricore/APP/LQ_GPT_mini512.o \
src/AppSw/Tricore/APP/LQ_I2C_MPU6050.o \
src/AppSw/Tricore/APP/LQ_ICM20602.o \
src/AppSw/Tricore/APP/LQ_STM_Timer.o \
src/AppSw/Tricore/APP/LQ_TFT18.o \
src/AppSw/Tricore/APP/LQ_Tim_InputCature.o \
src/AppSw/Tricore/APP/LQ_Tom_Servo.o \
src/AppSw/Tricore/APP/LQ_UART_Bluetooth.o 


# Each subdirectory must supply rules for building sources it contributes
src/AppSw/Tricore/APP/LQ_ADC_7Mic.src: ../src/AppSw/Tricore/APP/LQ_ADC_7Mic.c src/AppSw/Tricore/APP/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc26xb "-fD:/Infineon ADS/ADS WorkPlace/LQ_TC26xB_LIB_ADS1/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
src/AppSw/Tricore/APP/LQ_ADC_7Mic.o: src/AppSw/Tricore/APP/LQ_ADC_7Mic.src src/AppSw/Tricore/APP/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
src/AppSw/Tricore/APP/LQ_Atom_Motor.src: ../src/AppSw/Tricore/APP/LQ_Atom_Motor.c src/AppSw/Tricore/APP/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc26xb "-fD:/Infineon ADS/ADS WorkPlace/LQ_TC26xB_LIB_ADS1/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
src/AppSw/Tricore/APP/LQ_Atom_Motor.o: src/AppSw/Tricore/APP/LQ_Atom_Motor.src src/AppSw/Tricore/APP/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
src/AppSw/Tricore/APP/LQ_CCU6_Timer.src: ../src/AppSw/Tricore/APP/LQ_CCU6_Timer.c src/AppSw/Tricore/APP/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc26xb "-fD:/Infineon ADS/ADS WorkPlace/LQ_TC26xB_LIB_ADS1/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
src/AppSw/Tricore/APP/LQ_CCU6_Timer.o: src/AppSw/Tricore/APP/LQ_CCU6_Timer.src src/AppSw/Tricore/APP/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
src/AppSw/Tricore/APP/LQ_GPIO_ExInt.src: ../src/AppSw/Tricore/APP/LQ_GPIO_ExInt.c src/AppSw/Tricore/APP/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc26xb "-fD:/Infineon ADS/ADS WorkPlace/LQ_TC26xB_LIB_ADS1/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
src/AppSw/Tricore/APP/LQ_GPIO_ExInt.o: src/AppSw/Tricore/APP/LQ_GPIO_ExInt.src src/AppSw/Tricore/APP/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
src/AppSw/Tricore/APP/LQ_GPIO_KEY.src: ../src/AppSw/Tricore/APP/LQ_GPIO_KEY.c src/AppSw/Tricore/APP/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc26xb "-fD:/Infineon ADS/ADS WorkPlace/LQ_TC26xB_LIB_ADS1/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
src/AppSw/Tricore/APP/LQ_GPIO_KEY.o: src/AppSw/Tricore/APP/LQ_GPIO_KEY.src src/AppSw/Tricore/APP/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
src/AppSw/Tricore/APP/LQ_GPIO_LED.src: ../src/AppSw/Tricore/APP/LQ_GPIO_LED.c src/AppSw/Tricore/APP/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc26xb "-fD:/Infineon ADS/ADS WorkPlace/LQ_TC26xB_LIB_ADS1/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
src/AppSw/Tricore/APP/LQ_GPIO_LED.o: src/AppSw/Tricore/APP/LQ_GPIO_LED.src src/AppSw/Tricore/APP/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
src/AppSw/Tricore/APP/LQ_GPT_mini512.src: ../src/AppSw/Tricore/APP/LQ_GPT_mini512.c src/AppSw/Tricore/APP/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc26xb "-fD:/Infineon ADS/ADS WorkPlace/LQ_TC26xB_LIB_ADS1/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
src/AppSw/Tricore/APP/LQ_GPT_mini512.o: src/AppSw/Tricore/APP/LQ_GPT_mini512.src src/AppSw/Tricore/APP/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
src/AppSw/Tricore/APP/LQ_I2C_MPU6050.src: ../src/AppSw/Tricore/APP/LQ_I2C_MPU6050.c src/AppSw/Tricore/APP/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc26xb "-fD:/Infineon ADS/ADS WorkPlace/LQ_TC26xB_LIB_ADS1/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
src/AppSw/Tricore/APP/LQ_I2C_MPU6050.o: src/AppSw/Tricore/APP/LQ_I2C_MPU6050.src src/AppSw/Tricore/APP/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
src/AppSw/Tricore/APP/LQ_ICM20602.src: ../src/AppSw/Tricore/APP/LQ_ICM20602.c src/AppSw/Tricore/APP/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc26xb "-fD:/Infineon ADS/ADS WorkPlace/LQ_TC26xB_LIB_ADS1/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
src/AppSw/Tricore/APP/LQ_ICM20602.o: src/AppSw/Tricore/APP/LQ_ICM20602.src src/AppSw/Tricore/APP/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
src/AppSw/Tricore/APP/LQ_STM_Timer.src: ../src/AppSw/Tricore/APP/LQ_STM_Timer.c src/AppSw/Tricore/APP/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc26xb "-fD:/Infineon ADS/ADS WorkPlace/LQ_TC26xB_LIB_ADS1/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
src/AppSw/Tricore/APP/LQ_STM_Timer.o: src/AppSw/Tricore/APP/LQ_STM_Timer.src src/AppSw/Tricore/APP/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
src/AppSw/Tricore/APP/LQ_TFT18.src: ../src/AppSw/Tricore/APP/LQ_TFT18.c src/AppSw/Tricore/APP/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc26xb "-fD:/Infineon ADS/ADS WorkPlace/LQ_TC26xB_LIB_ADS1/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
src/AppSw/Tricore/APP/LQ_TFT18.o: src/AppSw/Tricore/APP/LQ_TFT18.src src/AppSw/Tricore/APP/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
src/AppSw/Tricore/APP/LQ_Tim_InputCature.src: ../src/AppSw/Tricore/APP/LQ_Tim_InputCature.c src/AppSw/Tricore/APP/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc26xb "-fD:/Infineon ADS/ADS WorkPlace/LQ_TC26xB_LIB_ADS1/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
src/AppSw/Tricore/APP/LQ_Tim_InputCature.o: src/AppSw/Tricore/APP/LQ_Tim_InputCature.src src/AppSw/Tricore/APP/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
src/AppSw/Tricore/APP/LQ_Tom_Servo.src: ../src/AppSw/Tricore/APP/LQ_Tom_Servo.c src/AppSw/Tricore/APP/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc26xb "-fD:/Infineon ADS/ADS WorkPlace/LQ_TC26xB_LIB_ADS1/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
src/AppSw/Tricore/APP/LQ_Tom_Servo.o: src/AppSw/Tricore/APP/LQ_Tom_Servo.src src/AppSw/Tricore/APP/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
src/AppSw/Tricore/APP/LQ_UART_Bluetooth.src: ../src/AppSw/Tricore/APP/LQ_UART_Bluetooth.c src/AppSw/Tricore/APP/subdir.mk
	cctc -cs --dep-file="$(*F).d" --misrac-version=2004 -D__CPU__=tc26xb "-fD:/Infineon ADS/ADS WorkPlace/LQ_TC26xB_LIB_ADS1/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
src/AppSw/Tricore/APP/LQ_UART_Bluetooth.o: src/AppSw/Tricore/APP/LQ_UART_Bluetooth.src src/AppSw/Tricore/APP/subdir.mk
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-src-2f-AppSw-2f-Tricore-2f-APP

clean-src-2f-AppSw-2f-Tricore-2f-APP:
	-$(RM) src/AppSw/Tricore/APP/LQ_ADC_7Mic.d src/AppSw/Tricore/APP/LQ_ADC_7Mic.o src/AppSw/Tricore/APP/LQ_ADC_7Mic.src src/AppSw/Tricore/APP/LQ_Atom_Motor.d src/AppSw/Tricore/APP/LQ_Atom_Motor.o src/AppSw/Tricore/APP/LQ_Atom_Motor.src src/AppSw/Tricore/APP/LQ_CCU6_Timer.d src/AppSw/Tricore/APP/LQ_CCU6_Timer.o src/AppSw/Tricore/APP/LQ_CCU6_Timer.src src/AppSw/Tricore/APP/LQ_GPIO_ExInt.d src/AppSw/Tricore/APP/LQ_GPIO_ExInt.o src/AppSw/Tricore/APP/LQ_GPIO_ExInt.src src/AppSw/Tricore/APP/LQ_GPIO_KEY.d src/AppSw/Tricore/APP/LQ_GPIO_KEY.o src/AppSw/Tricore/APP/LQ_GPIO_KEY.src src/AppSw/Tricore/APP/LQ_GPIO_LED.d src/AppSw/Tricore/APP/LQ_GPIO_LED.o src/AppSw/Tricore/APP/LQ_GPIO_LED.src src/AppSw/Tricore/APP/LQ_GPT_mini512.d src/AppSw/Tricore/APP/LQ_GPT_mini512.o src/AppSw/Tricore/APP/LQ_GPT_mini512.src src/AppSw/Tricore/APP/LQ_I2C_MPU6050.d src/AppSw/Tricore/APP/LQ_I2C_MPU6050.o src/AppSw/Tricore/APP/LQ_I2C_MPU6050.src src/AppSw/Tricore/APP/LQ_ICM20602.d src/AppSw/Tricore/APP/LQ_ICM20602.o src/AppSw/Tricore/APP/LQ_ICM20602.src src/AppSw/Tricore/APP/LQ_STM_Timer.d src/AppSw/Tricore/APP/LQ_STM_Timer.o src/AppSw/Tricore/APP/LQ_STM_Timer.src src/AppSw/Tricore/APP/LQ_TFT18.d src/AppSw/Tricore/APP/LQ_TFT18.o src/AppSw/Tricore/APP/LQ_TFT18.src src/AppSw/Tricore/APP/LQ_Tim_InputCature.d src/AppSw/Tricore/APP/LQ_Tim_InputCature.o src/AppSw/Tricore/APP/LQ_Tim_InputCature.src src/AppSw/Tricore/APP/LQ_Tom_Servo.d src/AppSw/Tricore/APP/LQ_Tom_Servo.o src/AppSw/Tricore/APP/LQ_Tom_Servo.src src/AppSw/Tricore/APP/LQ_UART_Bluetooth.d src/AppSw/Tricore/APP/LQ_UART_Bluetooth.o src/AppSw/Tricore/APP/LQ_UART_Bluetooth.src

.PHONY: clean-src-2f-AppSw-2f-Tricore-2f-APP

