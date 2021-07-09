# makefile per programma
# versione 1

app: Server.c ../include/llist.o 
	gcc -g -o exe ../include/llist.o Server.c

../include/llist.o: ../include/llist.c ../include/llist.h
	gcc -c ../include/llist.c -o ../include/llist.o

clean:
	rm *.o output
