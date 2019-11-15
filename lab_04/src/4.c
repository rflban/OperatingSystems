#include <unistd.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void)
{
    int child_qty = 5;
    pid_t prev_pid = 1;
    pid_t child_pids[child_qty];

    int descr[2];

    if (pipe(descr) == -1)
    {
        perror("pipe");

        exit(1);
    }

    for (int i = 0; i < child_qty; i++)
    {
        if (prev_pid != 0)
            if ((prev_pid = child_pids[i] = fork()) == -1)
            {
                perror("fork");

                exit(1);
            }
    }

    if (prev_pid == 0)
    {
        char message[64];
        sprintf(message, "Hello from child %d to parent %d!",
                getpid(), getppid());

        close(descr[0]);
        write(descr[1], message, strlen(message) + 1);
    }
    else
    {
        printf("parent: pid = %d, ppid = %d, grid = %d\n",
               getpid(), getppid(), getgid());

        char message[64];
        close(descr[1]);

        printf("parent reads:\n");
        for (int i = 0; i < child_qty; i++)
        {
            for
            (
                int j = 0;
                read(descr[0], message + j, 1), message[j++] != '\0';
            );

            printf("%s\n", message);
        }

        int status;
        pid_t child_pid;

        while ((child_pid = wait(&status)) != -1)
        {
            if (WIFEXITED(status))
            {
                printf("parent: child %d terminated normally with %d rc\n",
                       child_pid, WEXITSTATUS(status));
            }
            else
            {
                printf("parent: child %d terminated abnormally with %d rc\n",
                       child_pid, WEXITSTATUS(status));
            }
        }
    }

    return 0;
}

