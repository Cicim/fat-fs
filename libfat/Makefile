# Authored by Cicim and Claziero

CC = gcc
CCOPTS = --std=gnu99 -Wall 
AR = ar

HEADERS = fat.h\
	internals.h

OBJS = dir_change.o\
	dir_create.o\
	dir_erase.o\
	dir_list.o\
	file_create.o\
	file_erase.o\
	file_read.o\
	file_seek.o\
	file_write.o\
	internals.o

LIBS = libfat.a

libfat.a: $(OBJS) $(HEADERS)
	$(AR) -rcs $@ $^

%.o:	%.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@  $<

clean:
	rm -rf *.o $(LIBS)
