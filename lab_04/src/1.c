#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    pid_t childpid;

    if ((childpid = fork()) == -1)
    {
        perror("can't fork");

        exit(1);
    }
    else if (childpid == 0)
    {
        sleep(1);

        printf("child: pid = %d, ppid = %d, grid = %d\n",
               getpid(), getppid(), getgid());
    }
    else
    {
        printf("parent: pid = %d, ppid = %d, grid = %d\n",
               getpid(), getppid(), getgid());
    }

    return 0;
}

