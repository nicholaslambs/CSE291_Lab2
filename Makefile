include ./cpu.mk

TARGETS = flush_reload.o spectre.o

CC = gcc
CFLAGS = -I.

all: $(TARGETS)

# Rules to build object files
flush_reload.o: ./flush+reload/flush_reload.c utility.h
	$(CC) $(CFLAGS) ./flush+reload/flush_reload.c -o flush_reload.o

spectre.o: ./spectre/spectre.c utility.h
	$(CC) $(CFLAGS) ./spectre/spectre.c -o spectre.o

# Run commands
run_flush: flush_reload.o
	@taskset -c $(SENDER_CPU) ./flush_reload.o

run_spectre: spectre.o
	@taskset -c $(SENDER_CPU) ./spectre.o

# Clean up generated files
.PHONY: clean
clean:
	rm -f flush_reload.o spectre.o
