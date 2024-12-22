OPT = -Wall -g #-fsanitize=address -fsanitize=bounds

all: 
	@echo "Usage:"
	@echo "  make compile"
	@echo "  make run ARGS='<readers> <writers> <iterations>'"
	@echo "  make cleanall"

compile: readers_writers.c ../assignment2/mythreads.c
	gcc $(OPT) readers_writers.c ../assignment2/mythreads.c -o readers_writers.bin

run: compile
	./readers_writers.bin $(ARGS)

cleanall:
	rm -f *.std *.bin