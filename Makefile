all: allocator.so tests

test: all
	./tests

allocator.so: allocator.c
	gcc -o allocator.so -Wall -fPIC -shared allocator.c
	

tests: tests.c allocator.c
	gcc -Wall tests.c -o tests

clean:
	rm -f allocator.so tests
