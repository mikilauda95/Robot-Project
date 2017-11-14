default:
	arm-linux-gnueabi-gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c movement.c -o movement.o
	arm-linux-gnueabi-gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c main.c -o main.o
	arm-linux-gnueabi-gcc movement.o main.o -Wall -lm -lev3dev-c -o main

