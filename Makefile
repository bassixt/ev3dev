all: bt_messages.o testmotorandultrasonic.o 
	gcc testmotorandultrasonic.o -Wall -lm -pthread -lev3dev-c -o testmotorandultrasonic -lrt -lbluetooth

bt_messages.o : bt_messages.c bt_messages.h
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c bt_messages.c 

testmotorandultrasonic.o: testmotorandultrasonic.c 
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c testmotorandultrasonic.c 

run:
	./testmotorandultrasonic
