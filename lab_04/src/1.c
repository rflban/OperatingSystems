#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    int child_qty = 5;
    pid_t prev_pid = 1;
    pid_t child_pids[child_qty];

    for (int i = 0; prev_pid != 0 && i < child_qty; i++)
    {
        if ((child_pids[i] = prev_pid = fork()) == -1)
        {
            perror("fork");

            exit(1);
        }
    }

    if (prev_pid == 0)
    {
        printf("child: pid = %d, ppid = %d, grid = %d\n",
               getpid(), getppid(), getgid());

        sleep(1);

        printf("child: pid = %d, ppid = %d, grid = %d\n",
               getpid(), getppid(), getgid());
    }
    else
    {
        printf("parent: pid = %d, ppid = %d, grid = %d\n",
               getpid(), getppid(), getgid());

        printf("parent: child pids:\n");
        for (int i = 0; i < child_qty; i++)
        {
            printf("%d\n", child_pids[i]);
        }
    }

    return 0;
}

