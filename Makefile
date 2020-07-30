SHELL=/bin/sh
TARGET=swuniq
SRCS=swuniq.c
DESTDIR=
prefix=/usr/local/bin
INSTALL=install
INSTALL_PROGRAM=$(INSTALL)

CFLAGS ?= -O2 -pie
DEBUGFLAGS+=-Wall -Wextra -Wconversion -Wcast-qual -Wcast-align \
			-Wshadow -Wundef -Wstrict-overflow=5 -Wstrict-prototypes \
			-Wswitch-enum -Wredundant-decls -Wvla -Wnarrowing \
			-Wpointer-arith -Wformat-security -Wformat=2 \
			-Winit-self -Wfloat-equal -Wwrite-strings
CFLAGS += $(DEBUGFLAGS)

# Add support for xxHash dispatch
ifeq ($(DISPATCH),1)
	CFLAGS += -DXXHSUM_DISPATCH=1
	SRCS += xxHash/xxh_x86dispatch.c
endif

.PHONY: swuniq
swuniq: $(SRCS)
	mkdir -p out
	$(CC) $(CFLAGS) $(SRCS) -o out/$(TARGET)

static: $(SRCS)
	mkdir -p out
	$(CC) $(CFLAGS) -static $(SRCS) -o out/$(TARGET)-static

# ## dispatch only works for x86/x64 systems
# dispatch: CPPFLAGS += -DXXHSUM_DISPATCH=1
# dispatch: xxHash/xxhash.o xxHash/xxh_x86dispatch.o swuniq.c
# 	$(CC) $(CFLAGS) $^ $(LDFLAGS)
# xxh_x86dispatch.o: xxHash/xxh_x86dispatch.c xxHash/xxh_x86dispatch.h xxHash/xxhash.h

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
	@if [ "$$({ seq 1 10; seq 1 10; } | out/swuniq -w 10 | wc -l)" -eq 10 ]; then \
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
