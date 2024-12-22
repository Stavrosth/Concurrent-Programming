OPT = -Wall -g #-fsanitize=address -fsanitize=bounds

all: 
	@echo "Usage:"
	@echo "  make compile"
	@echo "  make run ARGS='<number of workers/threads> <int1> <int2> ... <intN>'"
	@echo "  make heavytest ARGS='<number_of_times> <max_threads> <thread_step> <max_ints_num> <max_int_value>'"
	@echo "  make cleanall"

compile: PrimalityTesterWithSemaphores.c ../binarySemaphores.o ../MonitorsForBinarySemaphores.o
	gcc $(OPT) PrimalityTesterWithSemaphores.c ../binarySemaphores.o ../MonitorsForBinarySemaphores.o -o PrimalityTester.bin

run: compile
	./PrimalityTester.bin $(ARGS)
 
heavytest: compile heavytest.py
	python3 heavytest.py $(ARGS)

cleanall:
	rm -f *.bin *.std