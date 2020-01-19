#include <unistd.h>
#include <sys/wait.h>

#include <errno.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

const char *pics[] = { "../other/Dog1.jpg", "../other/Dog2.jpg",
                       "../other/Dog3.jpg", "../other/Dog4.jpg", "../other/Klyopy.jpg" };

int main(void)
{
    int child_qty = 5;
    pid_t prev_pid = 1;

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
        srand(time(0) ^ getpid());

        printf("child: pid = %d, ppid = %d, grid = %d\n",
               getpid(), getppid(), getgid());

        if (execlp("/usr/bin/display", "/usr/bin/display", pics[rand() % 5], 0) == -1)
        {
            perror("exec");

            exit(1);
        }
    }
    else
    {
        printf("parent: pid = %d, ppid = %d, grid = %d\n",
               getpid(), getppid(), getgid());

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

