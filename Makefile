

all:
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c testmotorandultrasonic.c -o testmotorandultrasonic.o
	gcc testmotorandultrasonic.o -Wall -lm -lev3dev-c -o testmotorandultrasonic

run:
	./testmotorandultrasonic
