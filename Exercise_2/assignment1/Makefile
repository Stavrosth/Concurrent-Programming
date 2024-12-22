OPT = -Wall -g #-fsanitize=address -fsanitize=bounds

all: 
	@echo "Usage:"
	@echo "  make compile"
	@echo "  make run"
	@echo "  make cleanall"

compile: binarySemaphoresO testO
	gcc $(OPT) test.o binarySemaphores.o -o test

run: compile
	./test

binarySemaphoresO: binarySemaphores.c
	gcc $(OPT) binarySemaphores.c -c

testO: test.c
	gcc $(OPT) test.c -c

cleanall:
	rm -f test test.o binarySemaphores.o *.std