CC = gcc
CFLAGS = -Wall -Ofast -march=native
LDFLAGS = 

all: joy

joy: forth.cxx
	$(CC) -o $@ forth.cxx $(CFLAGS) $(LDFLAGS)
