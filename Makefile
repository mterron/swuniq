SHELL=/bin/sh
TARGET=swuniq
SRCS=swuniq.c xxhash.h
CFLAGS=-O2
DESTDIR=/usr/local/bin
INSTALL=install
INSTALL_PROGRAM=$(INSTALL)

swuniq: $(SRCS)
	mkdir -p out/$(DESTDIR)
	$(CC) $(CFLAGS) $(TARGET).c -o out/$(DESTDIR)/$(TARGET)

static: $(SRCS)
	mkdir -p out/$(DESTDIR)
	$(CC) $(CFLAGS) -static $(TARGET).c -o out/$(DESTDIR)/$(TARGET)-static

all: swuniq static
.PHONY: all

install: swuniq
	$(INSTALL_PROGRAM) -m 755 -D out/$(DESTDIR)/$(TARGET) $(DESTDIR)

install-static: static
	$(INSTALL_PROGRAM) -m 755 -D out/$(DESTDIR)/$(TARGET)-static $(DESTDIR)

install-all: swuniq static
	$(INSTALL_PROGRAM) -m 755 -D out/$(DESTDIR)/$(TARGET) bin/$(TARGET)-static $(DESTDIR)

install-strip:
	make INSTALL_PROGRAM='install -s' install

install-strip-static:
	make INSTALL_PROGRAM='install -s' install-static

install-strip-all:
	make INSTALL_PROGRAM='install -s' install
	make INSTALL_PROGRAM='install -s' install-static

uninstall:
	rm -f $(DESTDIR)/$(TARGET) $(DESTDIR)/$(TARGET)-static

clean:
	rm -rf out/
