CFLAGS=-std=c11 -g -static

9cc: 9cc.c
	gcc 9cc.c -o 9cc $(CFLAGS)

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*