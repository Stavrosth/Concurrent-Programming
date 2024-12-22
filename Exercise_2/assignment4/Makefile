OPT = -Wall -g #-fsanitize=address -fsanitize=bounds

all: 
	@echo "Usage:"
	@echo "  make compile"
	@echo "  make run N=<train capacity>"
	@echo "  make cleanall"

compile: train.c ../assignment1/binarySemaphores.c
	gcc $(OPT) train.c ../assignment1/binarySemaphores.c -o train

run: compile
	./train $(N)
 
cleanall:
	rm -f train *.std