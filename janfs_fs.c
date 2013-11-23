//-----------------------------------------------------------------------------
//
// Just Another Network File System for Linux.
//
// Copyright (C) 2013 Luiz Gustavo Pereira da Silva.
//               2013 Gustavo Pantuza.
//
// This file is released under the GPL.
//
//-----------------------------------------------------------------------------
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/statfs.h>
#include <linux/backing-dev.h>
#include <asm/uaccess.h>
#include <linux/pagemap.h>
#include <linux/slab.h>

#include "protocol.h"
#include "socket.h"
#include "janfs_fs.h"

//-----------------------------------------------------------------------------
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Luiz Gustavo / Gustavo Pantuza");

//-----------------------------------------------------------------------------
#define JANFS_MAGIC 0xafafafaf
#define JANFS_BSIZE 1024

//-----------------------------------------------------------------------------
static const struct super_operations janfs_ops;
static const struct inode_operations janfs_dir_inode_operations;

//-----------------------------------------------------------------------------
// Local Functions Prototypes
//-----------------------------------------------------------------------------
static int janfs_statfs(struct dentry *dentry,
                        struct kstatfs *buf);
static struct inode *janfs_get_inode(struct super_block *sb,
				                     const struct inode *dir,
				                     umode_t mode,
				                     dev_t dev);
static void janfs_kill_sb(struct super_block *sb);




//-----------------------------------------------------------------------------
static struct backing_dev_info janfs_backing_dev_info = {
	.name		= "janfs",
	.ra_pages	= 0,	/* No readahead */
	.capabilities	= BDI_CAP_NO_ACCT_AND_WRITEBACK |
			  BDI_CAP_MAP_DIRECT | BDI_CAP_MAP_COPY |
			  BDI_CAP_READ_MAP | BDI_CAP_WRITE_MAP | BDI_CAP_EXEC_MAP,
};

//-----------------------------------------------------------------------------
static const struct inode_operations janfs_dir_inode_operations = {
//	.create		= janfs_create,
	.lookup		= simple_lookup,
	.link		= simple_link,
	.unlink		= simple_unlink,
//	.symlink	= janfs_symlink,
//	.mkdir		= janfs_mkdir,
	.rmdir		= simple_rmdir,
//	.mknod		= janfs_mknod,
	.rename		= simple_rename,
};

//-----------------------------------------------------------------------------
static const struct super_operations janfs_ops = {
	.statfs		= janfs_statfs,
	.drop_inode	= generic_delete_inode,
	.show_options	= generic_show_options,
};

//-----------------------------------------------------------------------------
const struct file_operations janfs_file_operations = {
	.read		= do_sync_read,
	.aio_read	= generic_file_aio_read,
	.write		= do_sync_write,
	.aio_write	= generic_file_aio_write,
	.mmap		= generic_file_mmap,
	.fsync		= noop_fsync,
	.splice_read	= generic_file_splice_read,
	.splice_write	= generic_file_splice_write,
	.llseek		= generic_file_llseek,
};

//-----------------------------------------------------------------------------
const struct inode_operations janfs_file_inode_operations = {
	.setattr	= simple_setattr,
	.getattr	= simple_getattr,
};

//-----------------------------------------------------------------------------
const struct address_space_operations janfs_aops = {
	.readpage	= simple_readpage,
	.write_begin	= simple_write_begin,
	.write_end	= simple_write_end,
//	.set_page_dirty = __set_page_dirty_no_writeback,
};

//-----------------------------------------------------------------------------
// The filesystem registry structure. Holds the name of the filesystem type
// and a function that can construct a superblock.
//-----------------------------------------------------------------------------
static struct file_system_type janfs_fstype = {
	.name		= "janfs",
	.mount		= janfs_mount,
	.kill_sb	= janfs_kill_sb,
	.fs_flags	= FS_USERNS_MOUNT,
};


//-----------------------------------------------------------------------------
// Read the superblock
//-----------------------------------------------------------------------------
int janfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode * inode = NULL;

	// Fill in the superblock
	sb->s_blocksize = PAGE_CACHE_SIZE;
	sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
	sb->s_magic = JANFS_MAGIC;
	sb->s_op = &janfs_ops;

	inode = janfs_get_inode(sb, NULL, S_IFDIR | 0755, 0);
	sb->s_root = d_make_root(inode);
	if (!sb->s_root)
		return -ENOMEM;

	return 0;
}

