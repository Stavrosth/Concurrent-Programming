OPT = -Wall -g #-fsanitize=address -fsanitize=bounds

all: 
	@echo "Usage:"
	@echo "  make compile"
	@echo "  make run N=<train capacity>"
	@echo "  make cleanall"

compile: train.c ../../assignment2/mythreads.c
	gcc $(OPT) train.c ../../assignment2/mythreads.c -o train.bin

run: compile
	./train.bin $(N)
 
cleanall:
	rm -f train *.std *.bin