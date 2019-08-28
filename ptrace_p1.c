#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>/* For constants
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
        int status;
        int child_pid = waitpid(-1, &status, __WALL)
        orig_rax = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * ORIG_RAX, NULL);
        printf("The child (%d) made a system call %ld\n", child_pid, orig_rax);
        ptrace(PTRACE_CONT, child, NULL, NULL);
    }
    return 0;
}