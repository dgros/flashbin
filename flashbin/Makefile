OBJ= flashbin.c syslog.c
CC=gcc
CFLAGS= -Wall
LDFLAGS= -lpthread
CIB= flashbin

$(CIB) : $(OBJ)
	$(CC) -o $(CIB) $(OBJ) $(LDFLAGS)

#%.o: %.c
#	$(CC) -c $@ $(CFLAGS) $(LDFLAGS) $<

clean:
	rm *.o
	rm $(CIB)



