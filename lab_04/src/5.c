#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

void handle_signal(int sig_num)
{
    printf("\n");
    printf("Catched signal(%d)\n", sig_num);

    exit(sig_num);
}

int main(void)
{
    signal(SIGINT, &handle_signal);

    int child_qty = 5;
    pid_t prev_pid = 1;

    int descr[2];

    if (pipe(descr) == -1)
    {
        perror("pipe");

        exit(1);
    }

    for (int i = 0; prev_pid != 0 && i < child_qty; i++)
    {
        if ((prev_pid = fork()) == -1)
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

        printf("sleep for 3 seconds\n");
        sleep(1);
        printf("sleep for 2 seconds\n");
        sleep(1);
        printf("sleep for 1 seconds\n");
        sleep(1);

        char message[64];
        close(descr[1]);

        printf("parent reads:\n");
        {
            int j;

            for (int i = 0; i < child_qty; i++)
            {
                j = 0;
                do
                {
                    read(descr[0], message + j, 1);
                }
                while (message[j++] != '\0');

                printf("%s\n", message);
            }
        }

        int status;
        pid_t child_pid;

        while ((child_pid = wait(&status)) != -1 && errno != ECHILD)
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

        if (errno != ECHILD)
        {
            perror("wait");

            exit(1);
        }
    }

    return 0;
}

