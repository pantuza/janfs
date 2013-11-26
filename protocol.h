#ifndef _JANFS_PROTOCOL_H_
#define _JANFS_PROTOCOL_H_

//-----------------------------------------------------------------------------
// Commands
//-----------------------------------------------------------------------------
#define MOUNT_CMD  0x01
#define READ_CMD   0x02
#define WRITE_CMD  0x03
#define CREATE_CMD 0x04
#define OPEN_CMD   0x05
#define CLOSE_CMD  0x06
#define DELETE_CMD 0x07

//-----------------------------------------------------------------------------
// Other definitions
//-----------------------------------------------------------------------------
// Total size in bytes of protocol control fields
#define PROTO_SIZE 6

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------
struct FileDesc {
	char name[3];
	char type;
	unsigned long size;
};

//-----------------------------------------------------------------------------
// Builds the requested command and returns the buffer and buffer size builded.
//-----------------------------------------------------------------------------
int proto_build_cmd(unsigned char command, unsigned char* send_buf,
                    unsigned short send_size, const unsigned char* data,
                    unsigned short data_size);

#endif // _JANFS_PROTOCOL_H_
