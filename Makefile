all: main.o crud.o
	gcc main.o crud.o -o exec -std=c99 -g -Wall

crud.o:
	gcc -c crud.c -o crud.o

main.o:
	gcc -c main.c -o main.o

clean:
	rm *.o exec

run:
	./exec
#valgrind --leak-check=yes -s --track-origins=yes ./exec