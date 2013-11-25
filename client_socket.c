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

//-----------------------------------------------------------------------------
// Local variables
//-----------------------------------------------------------------------------
static struct socket *client_socket;





//-----------------------------------------------------------------------------
uint32_t to_address(char *address)
{
	unsigned int p1, p2, p3, p4;

	if (sscanf(address,"%u.%u.%u.%u",&p1,&p2,&p3,&p4) != 4) {
		printk(KERN_ERR "Error reading address %s.", address);
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
		printk(KERN_ERR "Error creating client_socket.");
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
		printk(KERN_ERR "Error connecting to server.");
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
            unsigned short data_size, unsigned char* recv_buf,
            unsigned short recv_size)
{
	unsigned char* send_buf = NULL;
	unsigned short send_size = 0;
	char resp_buf[recv_size];
	unsigned short resp_size = 0;
	int n = -1, i;

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
	n = recv_srv_msg(resp_buf, recv_size);
	if (n < 0) {
		printk("Error receiving message from server.\n");
	}
	
	// Only for debug
	printk("Received message from server with (%d) bytes: [", n);
	for (i = 0; i < n; i++)
		printk("%02X:", resp_buf[i]);
	printk("]\n");
	
	// Removes the protocol bytes from response
	memcpy(&resp_size, &resp_buf[3], sizeof(resp_size));
	memcpy(recv_buf, &resp_buf[5], resp_size);
	
	kfree(send_buf);
	return resp_size;
	
out_err:
	kfree(send_buf);
out_err_ret:
	return -1;
}







//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------














