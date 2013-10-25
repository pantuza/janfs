#
# Makefile for JANFS client.
#

obj-$(CONFIG_JANFS_FS)  := janfs.o

janfs-objs += client_socket.o
