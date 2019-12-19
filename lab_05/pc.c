#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <string.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

#define SEMFLAGS (SEM_UNDO)
#define FULLPERM (S_IRWXU | S_IRWXG | S_IRWXO)

enum { SB = 0, SE, SF };
enum { P = -1, V = +1 };
enum { N = 3 };

struct sembuf pstart[2] = { {SE, P, SEMFLAGS}, {SB, P, SEMFLAGS} };
struct sembuf cstart[2] = { {SF, P, SEMFLAGS}, {SB, P, SEMFLAGS} };

struct sembuf pstop[2] = { {SF, V, SEMFLAGS}, {SB, V, SEMFLAGS} };
struct sembuf cstop[2] = { {SE, V, SEMFLAGS}, {SB, V, SEMFLAGS} };

void sig_handler(int sugnum);

pid_t child_pids[N * 2];

int main(void)
{
    srand(time(NULL));

    signal(SIGINT, &sig_handler);

    int shmid;
    int semid;

    int child_qty = sizeof(child_pids) / sizeof(child_pids[0]);

    printf("parent: pid = %d\n", getpid());

    for (int idx = 0; idx < child_qty; idx++)
    {
        if ((child_pids[idx] = fork()) == -1)
        {
            perror("fork");

            exit(1);
        }
        else if (child_pids[idx] == 0)
        {
            while (1);
        }
        else
        {
            printf("parent: child's #%d pid = %d\n",
                    idx + 1, child_pids[idx]);
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

    return 0;
}

void sig_handler(int signum)
{
    for (int i = 0; i < N; i++)
    {
        kill(child_pids[i], signum);
        kill(child_pids[i + N], signum);
    }

    exit(signum);
}

