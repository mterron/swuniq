TARGET=swuniq
SRCS=swuniq.c xxhash.h

all: $(SRCS)
	mkdir -p bin
	$(CC) -O2 $(TARGET).c -o bin/$(TARGET)

static: $(SRCS)
	mkdir -p bin/static/
	$(CC) -O2 -static $(TARGET).c -o bin/static/$(TARGET)

install: swuniq all
	install -D bin/$(TARGET) $(DESTDIR)/usr/local/bin/$(TARGET)

clean:
	rm -rf ./bin

