default:
	arm-linux-gnueabi-gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c movement.c -o movement.o
	arm-linux-gnueabi-gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c main.c -o main.o
	arm-linux-gnueabi-gcc -O2 -std=gnu99 -W -Wall -Wno-comment -c messages.c -o messages.o
	arm-linux-gnueabi-gcc messages.o movement.o main.o -Wall -lrt -lm -lev3dev-c -o main

mq:
	arm-linux-gnueabi-gcc -lrt -o mq01 mq01.c

