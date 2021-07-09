# makefile per programma
# versione 1

app: Client.c ../include/llist.o 
	gcc -g -o exe ../include/llist.o Client.c

../include/llist.o: ../include/llist.c ../include/llist.h
	gcc -c ../include/llist.c -o ../include/llist.o

clean:
	rm *.o output
