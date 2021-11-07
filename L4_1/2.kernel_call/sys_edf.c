#include "syslib.h"


int sys_edf(int deadline,endpoint_t endpoint)
{
    message m;
    m.m1_i2 = deadline;
    m.m1_i3 = endpoint;
    printf("sys_edf.c %d\n", deadline);
    return (_kernel_call(SYS_SETEDF, &m));
}
