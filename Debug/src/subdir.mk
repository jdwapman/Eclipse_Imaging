################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Tarp.cpp \
../src/getImages.cpp \
../src/gpuCustom.cpp \
../src/main.cpp \
../src/saveImages.cpp \
../src/tarpFind.cpp \
../src/tarpSort.cpp \
../src/timing.cpp 

OBJS += \
./src/Tarp.o \
./src/getImages.o \
./src/gpuCustom.o \
./src/main.o \
./src/saveImages.o \
./src/tarpFind.o \
./src/tarpSort.o \
./src/timing.o 

CPP_DEPS += \
./src/Tarp.d \
./src/getImages.d \
./src/gpuCustom.d \
./src/main.d \
./src/saveImages.d \
./src/tarpFind.d \
./src/tarpSort.d \
./src/timing.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__cplusplus=201103L -I/usr/local/include/opencv -I/usr/include/c++/5 -I/usr/include/c++/5.4.0 -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


