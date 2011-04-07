#!/usr/bin/make -f

TARGET = sftponly
OBJS = $(TARGET)
HEADS = 

PREFIX = /usr/local
LIBCHROOT_PATH = $(PREFIX)/lib/sftponly/libchroot.so

CC = gcc -std=gnu99 
LD = gcc
CFLAGS := -g -Wall
LDFLAGS =

#-----------------------------------------------------------------------------

.PHONY: all install

all: libchroot.so sftponly

install: all
	install -D -m 4755 sftponly     $(DESTDIR)$(PREFIX)/bin/sftponly
	install -D -m  755 libchroot.so $(DESTDIR)$(LIBCHROOT_PATH)

#-----------------------------------------------------------------------------
# linking

sftponly: sftponly.o
	$(LD) $(LDFLAGS) -fPIC $^ -o $@

libchroot.so: LDFLAGS += -ldl
libchroot.so: chroot.o open_hack.o getpwuid_hack.o
	$(LD) $(LDFLAGS) -shared -fPIC $^ -o $@

#-----------------------------------------------------------------------------
# object file dependencies and compilation rules

sftponly.o: CFLAGS += -D'LIBCHROOT_PATH="$(LIBCHROOT_PATH)"'
sftponly.o: get_login.h

chroot.o: getpwuid_hack.h open_hack.h
open_hack.o: open_hack.h
getpwuid_hack.o: getpwuid_hack.h get_login.h

%.o: %.c
	$(CC) -c -o $@ -fPIC $(CFLAGS) $(filter %.c,$^)

#-----------------------------------------------------------------------------

tags: $(wildcard *.c *.h)
	ctags $^

.PHONY: clean
clean:
	rm -f core *.so *.o tags $(TARGET)

srpm: VERSION=$(shell awk '$$1 == "%define" && $$2 == "_version" {print $$3}' redhat/sftponly.spec)
srpm:
	rm -rf rpm-build
	mkdir -p rpm-build/rpm/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
	git archive --format=tar --prefix=sftponly-$(VERSION)/ HEAD | gzip -9 > rpm-build/rpm/SOURCES/sftponly-$(VERSION).tar.gz
	rpmbuild --define="%_usrsrc $$PWD/rpm-build" --define="%_topdir %{_usrsrc}/rpm" -bs redhat/sftponly.spec
	mv rpm-build/rpm/SRPMS/sftponly-*.src.rpm .
	rm -r rpm-build

#-----------------------------------------------------------------------------
# vim:ft=make:ts=4:noet
