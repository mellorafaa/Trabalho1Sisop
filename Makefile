CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread

all: threads processos

threads:
	$(CC) $(CFLAGS) threads.c -o threads

processos:
	$(CC) $(CFLAGS) processos.c -o processos -lrt

run: all
	@echo "=============================="
	@echo " T1 - Threads SEM mutex"
	@echo "=============================="
	@for n in 2 4 8; do \
		echo "--- N=$$n ---"; \
		./threads $$n 1; \
	done

	@echo ""
	@echo "=============================="
	@echo " T2 - Threads COM mutex"
	@echo "=============================="
	@for n in 2 4 8; do \
		echo "--- N=$$n ---"; \
		./threads $$n 2; \
	done

	@echo ""
	@echo "=============================="
	@echo " P1 - Processos SEM semáforo"
	@echo "=============================="
	@for n in 2 4 8; do \
		echo "--- N=$$n ---"; \
		./processos $$n 1; \
	done

	@echo ""
	@echo "=============================="
	@echo " P2 - Processos COM semáforo"
	@echo "=============================="
	@for n in 2 4 8; do \
		echo "--- N=$$n ---"; \
		./processos $$n 2; \
	done

clean:
	rm -f threads processos