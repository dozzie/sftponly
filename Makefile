#!/usr/bin/make -f

TARGET = sftponly
OBJS = $(TARGET)
HEADS = 

SFTPSERVER_DIR =

OPENSSH_VERSION = 5.1p1
OPENSSH_DIR     = openssh-$(OPENSSH_VERSION)
OPENSSH_TARBALL = $(OPENSSH_DIR).tar.gz

CC = gcc -std=gnu99 
LD = gcc
ifneq ($(SFTPSERVER_DIR),)
CPPFLAGS = -D'SFTP_SERVER="$(SFTPSERVER_DIR)/sftp-server.so"'
endif
CFLAGS := -g -Wall
LDFLAGS = -ldl

.PHONY: clean run all

all: $(TARGET)

sftp-server.so: $(OPENSSH_DIR)/sftp-server.so
	cp $< $@

$(OPENSSH_DIR)/sftp-server.so: $(OPENSSH_DIR)/Makefile
	patch -p1 -d $(OPENSSH_DIR) < sftp-server.patch
	make sftp-server.so 'LD=$(CC) -shared -fPIC' 'CC=$(CC) -fPIC' EXEEXT=.so -C $(OPENSSH_DIR)

$(OPENSSH_DIR)/Makefile: $(OPENSSH_TARBALL)
	tar zxf $<
	cd $(OPENSSH_DIR) && ./configure --prefix=/usr

%.o: %.c $(HEADS)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

$(TARGET): $(TARGET).o
	$(LD) $(filter %.o, $^) -o $@ $(LDFLAGS)

tags: $(addsuffix .c, $(OBJS)) $(HEADS)
	ctags $^

run: $(TARGET)
	./$<

clean:
	rm -f core *.so *.o tags $(TARGET)
	rm -rf $(OPENSSH_DIR)
