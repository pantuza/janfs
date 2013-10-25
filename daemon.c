/**
 * This is the Janfs daemon. It must be running at server side. 
 * It listens clients connections through a socket interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>

#include <sys/types.h>
#include <sys/stat.h>


#define DAEMON_NAME "janfsd"


int main ()
{
    /* Process ID */
    pid_t pid;
    
    /* Session ID */
    pid_t sid;


    /* Creates a fork of the parent process */
    pid = fork();
    if(pid < 0)
    {
        printf("Can't create a fork. pid=%d\n", pid);
        exit(EXIT_FAILURE);
    }
    if(pid > 0)
    {
        printf("fork created with pid %d\n", pid);
        exit(EXIT_SUCCESS);
    }

    /* Change file mode mask */
    umask(0);

    /* Sets log options */
    setlogmask(LOG_UPTO(LOG_INFO));
    openlog(DAEMON_NAME, /*LOG_NOWAIT |*/ LOG_CONS | LOG_PERROR, LOG_USER);


    /* Creates a session for the child process */
    sid = setsid();
    if(sid < 0)
    {
        syslog(LOG_ERR, "Can't create a session. sid=%d", sid);
        exit(EXIT_FAILURE);
    }


    /* Changes the directory to the file system root */
    if((chdir("/")) < 0)
    {   
        syslog(LOG_INFO,"Can't change directory");
        exit(EXIT_FAILURE);
    }

    
    /* Closing standard files. Avoids user interaction */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    
    syslog(LOG_NOTICE, "Daemon started by User %d", getuid());

    /* Daemon task loop */
    int x = 0;
    while (1)
    {
        sleep(10);
        syslog(LOG_INFO, "hello nurse at %d", x++);
    }

    exit(EXIT_SUCCESS);
}
