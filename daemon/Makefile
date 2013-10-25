<<<<<<< HEAD
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
=======
#
# Makefile for JANFS client.
#

obj-$(CONFIG_JANFS_FS)  := janfs.o

janfs-objs += client_socket.o
>>>>>>> b4f7f982116475be1a3d0f6169932d941f99a756
