all:	main

main:
	nvcc src/main.cu -o ./build/main

normal:
	gcc src/main.c -o build/main_cpu -ggdb
