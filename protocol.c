#include <linux/kernel.h> /* printk() */
#include <linux/string.h> /* memcpy() */
#include <linux/slab.h>
#include "protocol.h"

//-----------------------------------------------------------------------------
// Local functions prototypes.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Local definitions
//-----------------------------------------------------------------------------
#define STX 0x02
#define ETX 0x03

//-----------------------------------------------------------------------------
// Local variables
//-----------------------------------------------------------------------------
static unsigned char command_count = 0;

//-----------------------------------------------------------------------------
int proto_build_cmd(unsigned char command, unsigned char* send_buf,
                    unsigned short send_size, const unsigned char* data,
                    unsigned short data_size)
{
	int pos = 0;
	unsigned char* buffer = send_buf;
	
	if (!send_buf)  // Data is not mandatory
		return -1; // Error

	// \TODO I think the best option is to allocate the buffer here

	// Fills buffer
	buffer[pos++] = STX;
	buffer[pos++] = command_count++;
	buffer[pos++] = command;
	memcpy(&buffer[pos], &data_size, sizeof(data_size));
	pos += sizeof(data_size);
	if (data) {
		memcpy(&buffer[pos], data, data_size);
		pos += data_size;
	}
	buffer[pos++] = ETX;
	
	// Only for debug
	printk("Build CMD(0x%02x) [", command);
	for (pos = 0; pos < send_size; pos++)
		printk("%02x:", buffer[pos]);
	printk("]\n");
	
	return 0; // Success
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


