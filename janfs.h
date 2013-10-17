#ifndef _LINUX_JANFS_H
#define _LINUX_JANFS_H

struct dentry *janfs_mount(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data);

int janfs_fill_super(struct super_block *sb, void *data, int silent);

#endif