//-----------------------------------------------------------------------------
// Mount the filesystem
//-----------------------------------------------------------------------------
struct dentry *janfs_mount(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data)
{
	int ret = 0, remote_port = 0;
//	unsigned char buf[4096];
    char* remote_address = "255.255.255.255";
	const char* data_buf = dev_name; //strdup(dev_name);
	/* /// \TODO
	char* token = strsep(&data_buf, ":");
	if (!token) {
		printk("Erro spliting string [%s].\n", dev_name);
		goto out_err;
	}
	strncpy(remote_address, token, sizeof(remote_address));
	
	token = strtok(NULL, ":");
	if (!token) {
		printk("Erro spliting string [%s].\n", dev_name);
		goto out_err;
	}
	rv = kstrtoint(token, 10, &remote_port); 
	remote_port = simple_strtoul(token, 0, 10);
	*/
	
	
	printk("Mounting janfs with remote addr=%s and port=%d.\n", remote_address, remote_port);

	ret = create_client_socket();
	if (ret) {
		printk(KERN_ERR "Could not create client socket.\n");
		goto out_err;
	}

	// Connect to specified address
	ret = connect_server(remote_address, remote_port);
	if (ret) {
		printk(KERN_ERR "Could not connect to remote host.\n");
		goto out_err;
	}
	
	// Send mount command
	printk("Mount data[%s], dev_name[%s].\n", data_buf, dev_name);
	
    //if (srv_cmd(MOUNT_CMD, data_buf, strlen(data_buf), buf, sizeof(buf)) != 0) {
    //	printk(KERN_ERR "Could not send or receive the mount command.\n");
    //	goto out_err;
    //}
   
	// Fills superblock with information received from server


	return mount_nodev(fs_type, flags, data, janfs_fill_super);

out_err:
	return ERR_PTR(-ENOMEM);
}


//-----------------------------------------------------------------------------
// Mount the filesystem
//-----------------------------------------------------------------------------
static void janfs_kill_sb(struct super_block *sb)
{
	kfree(sb->s_fs_info);
	kill_litter_super(sb);
}



//-----------------------------------------------------------------------------
// Return information about an JANFS volume
//-----------------------------------------------------------------------------
int janfs_statfs(struct dentry *dentry, struct kstatfs *buf)
{
//	struct super_block *sb = dentry->d_sb;

	buf->f_type = JANFS_MAGIC;
	buf->f_bsize = JANFS_BSIZE;
//	buf->f_blocks = JANFS_SB(sb)->blocks;
//	buf->f_bfree = 0;
//	buf->f_bavail = 0;
//	buf->f_files = JANFS_SB(sb)->files;
//	buf->f_ffree = 0;
	buf->f_namelen = 256;
	return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
struct inode *janfs_get_inode(struct super_block *sb,
				const struct inode *dir, umode_t mode, dev_t dev)
{
	struct inode * inode = new_inode(sb);

	if (inode) {
		inode->i_ino = get_next_ino();
		inode_init_owner(inode, dir, mode);
		inode->i_mapping->a_ops = &janfs_aops;
		inode->i_mapping->backing_dev_info = &janfs_backing_dev_info;
		mapping_set_gfp_mask(inode->i_mapping, GFP_HIGHUSER);
		mapping_set_unevictable(inode->i_mapping);
		inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
		switch (mode & S_IFMT) {
		default:
			init_special_inode(inode, mode, dev);
			break;
		case S_IFREG:
			inode->i_op = &janfs_file_inode_operations;
			inode->i_fop = &janfs_file_operations;
			break;
		case S_IFDIR:
			inode->i_op = &janfs_dir_inode_operations;
			inode->i_fop = &simple_dir_operations;

			// directory inodes start off with i_nlink == 2 (for "." entry)
			inc_nlink(inode);
			break;
		case S_IFLNK:
			inode->i_op = &page_symlink_inode_operations;
			break;
		}
	}
	return inode;
}




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------














/* 
 * Attaches the new filesystem to the kernel.
 */
static int __init janfs_init(void)
{
	printk("Registering janfs filesystem.\n");

	return register_filesystem(&janfs_fstype);
}

/* 
 * Removes the filesystem from the kernel.
 */
static void __exit janfs_exit(void)
{
	printk("Removing janfs filesystem. Closing client socket.\n");

	close_client_socket();

	unregister_filesystem(&janfs_fstype);
}

module_init(janfs_init)
module_exit(janfs_exit)



