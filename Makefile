CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread

all: threads processos

threads:
	$(CC) $(CFLAGS) threads.c -o threads

processos:
	$(CC) $(CFLAGS) processos.c -o processos -lrt

clean:
	rm -f threads processos