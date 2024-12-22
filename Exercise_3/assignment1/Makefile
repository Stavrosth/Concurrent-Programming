OPT = -Wall -g -lpthread #-fsanitize=address -fsanitize=bounds

all: 
	@echo "Usage:"
	@echo "  make compile"
	@echo "  make run"
	@echo "  make cleanall"

compile: MonitorsForBinarySemaphoresO binarySemaphoresO testO
	gcc $(OPT) test.o binarySemaphores.o MonitorsForBinarySemaphores.o -o test.bin

run: compile
	./test.bin

MonitorsForBinarySemaphoresO: MonitorsForBinarySemaphores.c
	gcc $(OPT) MonitorsForBinarySemaphores.c -c

binarySemaphoresO: binarySemaphores.c
	gcc $(OPT) binarySemaphores.c -c

testO: test.c
	gcc $(OPT) test.c -c

cleanall:
	rm -f *.bin *.o *.std
