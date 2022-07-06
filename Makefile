CC=gcc -m32
CFLAGS=-std=c99 -Wall -Wextra
LOAD=load_balancer
SERVER=server
LIST=linked_list
HASHRING=hash_ring

.PHONY: build clean

build: tema2

tema2: main.o $(LOAD).o $(HASHRING).o $(SERVER).o $(LIST).o 
	$(CC) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -c

$(LIST).o: $(LIST).c $(LIST).h
	$(CC) $(CFLAGS) $^ -c

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(HASHRING).o: $(HASHRING).c $(HASHRING).h
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c

pack:
	zip -FSr 311CA_Olteanu_MariaTeona_tema2.zip Makefile README *.c *.h

clean:
	rm -f *.o tema2 *.h.gch
