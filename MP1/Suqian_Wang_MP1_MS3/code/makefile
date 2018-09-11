# makefile

all: memtest

my_allocator.o : my_allocator.c my_allocator.h
	g++ -c -g my_allocator.c

free_list.o : free_list.c free_list.h
	g++ -c -g free_list.c

memtest: memtest.c my_allocator.o free_list.o
	g++ -o memtest memtest.c my_allocator.o free_list.o

