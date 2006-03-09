CC = gcc
CFLAGS = -Wall

all: cooc3d

cooc3d: cooc3d.o pnf_cooc.o
	$(CC) -o cooc3d cooc3d.o pnf_cooc.o

cooc3d.o: cooc3d.c pnf_cooc.h

pnf_cooc.o: pnf_cooc.c pnf_cooc.h

clean:
	- rm cooc3d cooc3d.o pnf_cooc.o
