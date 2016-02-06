################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ChatRoom.cpp \
../src/Listener.cpp \
../src/LoginManager.cpp \
../src/MThread.cpp \
../src/MessengerServer.cpp \
../src/MultipleTCPSocketsListener.cpp \
../src/TCPSocket.cpp \
../src/User.cpp \
../src/main.cpp 

OBJS += \
./src/ChatRoom.o \
./src/Listener.o \
./src/LoginManager.o \
./src/MThread.o \
./src/MessengerServer.o \
./src/MultipleTCPSocketsListener.o \
./src/TCPSocket.o \
./src/User.o \
./src/main.o 

CPP_DEPS += \
./src/ChatRoom.d \
./src/Listener.d \
./src/LoginManager.d \
./src/MThread.d \
./src/MessengerServer.d \
./src/MultipleTCPSocketsListener.d \
./src/TCPSocket.d \
./src/User.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


