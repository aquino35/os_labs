/* Example of use of fork system call */
#include <stdio.h>
void main()
{
    int pid;
    pid = fork();

    if (pid < 0) //error
    {
        fprintf(stderr, "Fork failed!\n"); exit(-1);
    }
    else if (pid==0) // child
    {
        execlp("/bin/ps", "ps", NULL);
        printf("Still around...\n");
    }
    else // parent
    {
        // display its own pid and the ppid:
        printf( "Child PID: %d\n Parent PID: %d\n\n", pid, (int)getpid() );
        // parent waits for the child process end before continuing
        int wait_pid = waitpid(pid);
        if( wait_pid < 0) printf( "Error while waiting for child!\n" );
        else printf( "Child's PID: %d\n\n", pid ); // display pid

        exit(0); // exit succesfully
    }
}