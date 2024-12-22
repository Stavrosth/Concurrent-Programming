OPT = -Wall -g #-fsanitize=address -fsanitize=bounds

all: 
	@echo "Usage:"
	@echo "  make compile"
	@echo "  make run"
	@echo "  make cleanall"

compile: simpletest.c mythreads.c
	gcc $(OPT) simpletest.c mythreads.c -o simpletest.bin

run: compile
	./simpletest.bin $(ARGS)

cleanall:
	rm -f *.std *.bin