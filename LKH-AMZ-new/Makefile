# CC=gcc
all:
	make -C SRC -j 16
	make -C JPT -j 16
	$(CC) -o get_Length -O3 get_Length.c -Wno-unused-result
clean:
	make -C SRC clean
	make -C JPT clean
	rm -f get_Length
	rm -f *~
