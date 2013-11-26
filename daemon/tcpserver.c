/*
* tcpserver.c - A simple TCP echo server
* usage: tcpserver <port>
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define BUFSIZE 512





//-----------------------------------------------------------------------------
// Luiz Gustavo
//-----------------------------------------------------------------------------
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


struct FileDesc {
	char name[3];
	char type;
	unsigned long size;
};

//-----------------------------------------------------------------------------
// Returns false if file name is equal to "." or ".."
//-----------------------------------------------------------------------------
int scan_filter(const struct dirent *entry)
{
  if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
     return 0;

  return 1;
}

//-----------------------------------------------------------------------------
// Reads directory and returns a list of files and directories
//-----------------------------------------------------------------------------
int read_local_dir(const char * path, struct FileDesc ** const list)
{
 struct dirent **namelist;
 int i, n;
 int nfiles;
 struct stat s;
 char filename[256];

 if (!list || !path || !path[0])
   return -1;

 nfiles = scandir(path, &namelist, scan_filter, alphasort);
 printf("There is(are) [%d] file(s) in [%s].\n", nfiles, path);
 i = 0;
 if(nfiles > 0) {
   *list = (struct FileDesc*) malloc(nfiles * sizeof(struct FileDesc));
   if(!(*list)) {
     printf("Can't alloc memory \n");
     return -1;
   }
   memset((*list), 0, nfiles * sizeof(struct FileDesc));
   for(n = 0; n < nfiles; n++) {
     sprintf(filename,"%s/%s", path, namelist[n]->d_name);
     // File stat
     printf("      Stat file [%s].\n", filename);
     if(stat(filename, &s) == 0) {
       strncpy((*list)[i].name, namelist[n]->d_name, sizeof((*list)[i].name)-1);
       (*list)[i].size = s.st_size;
       if(S_ISDIR(s.st_mode))
         (*list)[i].type = 'D';
       else
         (*list)[i].type = 'F';
       printf("        Added file {%s}{%c}{%lu} to list.\n", (*list)[i].name, (*list)[i].type, (*list)[i].size);

       i++;
     }
     else {
       printf("      Stat file [%s] error (%s).\n", filename, strerror(errno));
       free(namelist);
       return -1;
     }
     free(namelist[n]);
   }
   free(namelist);
 }
 return (i);
}




//-----------------------------------------------------------------------------
// Commands and protocol control bytes
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
// process_buffer - Process received message
//-----------------------------------------------------------------------------
int process_buffer(char* buf, int size, char* recv_buf, unsigned short* recv_size)
{
 unsigned char cmd = 0;
 unsigned char* data;
 unsigned short data_size = 0;
 
 if (!buf || size <= 0 || !recv_buf || !recv_size)
  return -1;

 *recv_size = 0;
 
 // Validates the first and the last byte
 if (buf[0] != STX) {
  printf("Error: first byte != STX!\n");
  return -1;
 }
 
 if (buf[size-1] != ETX) {
  printf("Error: last byte != ETX!\n");
  return -1;
 }
 
 cmd = buf[2];
 switch(cmd) {
 case MOUNT_CMD: {
   printf("   Mount command received.\n");
  // Reads data size with 2 bytes
  memcpy(&data_size, &buf[3], sizeof(data_size));
  printf("      Data size: %d.\n", data_size);
  data = malloc(data_size+1);
  memcpy(data, &buf[5], data_size);
  data[data_size] = 0;
  printf("      Dir to mount: %s.\n", data);
  // Read dir and mounts list of directories and files in root to return
  memcpy(recv_buf, buf, 3);    // STX, sequence number and command
  struct FileDesc* file_list = NULL;
  int nfiles = read_local_dir(data, &file_list);
  *recv_size = 0;
  if (nfiles > 0) {
     *recv_size = nfiles * sizeof(struct FileDesc);
     // Data
     memcpy(&recv_buf[5], file_list, *recv_size);
   }
   // Data size
   memcpy(&recv_buf[3], recv_size, 2);
   // End of text
   *recv_size += 6;
   recv_buf[*recv_size - 1] = ETX;

   if (data)      free(data);
   if (file_list) free(file_list);
 }
 break;
 case READ_CMD:
 
 break;
 case WRITE_CMD:
 
 break;
 case CREATE_CMD:
 
 break;
 case OPEN_CMD:
 
 break;
 case CLOSE_CMD:
 
 break;
 case DELETE_CMD:
 
 break;
 default:
  printf("   Error: unknown command (%d) received!\n", cmd);
  return -1;
 }
 

 return 0;
}




//-----------------------------------------------------------------------------
/*
* error - wrapper for perror
*/
void error(char *msg) {
 perror(msg);
 exit(1);
}

