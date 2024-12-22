OPT = -Wall -g #-fsanitize=address -fsanitize=bounds

all: 
	@echo "Usage:"
	@echo "  make compile"
	@echo "  make run N=<train capacity>"
	@echo "  make cleanall"

compile: train.c ../binarySemaphores.o ../MonitorsForBinarySemaphores.o
	gcc $(OPT) train.c ../binarySemaphores.o ../MonitorsForBinarySemaphores.o -o train.bin

run: compile
	./train.bin $(N)
 
cleanall:
	rm -f *.bin *.std