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
enum { PQTY = 3, CQTY = 3 };

void produce(int semid, int **buf_ptr);
void consume(int semid, int **buf_ptr);

void sig_handler(int sugnum);

void free_shbuf(int shmid, void *buffer);
void free_sem(int semid);

pid_t child_pids[PQTY + CQTY];

int shmid;
int semid;

int *shbuf_begin;
int *shbuf_end;

struct sembuf pstart[2] = {
    { SE, P, SEMFLAGS }, { SB, P, SEMFLAGS }
};
struct sembuf cstart[2] = {
    { SF, P, SEMFLAGS }, { SB, P, SEMFLAGS }
};

struct sembuf pstop[2] = {
    { SF, V, SEMFLAGS }, { SB, V, SEMFLAGS }
};
struct sembuf cstop[2] = {
    { SE, V, SEMFLAGS }, { SB, V, SEMFLAGS }
};

int main(void)
{
    signal(SIGINT, &sig_handler);

    if ((shmid = shmget(IPC_PRIVATE, N * sizeof(int) + 2 * sizeof(int *), FULLPERM)) == -1)
    {
        perror("shmget");

        exit(1);
    }

    shbuf_begin = shmat(shmid, NULL, 0);

    if (shbuf_begin == (void *)-1)
    {
        perror("shmat");

        exit(1);
    }

    shbuf_end = shbuf_begin + N;

    int **pptr = (int **)shbuf_end;
    *pptr = shbuf_end - 1;

    int **cptr = pptr + 1;
    *cptr = shbuf_end - 1;

    if ((semid = semget(IPC_PRIVATE, 3, FULLPERM)) == -1)
    {
        perror("semget");

        exit(1);
    }

    int sctl_b = semctl(semid, SB, SETVAL, 1);
    int sctl_e = semctl(semid, SE, SETVAL, N);
    int sctl_f = semctl(semid, SF, SETVAL, 0);

    if (sctl_b == -1 || sctl_e == -1 || sctl_f == -1)
    {
        perror("semctl");

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

            if (idx < PQTY)
            {
                while (1)
                {
                    sleep(rand() % 3);
                    produce(semid, pptr);
                }
            }
            else
            {
                while (1)
                {
                    sleep(rand() % 3);
                    consume(semid, cptr);
                }
            }
        }
        else
        {
            printf("parent: child's #%d pid = %d - ",
                   idx + 1, child_pids[idx]);

            if (idx < PQTY)
            {
                printf("producer\n");
            }
            else
            {
                printf("consumer\n");
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

    free_shbuf(shmid, shbuf_begin);
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
    for (int i = 0; i < N; i++)
    {
        kill(child_pids[i], signum);
        kill(child_pids[i + N], signum);
    }

    free_shbuf(shmid, shbuf_begin);

    exit(signum);
}

void produce(int semid, int **buf_ptr)
{
    if (semop(semid, pstart, 2) == -1)
    {
        perror("pstart semop");

        exit(1);
    }

    int val = **buf_ptr;
    *buf_ptr += 1;

    if (*buf_ptr >= shbuf_end)
    {
        *buf_ptr = shbuf_begin;
    }

    **buf_ptr = val + 1;
    printf("producer: pid = %d, write %d\n", getpid(), **buf_ptr);

    if (semop(semid, pstop, 2) == -1)
    {
        perror("pstop semop");

        exit(1);
    }
}

void consume(int semid, int **buf_ptr)
{
    if (semop(semid, cstart, 2) == -1)
    {
        perror("cstart semop");

        exit(1);
    }

    *buf_ptr += 1;

    if (*buf_ptr >= shbuf_end)
    {
        *buf_ptr = shbuf_begin;
    }

    printf("consumer: pid = %d, read %d\n", getpid(), **buf_ptr);

    if (semop(semid, cstop, 2) == -1)
    {
        perror("cstop semop");

        exit(1);
    }
}

