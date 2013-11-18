# 
# Janfs Makefile
#


# Compilation target (binary file)
daemon_out := janfsd


CC := gcc
DEBUG := -g 
CFLAGS := -std=gnu99 -Wall -O3


all: daemon

daemon: daemon.c listener.o
	@echo -e "\n== Building and linking daemon janfsd ==\n"
	$(CC) $^ $(CFLAGS) -o $(daemon_out) 

%.o: %.c
	@echo -e "\n== Building Objects ==\n"
	$(CC) -c $^ -o $@ $(CFLAGS)

clean:
	@echo -e "\n== Cleaning the Project ==\n"
	@rm -rvf *.o
	@rm -rvf $(daemon_out)
