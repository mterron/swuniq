vpath swuniq ./bin

all : swuniq.c
	mkdir -p bin
	$(CC) -O2 swuniq.c -o bin/swuniq -march=native -mtune=native

static : swuniq.c xxhash.h
	mkdir -p bin/static/
	$(CC) -O2 -static swuniq.c -o bin/static/swuniq -march=native -mtune=native

install: swuniq all
	install -D bin/swuniq $(DESTDIR)/usr/local/bin/swuniq

.PHONY: clean
clean :
	rm -f bin/swuniq
