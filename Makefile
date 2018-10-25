all : swuniq.c xxhash.h
	gcc -O swuniq.c -o swuniq

static : swuniq.c xxhash.h
	$(CC) -O -static swuniq.c -o swuniq

install: swuniq
	install swuniq /usr/local/bin

clean :
	rm -f swuniq