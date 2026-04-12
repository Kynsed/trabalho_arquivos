all: main.o crud.o
	gcc main.o crud.o -o programaTrab -std=c99 -g

crud.o:
	gcc -c crud.c -o crud.o

main.o:
	gcc -c main.c -o main.o

clean:
	rm *.o programaTrab

run:
	./programaTrab
#valgrind --leak-check=yes -s --track-origins=yes ./programaTrab
