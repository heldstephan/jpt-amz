all:
	make -C SRC -j 16
	make -C JPT -j 16
	gcc -o get_Cost -O3 get_Cost.c  -Wno-unused-result
clean:
	make -C SRC clean
	make -C JPT clean
	rm -f get_Cost
	rm -f *~
