# ################################ DONALD ROBOT #############################
# ev3dev
This repository is created for the OS course project.
The original project can be found at the page: http://soc.eurecom.fr/OS/projects_fall2016.html
To use this code the following steps must be done:
1 Create a new dir for the project
2 Clone the project
3 Use the makefile to compile the right "robot brain"

Connnect to the robot and in the terminal cut and paste these lines:
>mkdir donaldbrain
>cd donaldbrain
>git clone git@github.com:bassixt/ev3dev.git
>cd ev3dev
>make "wanted brain"

Replace "wanted brain" with the wanted side of the arena, for example:
SMALL ARENA:
>make beginner

Other options are

BEGINNER BIG ARENA LEFT
>make bigleft

BEGINNER BIG ARENA RIGHT
>make bigright

FINISHER BIG ARENA LEFT
>make finbigleft

FINISHER BIG ARENA RIGHT
>make finbigright

After the make instruction you can run the "robot brain" using the command:
SMALL ARENA:
./beginner

BEGINNER BIG ARENA LEFT
./beginner_big_left

BEGINNER BIG ARENA RIGHT
./beginner_big_right

FINISHER BIG ARENA LEFT
./finisher_big_right

FINISHER BIG ARENA RIGHT
./finisher_big_left
