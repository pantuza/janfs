#ifndef _JANFS_SOCKET_H_
#define _JANFS_SOCKET_H_


int create_client_socket(void);

int connect_server(char *address, int port);

void close_client_socket(void);

int send_srv_msg(char *msg, uint32_t len);

int recv_srv_msg(char *buffer, uint32_t len);

#endif
