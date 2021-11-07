#include <stdio.h>
#include <mycalllib.h>

int main()
{
    int deadline=99;
    printf("test_kernel_call.c %d\n", deadline); 
    mycall(deadline);
    return 0;
}
