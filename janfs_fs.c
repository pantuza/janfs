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

#include "socket.h"
#include "janfs_fs.h"

//-----------------------------------------------------------------------------
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Luiz Gustavo / Gustavo Pantuza");

//-----------------------------------------------------------------------------
#define JANFS_MAGIC 0xafafafaf
#define JANFS_BSIZE 1024

//-----------------------------------------------------------------------------
static const struct super_operations janfs_super_operations;
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
// Local Variables
//-----------------------------------------------------------------------------
static char mount_path[512];





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
    .readpage		= simple_readpage,
    .write_begin	= simple_write_begin,
    .write_end		= simple_write_end,
//	.set_page_dirty = __set_page_dirty_no_writeback,
};

//-----------------------------------------------------------------------------
// The filesystem registry structure. Holds the name of the filesystem type
// and a function that can construct a superblock.
//-----------------------------------------------------------------------------
static struct file_system_type janfs_fstype = {
    .owner      = THIS_MODULE,
    .name		= "janfs",
    .mount		= janfs_mount,
    .kill_sb	= janfs_kill_sb,
    .fs_flags	= FS_USERNS_MOUNT,
};

//-----------------------------------------------------------------------------
static const struct super_operations janfs_super_operations = {
    .statfs			= janfs_statfs,
    .drop_inode		= generic_delete_inode,
    .show_options	= generic_show_options,
};



/*
void mount_cmd(char* data)
{
    unsigned char recv_buf[512];
    unsigned short recv_size = 0;

    printk("mount_cmd INI: data[%s].\n", data);
    recv_size = sizeof(recv_buf);
    if (srv_cmd(MOUNT_CMD, data, strlen(data), recv_buf, recv_size) != 0) {
        printk(KERN_ERR "   Could not send or receive the mount command.\n");
        return;
    }

    // Set root inode
}
*/



//-----------------------------------------------------------------------------
// Read and popuplate directory
//-----------------------------------------------------------------------------
void janfs_read_dir(char* path, struct tree_descr** file_list)
{
    unsigned short nfiles = 0;
    printk("janfs_read_dir INI: path[%s].\n", path);
    srv_read_dir(path, file_list, &nfiles);
    printk(" janfs_read_dir returned [%d] files in directory [%s].\n", nfiles, path);
}

//-----------------------------------------------------------------------------
// Read the superblock
//-----------------------------------------------------------------------------
int janfs_fill_super(struct super_block *sb, void *data, int silent)
{
/*
    struct inode* i = NULL;

    // Fill in the superblock
    sb->s_blocksize = PAGE_CACHE_SIZE;
    sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
    sb->s_magic = JANFS_MAGIC;
    sb->s_flags |= MS_NOATIME; // Do not update access times
    sb->s_op = &janfs_super_operations;
    //sb->s_d_op = &janfs_dentry_operations;

    i = janfs_get_inode(sb, NULL, S_IFDIR | 0755, 0);

    sb->s_root = d_make_root(i);
    if (!sb->s_root)
        return -ENOMEM;

    return 0;
*/
    struct tree_descr *root_files = NULL;

    // Read root directory
    janfs_read_dir(mount_path, &root_files);

    // Populates root directory
    return simple_fill_super(sb, JANFS_MAGIC, root_files);
}


//-----------------------------------------------------------------------------
// Mount the filesystem
// From docs: the mount() method must return the root dentry of the tree
// requested by caller.  An active reference to its superblock must be
// grabbed and the superblock must be locked.  On failure it should return
// ERR_PTR(error).
//-----------------------------------------------------------------------------
struct dentry *janfs_mount(struct file_system_type *fs_type,
    int flags, const char *dev_name, void *data)
{
    int ret = 0, remote_port = 0;
    char remote_address[32];
    char* token = NULL;
    char* data_buf = NULL;

    printk("Janfs mounting filesystem: data[%s], dev_name[%s].\n", (char*)data, dev_name);

    data_buf = kstrdup(dev_name, GFP_KERNEL);
    if (!data_buf) {
        printk("   Error duplicating string [%s].\n", dev_name);
        goto out_err_ret;
    }

    // Remote address
    token = strsep(&data_buf, ":");
    if (!token) {
        printk("   Error spliting string [%s].\n", data_buf);
        goto out_err;
    }
    strncpy(remote_address, token, sizeof(remote_address));

    // Remote port
    token = strsep(&data_buf, ":");
    if (!token) {
        printk("   Error spliting string [%s].\n", data_buf);
        goto out_err;
    }
    if (kstrtoint(token, 10, &remote_port) != 0) {
        printk("   Error parsing integer from string [%s].\n", token);
        goto out_err;
    }

    // Copy remote mount path
    token = strsep(&data_buf, ":");
    if (!token) {
        printk("   Error spliting string [%s].\n", data_buf);
        goto out_err;
    }
    strncpy(mount_path, token, sizeof(mount_path));

    // Free duplicated string
    kfree(data_buf);

    printk("Mounting janfs with remote addr=%s, port=%d and path=%s.\n",
           remote_address, remote_port, token);

    ret = create_client_socket();
    if (ret) {
        printk(KERN_ERR "   Could not create client socket.\n");
        goto out_err;
    }

    // Connect to specified address
    ret = connect_server(remote_address, remote_port);
    if (ret) {
        printk(KERN_ERR "   Could not connect to remote host.\n");
        goto out_err;
    }

    return mount_nodev(fs_type, flags, data, janfs_fill_super);

out_err:
    kfree(data_buf);
out_err_ret:
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
        //mapping_set_unevictable(inode->i_mapping);
        inode->i_flags |= S_NOATIME|S_NOCMTIME;
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
    printk("Removing janfs filesystem.\n");

    close_client_socket();

    unregister_filesystem(&janfs_fstype);
}

module_init(janfs_init)
module_exit(janfs_exit)



