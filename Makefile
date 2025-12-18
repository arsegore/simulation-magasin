CC=gcc
CFLAGS=-Iinc -Wall

all: initial client vendeur caissier

initial: bin cle.o sem.o smp.o logs.o msg.o
	$(CC) $(CFLAGS) src/initial.c build/cle.o build/sem.o build/smp.o build/logs.o build/msg.o -o bin/initial

client: bin cle.o sem.o smp.o logs.o msg.o
	$(CC) $(CFLAGS) src/client.c build/cle.o build/sem.o build/smp.o build/logs.o build/msg.o -o bin/client

vendeur: bin cle.o sem.o smp.o logs.o msg.o
	$(CC) $(CFLAGS) src/vendeur.c build/cle.o build/sem.o build/smp.o build/logs.o build/msg.o -o bin/vendeur

caissier: bin cle.o sem.o smp.o logs.o msg.o
	$(CC) $(CFLAGS) src/caissier.c build/cle.o build/sem.o build/smp.o build/logs.o build/msg.o -o bin/caissier

cle.o: build
	$(CC) $(CFLAGS) src/cle/cle.c -o build/cle.o -c

sem.o: build
	$(CC) $(CFLAGS) src/semaphore/sem.c -o build/sem.o -c

smp.o: build
	$(CC) $(CFLAGS) src/smp/smp.c -o build/smp.o -c

logs.o: build
	$(CC) $(CFLAGS) src/logs.c -o build/logs.o -c

msg.o:
	$(CC) $(CFLAGS) src/msg/msg.c -o build/msg.o -c

bin:
	mkdir -p bin

build:
	mkdir -p build

clean:
	rm -rf bin build