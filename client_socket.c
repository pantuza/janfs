#include <linux/net.h>
#include <linux/socket.h>
#include <linux/tcp.h>
#include <linux/slab.h>
#include <net/sock.h>
#include <net/tcp.h>

#include "protocol.h"
#include "socket.h"

//-----------------------------------------------------------------------------
// Local functions prototypes.
//-----------------------------------------------------------------------------

// Returns a 4-byte integer address to connect to.
static uint32_t to_address(char *address);

// Sends a command to server and returns the response.
int srv_cmd(int cmd, const unsigned char* data_buf,
            unsigned short data_size, unsigned char **recv_buf,
            unsigned short *recv_size);

//-----------------------------------------------------------------------------
// Local variables
//-----------------------------------------------------------------------------
static struct socket *client_socket;





//-----------------------------------------------------------------------------
uint32_t to_address(char *address)
{
    unsigned int p1, p2, p3, p4;

    if (sscanf(address,"%u.%u.%u.%u",&p1,&p2,&p3,&p4) != 4) {
        printk(KERN_ERR "Error reading address %s.\n", address);
        return 0;
    }

    return ((p1 << 24) + (p2 << 16) + (p3 << 8) + p4);
}

//-----------------------------------------------------------------------------
int create_client_socket()
{
    int ret;

    ret = sock_create_kern(AF_INET, SOCK_STREAM, IPPROTO_TCP, &client_socket);
    if (ret < 0) {
        printk(KERN_ERR "Error creating client_socket.\n");
    }

    return 0;
/*
   // Server side
    ret = socket->ops->bind(socket,(struct sockaddr*) &saddr,sizeof(saddr));
    if (ret < 0) {
        printk(KERN_ERR "Error binding socket.");
        return ret;
    }
    ret = socket->ops->listen(socket, 1);
    if (ret != 0) {
        printk(KERN_ERR "Error listening on socket.");
        return ret;
    }
*/
}

//-----------------------------------------------------------------------------
int connect_server(char *address, int port)
{
    struct sockaddr_in saddr;
    int ret;

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family          = AF_INET;
    saddr.sin_port            = htons(port);
    saddr.sin_addr.s_addr     = htonl(to_address(address));
    client_socket->sk->sk_allocation = GFP_NOFS;

    ret = client_socket->ops->connect(client_socket, (struct sockaddr *)&saddr, sizeof(saddr), O_RDWR);
    if (ret < 0) {
        printk(KERN_ERR "Error connecting to server.\n");
        return ret;
    }

    return 0;
}

//-----------------------------------------------------------------------------
void close_client_socket()
{
    if (client_socket && client_socket->ops) {
        printk("Closing client socket.\n");
        sock_release(client_socket);
        //client_socket->ops->shutdown(client_socket, 0);
        //client_socket->ops->release(client_socket);
    }
}

//-----------------------------------------------------------------------------
int send_srv_msg(unsigned char* msg, unsigned short len)
{
    int ret;
    mm_segment_t oldfs;
    struct msghdr sndmsg;
    struct iovec  iov;

    sndmsg.msg_name = 0;
    sndmsg.msg_namelen = 0;
    sndmsg.msg_iov = &iov;
    sndmsg.msg_iovlen = 1;
    sndmsg.msg_control = NULL;
    sndmsg.msg_controllen = 0;
    sndmsg.msg_flags = MSG_WAITALL;

    sndmsg.msg_iov->iov_base = msg;
    sndmsg.msg_iov->iov_len = len;

    oldfs = get_fs();
    set_fs(KERNEL_DS);
    ret = sock_sendmsg(client_socket, &sndmsg, len);
    if (ret == -1)
        close_client_socket();
    set_fs(oldfs);

    return ret;
}

//-----------------------------------------------------------------------------
int recv_srv_msg(unsigned char* buffer, unsigned short len)
{
    int ret;
    mm_segment_t oldfs;
    struct msghdr sndmsg;
    struct iovec  iov;

    sndmsg.msg_name = 0;
    sndmsg.msg_namelen = 0;
    sndmsg.msg_iov = &iov;
    sndmsg.msg_iovlen = 1;
    sndmsg.msg_control = NULL;
    sndmsg.msg_controllen = 0;
    sndmsg.msg_flags = MSG_WAITALL; //MSG_DONTWAIT;

    sndmsg.msg_iov->iov_base = buffer;
    sndmsg.msg_iov->iov_len = len;

    oldfs = get_fs();
    set_fs(KERNEL_DS);
    ret = sock_recvmsg(client_socket, &sndmsg, len, 0);
    if (ret == -1)
        close_client_socket();
    set_fs(oldfs);

    return ret;
}

