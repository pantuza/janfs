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
#include <linux/fs.h>
#include <linux/backing-dev.h>
#include <asm/uaccess.h>
#include "janfs.h"

//-----------------------------------------------------------------------------
static const struct super_operations janfs_ops;
static const struct inode_operations janfs_dir_inode_operations;

//-----------------------------------------------------------------------------
/*
static struct backing_dev_info janfs_backing_dev_info = {
	.name		= "janfs",
	.ra_pages	= 0,	// No readahead
	.capabilities	= BDI_CAP_NO_ACCT_AND_WRITEBACK | BDI_CAP_MAP_DIRECT |
			  BDI_CAP_MAP_COPY | BDI_CAP_READ_MAP |
                          BDI_CAP_WRITE_MAP | BDI_CAP_EXEC_MAP,
};
*/

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
static const struct super_operations ramfs_ops = {
	.statfs		= simple_statfs,
	.drop_inode	= generic_delete_inode,
	.show_options	= generic_show_options,
};

//-----------------------------------------------------------------------------
static struct file_system_type janfs_fstype = {
	.name		= "janfs",
	.mount		= janfs_mount,
//	.kill_sb	= janfs_kill_sb,
	.fs_flags	= FS_USERNS_MOUNT,
};

//-----------------------------------------------------------------------------
struct dentry *janfs_mount(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data)
{
	/// \todo Substituir pela inicializacacao correta

	return mount_nodev(fs_type, flags, data, janfs_fill_super);
}

//-----------------------------------------------------------------------------
int janfs_fill_super(struct super_block *sb, void *data, int silent)
{
	/// \todo

	return -1;  // error
}












//-----------------------------------------------------------------------------
static int __init init_janfs(void)
{
	return register_filesystem(&janfs_fstype);
}

module_init(init_janfs)


