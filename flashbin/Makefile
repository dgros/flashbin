# Makefile for the flashbin project
CC=gcc
LIBS= -lpthread
FLAGS= -Wall -D_REENTRANT
OBJ=flashbin.o
PROG=flashbin

all: $(PROG)

$(PROG): $(OBJ)
	$(CC) $(LIBS) -o $@ $^

%.o: %.c
	$(CC) -c $< $(FLAGS)  $(LIBS)

clean:
	rm -f $(OBJ) $(PROG)
