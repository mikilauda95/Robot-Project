#This makefile will output files to the ./build directory. After running make, you can transfer the correct file to the robot and run it

default:
	arm-linux-gnueabi-gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c ./source/movement.c -o ./build/movement.o
	arm-linux-gnueabi-gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c ./source/sensors.c -o ./build/sensors.o
	arm-linux-gnueabi-gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c ./source/main.c -o ./build/main.o
	arm-linux-gnueabi-gcc -O2 -std=gnu99 -W -Wall -Wno-comment -c ./source/messages.c -o ./build/messages.o
	arm-linux-gnueabi-gcc ./build/messages.o ./build/sensors.o ./build/movement.o ./build/main.o -Wall -lrt -lm -lev3dev-c -o ./build/main


