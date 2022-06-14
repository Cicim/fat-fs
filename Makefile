# Authored by Cicim and Claziero

CC = gcc
CCOPTS = --std=gnu99 -Wall 
TESTER_CCOPTS = $(CCOPTS) -Wno-unused-label

USER_HEADERS = libfat/fat.h
LIB_HEADERS = libfat/internals.h

LIBS = libfat/libfat.a

BINS = fat_man tester fat_test


all:
	make -C libfat --no-print-directory
	make $(BINS) --no-print-directory

fat_man: $(USER_HEADERS) $(LIBS) fat_man.c
	$(CC) $(CCOPTS) -o $@ fat_man.c $(LIBS)

fat_test: $(LIB_HEADERS) $(LIBS) fat_test.c
	$(CC) $(CCOPTS) -o $@ fat_test.c $(LIBS)

tester: $(LIB_HEADERS) $(LIBS) tester.c
	$(CC) $(TESTER_CCOPTS) -o $@ tester.c $(LIBS)

clean:
	rm -rf *.o *.dat $(BINS)
	make -C libfat clean --no-print-directory
