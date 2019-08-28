/*
Notes: 
-  by using PTRACE_PEEKUSER as the first argument, we can examine the contents of the USER area where register contents and other 
information is stored

*/
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <stdio.h>/* For constants
 ORIG_EAX etc */
int main(){
    pid_t child;
    long orig_rax;
    child = fork();
    if(child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("/bin/ls", "ls", NULL);
    }
    else {
        else {
            while(1) {
                wait(&status);
                if(WIFEXITED(status))
                    break;
                orig_rax = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * ORIG_RAX, NULL);
                if(orig_rax == SYS_write) {
                    if(insyscall == 0) {
                    /* Syscall entry */
                    insyscall = 1;
                    params[0] = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * RDI, NULL);
                    params[1] = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * RSI, NULL);
                    params[2] = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * RDX, NULL);
                    printf("Write called with " "%ld, %ld, %ld\n", params[0], params[1], params[2]);
                    }
                    else { /* Syscall exit */
                        rax = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * RAX, NULL);
                        printf("Write returned with %ld\n", rax);
                        insyscall = 0;
                    }
                }
                ptrace(PTRACE_SYSCALL, child, NULL, NULL);
            }
    return 0;
}