OPT = -Wall -g #-fsanitize=address -fsanitize=bounds

all: 
	@echo "Usage:"
	@echo "  make compile"
	@echo "  make run N=<max number of cars on bridge>"
	@echo "  make cleanall"

compile: bridge.c ../binarySemaphores.o ../MonitorsForBinarySemaphores.o
	gcc $(OPT) bridge.c ../binarySemaphores.o ../MonitorsForBinarySemaphores.o -o bridge.bin

run: compile
	./bridge.bin $(N)
 
cleanall:
	rm -f *.bin *.std