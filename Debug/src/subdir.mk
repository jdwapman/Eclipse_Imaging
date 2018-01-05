################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Tarp.cpp \
../src/gpuCustom.cpp \
../src/main.cpp \
../src/tarpSort.cpp 

OBJS += \
./src/Tarp.o \
./src/gpuCustom.o \
./src/main.o \
./src/tarpSort.o 

CPP_DEPS += \
./src/Tarp.d \
./src/gpuCustom.d \
./src/main.d \
./src/tarpSort.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__cplusplus=201103L -I/usr/local/include/opencv -I/usr/include/c++/5 -I/usr/include/c++/5.4.0 -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


