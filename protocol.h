#ifndef _JANFS_PROTOCOL_H_
#define _JANFS_PROTOCOL_H_

//-----------------------------------------------------------------------------
// Returns the MOUNT command and the buffer length.
//-----------------------------------------------------------------------------
void mount_cmd(unsigned char* buf, int* len);

//-----------------------------------------------------------------------------
// Returns the READ command and the buffer length.
//-----------------------------------------------------------------------------
void read_cmd(unsigned char* buf, int* len);

//-----------------------------------------------------------------------------
// Returns the WRITE command and the buffer length.
//-----------------------------------------------------------------------------
void write_cmd(unsigned char* buf, int* len);

//-----------------------------------------------------------------------------
// Returns the CREATE command and the buffer length.
//-----------------------------------------------------------------------------
void create_cmd(unsigned char* buf, int* len);

//-----------------------------------------------------------------------------
// Returns the OPEN command and the buffer length.
//-----------------------------------------------------------------------------
void open_cmd(unsigned char* buf, int* len);

//-----------------------------------------------------------------------------
// Returns the CLOSE command and the buffer length.
//-----------------------------------------------------------------------------
void close_cmd(unsigned char* buf, int* len);

//-----------------------------------------------------------------------------
// Returns the DELETE command and the buffer length.
//-----------------------------------------------------------------------------
void delete_cmd(unsigned char* buf, int* len);

#endif // _JANFS_PROTOCOL_H_
