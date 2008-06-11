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

install:
	cp flashbin.sh /usr/local/bin
	cp flashbin /usr/local/bin
	cp flashbin.conf /etc/flashbin.conf
	cp modification.sh /usr/local/bin
