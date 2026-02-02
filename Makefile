CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

all: rtimer

rtimer: src/rtimer.c
	$(CC) $(CFLAGS) -o rtimer src/rtimer.c

install: rtimer
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 rtimer $(DESTDIR)$(BINDIR)/rtimer

clean:
	rm -f rtimer
