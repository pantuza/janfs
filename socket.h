#ifndef _JANFS_SOCKET_H_
#define _JANFS_SOCKET_H_

//-----------------------------------------------------------------------------
// Creates the client socket.
//-----------------------------------------------------------------------------
int create_client_socket(void);

//-----------------------------------------------------------------------------
// Connects to remote host.
//-----------------------------------------------------------------------------
int connect_server(char *address, int port);

//-----------------------------------------------------------------------------
// Closes the client socket.
//-----------------------------------------------------------------------------
void close_client_socket(void);

//-----------------------------------------------------------------------------
// Sends a message to server.
//-----------------------------------------------------------------------------
int send_srv_msg(unsigned char* msg, unsigned short len);

//-----------------------------------------------------------------------------
// Receives a message from server with at most len bytes.
//-----------------------------------------------------------------------------
int recv_srv_msg(unsigned char* buffer, unsigned short len);

//-----------------------------------------------------------------------------
// Sends a command to server and returns the response.
//-----------------------------------------------------------------------------
int srv_cmd(int cmd, const unsigned char* data_buf,
            unsigned short data_size, unsigned char* recv_buf,
            unsigned short recv_size);

#endif // _JANFS_SOCKET_H_
