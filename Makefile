#
# Makefile for JANFS client.
#

obj-$(CONFIG_JANFS_FS)  := janfs.o

janfs-objs += janfs_fs.o client_socket.o protocol.o

# TODO: Change path discovery. Do not use PWD
all:
	make -C $(PWD)/../../ M=$(PWD) modules

clean:
	make -C $(PWD)/../../ M=$(PWD) clean
