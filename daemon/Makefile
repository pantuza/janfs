# 
# Janfs Makefile
#


# Compilation target (binary file)
daemon_out := janfsd


CC := gcc
DEBUG := -g 
CFLAGS := -std=gnu99 -Wall -O3


all: daemon
	@echo -e "Building entire project.."


daemon: daemon.c
	@echo -e "Building daemon janfsd.."
	$(CC) $< $(CFLAGS) -o $(daemon_out) 


clean:
	@rm -rvf *.o
	@rm -rvf $(daemon_out)
