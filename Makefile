#!/usr/bin/make -f

TARGET = sftponly
OBJS = $(TARGET)
HEADS = 

CC = gcc -std=gnu99 
LD = gcc
CFLAGS := -g -Wall
LDFLAGS = -ldl

.PHONY: clean run all

all: $(TARGET)

%.o: %.c $(HEADS)
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(TARGET).o
	$(LD) $(filter %.o, $^) -o $@ $(LDFLAGS)
	sudo chown root:root $@
	sudo chmod u+s $@

tags: $(addsuffix .c, $(OBJS)) $(HEADS)
	ctags $^

run: $(TARGET)
	./$<

clean:
	rm -f core *.so *.o tags $(TARGET)
