# Authored by Cicim and Claziero

CC = gcc
CCOPTS = --std=gnu99 -Wall 

HEADERS = libfat/fat.h

OBJS = fat_man.o

LIBS = libfat/libfat.a

BINS = fat_man

fat_man: $(OBJS) $(HEADERS) $(LIBS)
	$(CC) $(CCOPTS) -o $@ $(OBJS) $(LIBS)

%.o:	%.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@  $<

libfat/libfat.a:
	make -C libfat

clean:
	rm -rf *.o $(BINS)
	make -C libfat clean
