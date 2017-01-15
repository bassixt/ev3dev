all: btmessages.o testmotorandultrasonic.o 
	gcc btmessages.o testmotorandultrasonic.o -Wall -lm -pthread -lev3dev-c -o testmotorandultrasonic -lrt -lbluetooth

btmessages.o : mystructures.h btmessages.h btmessages.c
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c btmessages.c 

testmotorandultrasonic.o: testmotorandultrasonic.c 
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c testmotorandultrasonic.c 

run:
	./testmotorandultrasonic
	
clean:
	rm -f *.o
	rm testmotorandultrasonic
