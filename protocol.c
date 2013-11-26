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
int proto_data_field(unsigned char* recv_buf, unsigned short* recv_size)
{
	unsigned char* tmp_buf;

	// Validate parameters
	if (!recv_buf || !recv_size)
		return -1;

	// Validate buffer received
	if (recv_buf[0] != STX || recv_buf[(*recv_size)-1] != ETX)
		return -1;

    tmp_buf = kmalloc(*recv_size, GFP_KERNEL);
    if (!tmp_buf)
		return -1;

	// Data size length
	memcpy(recv_size, &recv_buf[3], sizeof(*recv_size));

	// Use new data size length to copy data to tmp buffer
	memcpy(tmp_buf, &recv_buf[5], *recv_size);

	// Update original buffer with only data bytes
	memcpy(recv_buf, tmp_buf, *recv_size);

	// Free tmp buffer
	kfree(tmp_buf);

	return 0;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


