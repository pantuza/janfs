#include <linux/kernel.h> /* printk() */
#include <linux/string.h> /* memcpy() */
#include "protocol.h"

//-----------------------------------------------------------------------------
// Local functions prototypes.
//-----------------------------------------------------------------------------
static int build(unsigned char command,
                 unsigned char* buf,
                 unsigned short* buf_len,
                 const unsigned char* data,
                 unsigned short data_len);

//-----------------------------------------------------------------------------
// Local definitions
//-----------------------------------------------------------------------------
#define MOUNT_CMD  0x01
#define READ_CMD   0x02
#define WRITE_CMD  0x03
#define CREATE_CMD 0x04
#define OPEN_CMD   0x05
#define CLOSE_CMD  0x06
#define DELETE_CMD 0x07

#define STX 0x02
#define ETX 0x03

//-----------------------------------------------------------------------------
// Local variables
//-----------------------------------------------------------------------------
static unsigned char command_count = 0;

//-----------------------------------------------------------------------------
static int build(unsigned char command,
                 unsigned char* buf,
                 unsigned short* buf_len,
                 const unsigned char* data,
                 unsigned short data_len)
{
	int pos = 0;
	
	if (!buf || !buf_len)  // data is not mandatory
		return -1;
		
	buf[pos++] = STX;
	buf[pos++] = command_count++;
	buf[pos++] = command;
	memcpy(&buf[pos], &data_len, sizeof(data_len));
	pos += data_len;
	if (data) {
		memcpy(&buf[pos], data, data_len);
		pos += data_len;
	}
	buf[pos++] = ETX;
	// Set buffer size
	*buf_len = pos;
	
	// Only for debug
	printk("Buffer [");
	for (pos = 0; pos < *buf_len; pos++)
		printk("%02x:", buf[pos]);
	printk("]");
}

//-----------------------------------------------------------------------------
void mount_cmd(unsigned char* buf, int* len)
{
	
}

//-----------------------------------------------------------------------------
void read_cmd(unsigned char* buf, int* len)
{
}

//-----------------------------------------------------------------------------
void write_cmd(unsigned char* buf, int* len)
{
}

//-----------------------------------------------------------------------------
void create_cmd(unsigned char* buf, int* len)
{
}

//-----------------------------------------------------------------------------
void open_cmd(unsigned char* buf, int* len)
{
}

//-----------------------------------------------------------------------------
void close_cmd(unsigned char* buf, int* len)
{
}

//-----------------------------------------------------------------------------
void delete_cmd(unsigned char* buf, int* len)
{
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


