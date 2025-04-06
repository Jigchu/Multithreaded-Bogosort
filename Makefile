bogosort.exe: bogosort.o pcg_basic.o
	gcc bogosort.o pcg_basic.o -o bogosort.exe

clean:
	rm -f *.o

bogosort.o: bogosort.c
	gcc -c -Wall bogosort.c -o bogosort.o

pcg_basic.o: pcg_basic.c pcg_basic.h
	gcc -c pcg_basic.c -o pcg_basic.o