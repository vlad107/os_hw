all: cat

cat.o: cat.c
	gcc -c cat.c -o cat.o

cat: cat.o
	gcc cat.o -o cat

clean:
	rm -f cat.o cat
