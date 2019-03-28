vpath swuniq ./bin

all : swuniq.c xxhash.h
	mkdir -p bin
	$(CC) -O2 swuniq.c -o bin/swuniq

static : swuniq.c xxhash.h
	mkdir -p bin
	$(CC) -O2 -static swuniq.c -o bin/swuniq

install: swuniq all
	install bin/swuniq /usr/local/bin

.PHONY: clean
clean :
	rm -f bin/swuniq
