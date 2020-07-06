SHELL=/bin/sh
TARGET=swuniq
SRCS=swuniq.c xxhash.h
CFLAGS=-O2
DESTDIR=/usr/local/bin
INSTALL=install
INSTALL_PROGRAM=$(INSTALL)

swuniq: $(SRCS)
	mkdir -p out
	$(CC) $(CFLAGS) $(TARGET).c -o out/$(TARGET)

static: $(SRCS)
	mkdir -p out
	$(CC) $(CFLAGS) -static $(TARGET).c -o out/$(TARGET)-static

all: swuniq static
.PHONY: all

install: swuniq
	mkdir -p $(DESTDIR)
	$(INSTALL_PROGRAM) -m 755 out/$(TARGET) $(DESTDIR)

install-static: static
	mkdir -p $(DESTDIR)
	$(INSTALL_PROGRAM) -m 755 out/$(TARGET)-static $(DESTDIR)

install-all: swuniq static
	mkdir -p $(DESTDIR)
	$(INSTALL_PROGRAM) -m 755 out/$(TARGET) out/$(TARGET)-static $(DESTDIR)

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
