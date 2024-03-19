CC = gcc
CFLAGS = -Wall -Ofast -march=native -pedantic
LDFLAGS = 

all: joy

joy: forth.cxx
	$(CC) -o $@ forth.cxx $(CFLAGS) $(LDFLAGS)
