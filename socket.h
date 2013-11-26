#ifndef _JANFS_SOCKET_H_
#define _JANFS_SOCKET_H_

#include "protocol.h"

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
// Reads remote directory and returns the list of it's files.
// The list is formatted according to protocol structure FileDesc.
//-----------------------------------------------------------------------------
int srv_read_dir(char* path, struct FileDesc **file_list,
                 unsigned short *nfiles);

#endif // _JANFS_SOCKET_H_
