OPT = -Wall -g #-fsanitize=address -fsanitize=bounds

all:
	@echo "Usage:"
	@echo "  make compile"
	@echo "  make run f=<filename>"
	@echo "  make create_bin f=<filename> s=<size>"
	@echo "  make read_bin f=<filename>"
	@echo "  make convert_bin_to_txt f=<filename>"
	@echo "  make check_sorted f=<filename>"
	@echo "  make heavytest f=<filename> n=<array_size>"
	@echo "  make clean"
	@echo "  make clean.hidden"
	@echo "  make cleanall"
	
compile: ExtMergeSort.c
	gcc $(OPT) ExtMergeSort.c -o ExtMergeSort

run: compile
	./ExtMergeSort $(f)

create_bin: create_binary_file.py
	python3 create_binary_file.py $(f) $(s)

read_bin: read_binary_file.py
	python3 read_binary_file.py $(f)

convert_bin_to_txt: read_binary_file.py
	python3 read_binary_file.py $(f) >$(f).txt

check_sorted: check_sorted_file.py
	python3 check_sorted_file.py $(f)

heavytest: compile heavytest.py
	python3 heavytest.py $(f) $(n)

clean:
	rm -f ExtMergeSort *.txt *.bin

clean.hidden:
	rm -f .[!.]*

cleanall: clean clean.hidden