//-----------------------------------------------------------------------------
int srv_cmd(int cmd, const unsigned char* data_buf,
            unsigned short data_size, unsigned char** recv_buf,
            unsigned short* recv_size)
{
    unsigned char* send_buf = NULL;
    unsigned short send_size = 0;
    char resp_buf[512];
    unsigned short resp_size = 0;
    int n = -1, i;

    // Validates parameters
    if (!recv_buf || !recv_size) {
        printk(KERN_ERR "Error in srv_cmd() parameters.\n");
        return -1;
    }

    // Allocates memory
    send_size = PROTO_SIZE + data_size;
    send_buf = kmalloc(send_size, GFP_KERNEL);
    if (!send_buf) {
        printk(KERN_ERR "Error allocating buffer to send message to server.\n");
        goto out_err_ret;
    }

    printk("Building command %d.\n", cmd);
    n = proto_build_cmd(cmd, send_buf, send_size, data_buf, data_size);
    if (n != 0)
        goto out_err;

    printk("Sending message to server...\n");
    n = send_srv_msg(send_buf, send_size);
    if (n != send_size) {
        printk(KERN_ERR "Error sending message to server.\n");
        goto out_err;
    }

    printk("Receiving response from server...\n");
    *recv_size = 0;
    while((n = recv_srv_msg(resp_buf, sizeof(resp_buf))) > 0) {
        // Realloc recv_buf
        printk("   received (%d) bytes from server.\n", n);
        *recv_buf = krealloc(*recv_buf, (*recv_size) + n, GFP_KERNEL);
        if ((*recv_buf) == NULL) {
            printk(KERN_ERR "Error allocating memory to recv_buffer.\n");
            kfree(*recv_buf);
            goto out_err;
        }
        memcpy(&(*recv_buf)[*recv_size], resp_buf, n);
        *recv_size += n;
    }

    // Only for debug
    printk("Received message from server with (%d) bytes: [", n);
    for (i = 0; i < n; i++)
        printk("%02X:", resp_buf[i]);
    printk("]\n");

    kfree(send_buf);
    return resp_size;

out_err:
    kfree(send_buf);
out_err_ret:
    return -1;
}

//-----------------------------------------------------------------------------
int srv_read_dir(char* path, struct tree_descr **file_list, unsigned short *nfiles)
{
    unsigned char* recv_buf = NULL;
    unsigned short recv_size = 0;
    unsigned short data_size = 0;
    int i, nreg = 0;

    // Validate parameters
    if (!file_list || !nfiles) {
        printk(KERN_ERR "Error in srv_read_dir() parameters.\n");
        return -1;
    }

    // Send command
    if (srv_cmd(MOUNT_CMD, path, strlen(path), &recv_buf, &recv_size) != 0) {
        printk(KERN_ERR "Could not send or receive the mount command.\n");
        return -1;
    }

    // Process response
    // Data size
    memcpy(&data_size, &recv_buf[3], sizeof(data_size));
    nreg = data_size / sizeof(struct tree_descr);

    printk("Mount command returned [%d] files in remote directory [%s].\n", nreg, path);

    *file_list = kmalloc(nreg * sizeof(struct tree_descr), GFP_KERNEL);
    if ((*file_list) == NULL) {
        printk(KERN_ERR "Error allocating memory in srv_read_dir().\n");
        kfree(recv_buf);
        return -1;
    }
    for (i = 0; i < nreg; i++) {
        memcpy(&(*file_list)[i], &recv_buf[i * sizeof(struct tree_descr)], sizeof(struct tree_descr));
        printk("   Copied tree_descr: name[%s], mode[%03d].\n", (*file_list)[i].name, (*file_list)[i].mode);
    }

    *nfiles = nreg;

    kfree(recv_buf);

    return nreg;
}




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------














