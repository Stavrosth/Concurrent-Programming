OPT = -Wall -g #-fsanitize=address -fsanitize=bounds

all: 
	@echo "Usage:"
	@echo "  make compile"
	@echo "  make run N=<max number of cars on bridge>"
	@echo "  make cleanall"

compile: bridge.c ../assignment1/binarySemaphores.c
	gcc $(OPT) bridge.c ../assignment1/binarySemaphores.c -o bridge

run: compile
	./bridge $(N)
 
cleanall:
	rm -f bridge *.std