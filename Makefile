# Authored by Cicim and Claziero

CC = gcc
CCOPTS = --std=gnu99 -Wall 

HEADERS = libfat/fat.h
HEADERS_TEST = libfat/internals.h

OBJS = fat_man.o
OBJS_TEST = fat_test.o
	
LIBS = libfat/libfat.a

BINS = fat_man
BINS_TEST = fat_test


all:
	make -C libfat
	make fat_man
	make fat_test

fat_man: $(OBJS) $(HEADERS) $(LIBS)
	$(CC) $(CCOPTS) -o $@ $(OBJS) $(LIBS)

fat_test: $(OBJS_TEST) $(HEADERS_TEST) $(LIBS)
	$(CC) $(CCOPTS) -o $@ $(OBJS_TEST) $(LIBS)

internals.o:	internals.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@  $<

%.o:	%.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@  $<

clean:
	rm -rf *.o *.dat $(BINS) $(BINS_TEST)
	make -C libfat clean
