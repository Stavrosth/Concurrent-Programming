OPT = -Wall -g #-fsanitize=address -fsanitize=bounds

all: 
	@echo "Usage:"
	@echo "  make compile"
	@echo "  make run s=<storage_size> f=<filename>"
	@echo "  make clean"
	@echo "  make clean.copy"
	@echo "  make cleanall"
	@echo "  make diff f=<filename>"

compile: coroutinesO prod_consO clean.copy
	gcc $(OPT) prod_cons.o coroutines.o -o prod_cons.bin

run: compile
	./prod_cons.bin $(s) $(f)
	
prod_consO: prod_cons.c
	gcc $(OPT) prod_cons.c -c

coroutinesO: coroutines.c
	gcc $(OPT) coroutines.c -c

clean:
	rm -f *.o *.std *.error *.bin

clean.copy:
	rm -f *.copy

cleanall: clean clean.copy

diff:
	diff $(f) $(f).copy