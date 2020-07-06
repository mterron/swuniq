SHELL=/bin/sh
TARGET=swuniq
SRCS=swuniq.c xxhash.h
CFLAGS=-O2
DESTDIR=
prefix=/usr/local/bin
INSTALL=install
INSTALL_PROGRAM=$(INSTALL)

swuniq: $(SRCS)
	mkdir -p out
	$(CC) $(CFLAGS) $(TARGET).c -o out/$(TARGET)

static: $(SRCS)
	mkdir -p out
	$(CC) $(CFLAGS) -static $(TARGET).c -o out/$(TARGET)-static

.PHONY: all
all: swuniq static


install: swuniq
	mkdir -p $(DESTDIR)$(prefix)
	$(INSTALL_PROGRAM) -m 755 out/$(TARGET) $(DESTDIR)$(prefix)

install-static: static
	mkdir -p $(DESTDIR)$(prefix)
	$(INSTALL_PROGRAM) -m 755 out/$(TARGET)-static $(DESTDIR)$(prefix)

install-all: swuniq static
	mkdir -p $(DESTDIR)$(prefix)
	$(INSTALL_PROGRAM) -m 755 out/$(TARGET) out/$(TARGET)-static $(DESTDIR)$(prefix)

install-strip:
	make INSTALL_PROGRAM='install -s' install

install-strip-static:
	make INSTALL_PROGRAM='install -s' install-static

install-strip-all:
	make INSTALL_PROGRAM='install -s' install
	make INSTALL_PROGRAM='install -s' install-static

.PHONY: check
check:
	@if [ "$$({ seq 1 10; seq 1 10; } | out/swuniq -w 10 | wc -l)" -eq 10 ]; then  \
		echo 'Test suite result [SUCCESS]' \
		exit 0 ;\
	else \
		echo 'Test suite result [FAIL]' \
		exit 1 ;\
	fi


uninstall:
	rm -f $(DESTDIR)$(prefix)/$(TARGET) $(DESTDIR)$(prefix)/$(TARGET)-static

clean:
	rm -rf out/
