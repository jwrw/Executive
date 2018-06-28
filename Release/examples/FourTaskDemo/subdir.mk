################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
INO_SRCS += \
../examples/FourTaskDemo/FiveTaskDemo.ino 

CPP_SRCS += \
../examples/FourTaskDemo/sloeber.ino.cpp 

OBJS += \
./examples/FourTaskDemo/FiveTaskDemo.o \
./examples/FourTaskDemo/sloeber.ino.o 

INO_DEPS += \
./examples/FourTaskDemo/FiveTaskDemo.d 

CPP_DEPS += \
./examples/FourTaskDemo/sloeber.ino.d 


# Each subdirectory must supply rules for building sources it contributes
examples/FourTaskDemo/%.o: ../examples/FourTaskDemo/%.ino
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

examples/FourTaskDemo/%.o: ../examples/FourTaskDemo/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


