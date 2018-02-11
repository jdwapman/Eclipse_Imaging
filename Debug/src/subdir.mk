################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ImgSource.cpp \
../src/Tarp.cpp \
../src/filterImageGPU.cpp \
../src/main.cpp \
../src/saveImage.cpp \
../src/searchImage.cpp \
../src/tarpSort.cpp \
../src/timing.cpp 

OBJS += \
./src/ImgSource.o \
./src/Tarp.o \
./src/filterImageGPU.o \
./src/main.o \
./src/saveImage.o \
./src/searchImage.o \
./src/tarpSort.o \
./src/timing.o 

CPP_DEPS += \
./src/ImgSource.d \
./src/Tarp.d \
./src/filterImageGPU.d \
./src/main.d \
./src/saveImage.d \
./src/searchImage.d \
./src/tarpSort.d \
./src/timing.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__cplusplus=201103L -I/usr/local/include/opencv -I/usr/include/c++/5 -I/usr/include/c++/5.4.0 -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


