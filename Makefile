bogosort.exe: bogosort.o pcg_basic.o
	gcc -g bogosort.o pcg_basic.o -pthread -lgmp -o bogosort.exe

clean:
	rm -rf *.o

bogosort.o: bogosort.c
	gcc -c -Wall bogosort.c -pthread -lgmp -o bogosort.o

pcg_basic.o: pcg_basic.c pcg_basic.h
	gcc -c pcg_basic.c -o pcg_basic.o