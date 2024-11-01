include ./cpu.mk

all: flush_reload spectre

run_flush: flush_reload
	@taskset -c $(SENDER_CPU) ./flush_reload

run_spectre: spectre
	@taskset -c $(SENDER_CPU) ./spectre

flush_reload: flush_reload.c Makefile
	@gcc flush_reload.c -o flush_reload

spectre: spectre.c Makefile
	@gcc spectre.c -o spectre

.PHONY: clean

clean:
	rm -f flush_reload spectre
