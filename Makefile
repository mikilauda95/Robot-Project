default:
	arm-linux-gnueabi-gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c ./source/movement.c -o ./build/movement.o
	arm-linux-gnueabi-gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c ./source/main.c -o ./build/main.o
	arm-linux-gnueabi-gcc -O2 -std=gnu99 -W -Wall -Wno-comment -c ./source/messages.c -o ./build/messages.o
	arm-linux-gnueabi-gcc ./build/messages.o ./build/movement.o ./build/main.o -Wall -lrt -lm -lev3dev-c -o ./build/main

mq:
	arm-linux-gnueabi-gcc -lrt -o ./build/mq01 ./source/mq01.c

