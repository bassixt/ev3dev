

all:
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c testmotorandultrasonic.c -o testmotorandultrasonic.o
	gcc testmotorandultrasonic.o -Wall -lm -pthread -lev3dev-c -o testmotorandultrasonic -lrt -lbluetooth

beginner:
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c beginner.c -o beginner.o
	gcc beginner.o -Wall -lm -pthread -lev3dev-c -o beginner -lrt -lbluetooth

bigleft:
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c beginner_big_left.c -o beginner_big_left.o
	gcc beginner_big_left.o -Wall -lm -pthread -lev3dev-c -o beginner_big_left -lrt -lbluetooth

	
run:
	./testmotorandultrasonic
