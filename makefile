execute: compile
	./exercise_1

debug: compile_debugger
	gdb exercise_1

compile:
	gcc -o exercise_1 exercise_1.c -fopenmp

compile_debugger:
	gcc -g -o exercise_1 exercise_1.c -fopenmp