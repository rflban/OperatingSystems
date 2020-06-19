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

enum { SB = 0, SAR, SAW, SWW };
enum { P = -1, W = 0, V = +1 };
enum { RQTY = 5, WQTY = 3 };

void _write(int semid, int *num);
void _read(int semid, int *num);

void sig_handler(int sugnum);

void free_shbuf(int shmid, void *buffer);
void free_sem(int semid);

pid_t child_pids[RQTY + WQTY];

int shmid;
int semid;

int *shared_num;

#define array_len(array) (sizeof(array) / sizeof(array[0]))

struct sembuf rstart[] = {
    { SWW, W, SEMFLAGS },
    { SAW, W, SEMFLAGS },
    { SAR, V, SEMFLAGS }
};

struct sembuf wstart[] = {
    { SWW, V, SEMFLAGS },
    { SAR, W, SEMFLAGS },
    {  SB, P, SEMFLAGS },
    { SAW, V, SEMFLAGS },
    { SWW, P, SEMFLAGS }
};

struct sembuf rstop[] = {
    { SAR, P, SEMFLAGS }
};

struct sembuf wstop[] = {
    { SAW, P, SEMFLAGS },
    {  SB, V, SEMFLAGS }
};

int main(void)
{
    signal(SIGINT, &sig_handler);

    if ((shmid = shmget(IPC_PRIVATE, sizeof(int), FULLPERM)) == -1)
    {
        perror("shmget");

        exit(1);
    }

    shared_num = shmat(shmid, NULL, 0);

    if (shared_num == (void *)-1)
    {
        perror("shmat");

        exit(1);
    }

    *shared_num = 0;

    if ((semid = semget(IPC_PRIVATE, 4, FULLPERM)) == -1)
    {
        perror("semget");

        exit(1);
    }

    if (semctl(semid, SB, SETVAL, 1) == -1)
    {
        perror("semctl SETVAL");

        exit(1);
    }

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
            srand(time(NULL) ^ getpid());

            if (idx < WQTY)
            {
                while (1)
                {
                    sleep(rand() % 3);
                    _write(semid, shared_num);
                }
            }
            else
            {
                while (1)
                {
                    sleep(rand() % 3);
                    _read(semid, shared_num);
                }
            }
        }
        else
        {
            printf("parent: child's #%d pid = %d - ",
                   idx + 1, child_pids[idx]);

            if (idx < WQTY)
            {
                printf("writer\n");
            }
            else
            {
                printf("reader\n");
            }
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

    free_shbuf(shmid, shared_num);
    free_sem(semid);

    return 0;
}

void free_shbuf(int shmid, void *buffer)
{
    if (shmctl(shmid, IPC_RMID, 0) == -1)
    {
        perror("shmctl IPC_RMID");

        exit(1);
    }

    if (shmdt(buffer) == -1)
    {
        perror("shmdt");

        exit(1);
    }
}

void free_sem(int semid)
{
    if (semctl(semid, IPC_RMID, 0) == -1)
    {
        perror("shmctl IPC_RMID");

        exit(1);
    }
}

void sig_handler(int signum)
{
    for (int i = 0; i < (RQTY + WQTY); i++)
    {
        kill(child_pids[i], signum);
    }

    free_shbuf(shmid, shared_num);

    exit(signum);
}

void _write(int semid, int *num)
{
    if (semop(semid, wstart, array_len(wstart)) == -1)
    {
        perror("semop writer start");

        exit(1);
    }

    *num += 1;
    printf("writer: pid % d, write %d\n", getpid(), *num);

    if (semop(semid, wstop, array_len(wstop)) == -1)
    {
        perror("semop writer stop");

        exit(1);
    }
}

void _read(int semid, int *num)
{
    if (semop(semid, rstart, array_len(rstart)) == -1)
    {
        perror("semop reader start");

        exit(1);
    }

    printf("reader: pid %d, read %d\n", getpid(), *num);

    if (semop(semid, rstop, array_len(rstop)) == -1)
    {
        perror("semop reader stop");

        exit(1);
    }
}

