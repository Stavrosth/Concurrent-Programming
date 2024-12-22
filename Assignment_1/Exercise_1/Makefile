OPT = -Wall -g #-fsanitize=address -fsanitize=bounds

all: 
	@echo "Usage:"
	@echo "  make compile"
	@echo "  make run f=<filename>"
	@echo "  make heavytest f=<filename> n=<number_of_iterations>"
	@echo "  make clean"
	@echo "  make clean.copy"
	@echo "  make cleanall"
	@echo "  make diff f=<filename>"

compile: pipesO testO clean.copy
	gcc $(OPT) test.o pipes.o -o test

run: compile
	./test $(f)

heavytest: compile heavytest.py
	python3 heavytest.py $(f) $(n)
	
testO: test.c
	gcc $(OPT) test.c -c

pipesO: pipes.c
	gcc $(OPT) pipes.c -c

clean:
	rm -f test test.o pipes.o *.std *.error

clean.copy:
	rm -f *.txt.copy *.txt.copy2 

cleanall: clean clean.copy

diff:
	diff $(f) $(f).copy
	diff $(f) $(f).copy2