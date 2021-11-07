*/ located in /usr/include */

#include <lib.h>
#include <unistd.h>

int mycall(void)
{
    message m;
    return (_syscall(PM_PROC_NR,MYCALL,&m));
}