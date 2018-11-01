vpath swuniq ./bin

all : swuniq.c xxhash.h
	mkdir -p bin
	$(CC) -O swuniq.c -o bin/swuniq

static : swuniq.c xxhash.h
	mkdir -p bin
	$(CC) -O -static swuniq.c -o bin/swuniq

install: swuniq all
	install bin/swuniq /usr/local/bin

.PHONY: clean
clean :
	rm -f bin/swuniq