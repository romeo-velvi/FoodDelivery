# makefile per programma
# versione 1

app: uno.c ../include/llist.o 
	gcc -g -o exe ../include/llist.o uno.c -pthread

../include/llist.o: ../include/llist.c ../include/llist.h
	gcc -c ../include/llist.c -o ../include/llist.o

clean:
	rm *.o output
