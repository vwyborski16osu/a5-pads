main:
	gcc -std=gnu99 -Wall -g -pthread -o keygen keygen.c

clean:
	rm -f keygen