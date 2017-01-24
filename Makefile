finisher:
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c finisher.c -o finisher.o
	gcc finisher.o -Wall -lm -pthread -lev3dev-c -o finisher -lrt -lbluetooth

beginner:
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c beginner.c -o beginner.o
	gcc beginner.o -Wall -lm -pthread -lev3dev-c -o beginner -lrt -lbluetooth

bigleft:
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c beginner_big_left.c -o beginner_big_left.o
	gcc beginner_big_left.o -Wall -lm -pthread -lev3dev-c -o beginner_big_left -lrt -lbluetooth

bigrightbeg:
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c beginner_big_right.c -o beginner_big_right.o
	gcc beginner_big_right.o -Wall -lm -pthread -lev3dev-c -o beginner_big_right -lrt -lbluetooth
	
finbigright:
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c finisher_big_right.c -o finisher_big_right.o
	gcc finisher_big_right.o -Wall -lm -pthread -lev3dev-c -o finisher_big_right -lrt -lbluetooth
	
finbigleft:
	gcc -I./ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment -c finisher_big_left.c -o finisher_big_left.o
	gcc finisher_big_left.o -Wall -lm -pthread -lev3dev-c -o finisher_big_left -lrt -lbluetooth