//-----------------------------------------------------------------------------
int main(int argc, char **argv) {
 int parentfd; /* parent socket */
 int childfd; /* child socket */
 int portno; /* port to listen on */
 int clientlen; /* byte size of client's address */
 struct sockaddr_in serveraddr; /* server's addr */
 struct sockaddr_in clientaddr; /* client addr */
 struct hostent *hostp; /* client host info */
 char buf[BUFSIZE]; /* message buffer */
 char *hostaddrp; /* dotted decimal host addr string */
 int optval; /* flag value for setsockopt */
 int n; /* message byte size */
 int i;

 char recv_buf[8192];
 unsigned short recv_size;

 /*
  * check command line arguments
  */
 if (argc != 2) {
   fprintf(stderr, "usage: %s <port>\n", argv[0]);
   exit(1);
 }
 portno = atoi(argv[1]);

 /*
  * socket: create the parent socket
  */
 parentfd = socket(AF_INET, SOCK_STREAM, 0);
 if (parentfd < 0)
   error("ERROR opening socket");

 /* setsockopt: Handy debugging trick that lets
  * us rerun the server immediately after we kill it;
  * otherwise we have to wait about 20 secs.
  * Eliminates "ERROR on binding: Address already in use" error.
  */
 optval = 1;
 setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR,
    (const void *)&optval , sizeof(int));

 /*
  * build the server's Internet address
  */
 bzero((char *) &serveraddr, sizeof(serveraddr));

 /* this is an Internet address */
 serveraddr.sin_family = AF_INET;

 /* let the system figure out our IP address */
 serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

 /* this is the port we will listen on */
 serveraddr.sin_port = htons((unsigned short)portno);

 /*
  * bind: associate the parent socket with a port
  */
 if (bind(parentfd, (struct sockaddr *) &serveraddr,
  sizeof(serveraddr)) < 0)
   error("ERROR on binding");

 /*
  * listen: make this socket ready to accept connection requests
  */
 if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */
   error("ERROR on listen");

 /*
  * main loop: wait for a connection request, echo input line,
  * then close connection.
  */
 clientlen = sizeof(clientaddr);

 while (1) {
 /*
  *  accept: wait for a connection request
  */
 childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
 if (childfd < 0)
   error("ERROR on accept");
 
 /*
  * gethostbyaddr: determine who sent the message
  */
 hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                       sizeof(clientaddr.sin_addr.s_addr), AF_INET);
 if (hostp == NULL)
   error("ERROR on gethostbyaddr");
 hostaddrp = inet_ntoa(clientaddr.sin_addr);
 if (hostaddrp == NULL)
   error("ERROR on inet_ntoa\n");

 printf("--> Server established connection with %s (%s)\n", hostp->h_name, hostaddrp);
   


   /*
    * read: read input string from the client
    */
   bzero(buf, BUFSIZE);
   n = read(childfd, buf, BUFSIZE);
   if (n < 0)
     error("   ERROR reading from socket");

   printf("   Server received %d bytes: [", n);
   for(i = 0; i < n; i++)
      printf("%02x:", buf[i]);
   printf("]\n");
   
   /*
    * Process received message
    */
   recv_size = 0;
   process_buffer(buf, n, recv_buf, &recv_size);
   
   /*
    * write:
    */
   if (recv_size > 0) {
     n = write(childfd, recv_buf, recv_size);
     if (n < 0)
       error("   ERROR writing to socket");

     printf("   Server response (%d): [", recv_size);
     for(i = 0; i < recv_size; i++)
        printf("%02x:", recv_buf[i]);
     printf("]\n");
   }
   else {
     printf("    No response from sever.\n");
     write(childfd, '\0', 1);
   }

   close(childfd);

 }



}


