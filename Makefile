compiler = "gcc"

bogosort: bogosort.o pcg_basic.o
	$(compiler) -g bogosort.o pcg_basic.o -pthread -o bogosort

bogosort.o: bogosort.c
	$(compiler) -c -Wall -Wno-uninitialized bogosort.c -pthread -o bogosort.o

pcg_basic.o: pcg_basic.c pcg_basic.h
	$(compiler) -c pcg_basic.c -o pcg_basic.o

clean:
	rm -rf *.o
