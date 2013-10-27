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


/* Creates a fork of the parent process */
void do_fork(pid_t *pid)
{
    *pid = fork();

    if (*pid < 0) {
        printf("Can't create a fork. pid=%d\n", *pid);
        exit(EXIT_FAILURE);
    }

    if (*pid > 0) {
        printf("fork created with pid %d\n", *pid);
        exit(EXIT_SUCCESS);
    }
}



/* Configures syslog */
void start_log()
{
    setlogmask(LOG_UPTO(LOG_INFO));
    openlog(DAEMON_NAME, LOG_NOWAIT | LOG_CONS | LOG_PERROR, LOG_USER);
}


/* Creates a session for the child process */
void do_session(pid_t *sid)
{
    *sid = setsid();

    if(*sid < 0) {
        syslog(LOG_ERR, "Can't create a session. sid=%d", *sid);
        exit(EXIT_FAILURE);
    }
}


/* Changes the directory to the file system root */
void change_dir()
{
    if((chdir("/")) < 0) {   
        syslog(LOG_INFO, "Can't change directory");
        exit(EXIT_FAILURE);
    }
}


/* Closing standard files. Avoids user interaction */
void close_std_files()
{
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}


int main ()
{
    /* Process ID */
    pid_t pid;
    
    /* Session ID */
    pid_t sid;


    do_fork(&pid);
    umask(0);  // Change file mode mask
    start_log();
    do_session(&sid);
    change_dir();
    close_std_files();
    

    syslog(LOG_NOTICE, "Daemon started by User %d", getuid());


    /* Daemon task - TCP socket listener */
    tcp_listener();



    // TODO: Insert here call to listener

    while (1) {
        syslog(LOG_INFO, "hello nurse at %d", x++);
        sleep(5);
    }

    exit(EXIT_SUCCESS);
}
