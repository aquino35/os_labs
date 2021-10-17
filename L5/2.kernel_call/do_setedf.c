#include "kernel/system.h"
#include <minix/endpoint.h>

int do_setedf(struct proc *caller, message *m_ptr)
{
    struct proc *p;
    int proc_nr = 0;
    if(!isokendpt(m_ptr->m1_i3, &proc_nr))
        return EINVAL;
    p = proc_addr(proc_nr);
    p->deadline = m_ptr -> m1_i2;
    printf("do_setedf.c. %d\n",m_ptr->m1_i2);
    return(OK);
}