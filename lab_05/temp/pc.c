#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define SB 0
#define SE 1
#define SF 2
#define P -1
#define V  1
#define N  3
#define PCNT 3
#define CCNT 3
#define PERMITIONS (S_IRWXU | S_IRWXG | S_IRWXO)

#define SHMKEY 100
#define SEMKEY 100

int shm_id;
int sem_id;
int *buffer_beg;
int *buffer_end;

pid_t child_pids[PCNT + CCNT];

void producer(int sem_id, int **buf_ptr);
void consumer(int sem_id, int **buf_ptr);
void rmshm(int shm_id, void *buffer);
void rmsem(int sem_id);
void sig_process(int sugnum);

void startp();
void startc();
void stopp();
void stopc();

struct sembuf pstart[] = { {SE, P, SEM_UNDO}, {SB, P, SEM_UNDO} };
struct sembuf cstart[] = { {SF, P, SEM_UNDO}, {SB, P, SEM_UNDO} };
struct sembuf pstop[] = { {SF, V, SEM_UNDO}, {SB, V, SEM_UNDO} };
struct sembuf cstop[] = { {SE, V, SEM_UNDO}, {SB, V, SEM_UNDO} };

#define LEN(array) (sizeof(array) / sizeof(array[0]))

int main(void)
{
    signal(SIGINT, &sig_process);

    if ((shm_id = shmget(SHMKEY, N * sizeof(int) + 2 * sizeof(int *), IPC_CREAT | PERMITIONS)) == -1)
    {
        perror("shmget");

        exit(1);
    }

    buffer_beg = shmat(shm_id, 0, 0);

    if (buffer_beg == (void *)-1)
    {
        perror("shmat");

        exit(1);
    }

    buffer_end = buffer_beg + N;

    int **pptr = (int **)buffer_end;
    *pptr = buffer_end - 1;

    int **cptr = pptr + 1;
    *cptr = buffer_end - 1;

    if ((sem_id = semget(SEMKEY, 3, IPC_CREAT | PERMITIONS)) == -1)
    {
        perror("semget");

        exit(1);
    }

    int sctl_b = semctl(sem_id, SB, SETVAL, 1);
    int sctl_e = semctl(sem_id, SE, SETVAL, N);
    int sctl_f = semctl(sem_id, SF, SETVAL, 0);

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
            srand(time(0) ^ getpid());

            if (idx < PCNT)
            {
                while (1)
                {
                    sleep(rand() % 3);
                    producer(sem_id, pptr);
                }
            }
            else
            {
                while (1)
                {
                    sleep(rand() % 3);
                    consumer(sem_id, cptr);
                }
            }
        }
        else
        {
            printf("parent: child's #%d pid = %d - ",
                   idx + 1, child_pids[idx]);

            if (idx < PCNT)
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

    rmshm(shm_id, buffer_beg);
    rmsem(sem_id);

    return 0;
}

void producer(int sem_id, int **buf_ptr)
{
    startp();

    int val = **buf_ptr;
    *buf_ptr += 1;

    if (*buf_ptr >= buffer_end)
        *buf_ptr = buffer_beg;

    **buf_ptr = val + 1;
    printf("producer: pid = %d, write %d\n", getpid(), **buf_ptr);

    stopp();
}

void consumer(int sem_id, int **buf_ptr)
{
    startc();

    *buf_ptr += 1;

    if (*buf_ptr >= buffer_end)
        *buf_ptr = buffer_beg;

    printf("consumer: pid = %d, read %d\n", getpid(), **buf_ptr);

    stopc();
}

void startp()
{
    if (semop(sem_id, pstart, LEN(pstart)) == -1)
    {
        perror("pstart semop");

        exit(1);
    }
}

void startc()
{
    if (semop(sem_id, cstart, LEN(cstart)) == -1)
    {
        perror("cstart semop");

        exit(1);
    }
}

void stopp()
{
    if (semop(sem_id, pstop, LEN(pstop)) == -1)
    {
        perror("pstop semop");

        exit(1);
    }
}

void stopc()
{
    if (semop(sem_id, cstop, LEN(cstop)) == -1)
    {
        perror("cstop semop");

        exit(1);
    }
}


void rmshm(int shm_id, void *buffer)
{
    if (shmctl(shm_id, IPC_RMID, 0) == -1)
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

void rmsem(int sem_id)
{
    if (semctl(sem_id, IPC_RMID, 0) == -1)
    {
        perror("shmctl IPC_RMID");

        exit(1);
    }
}

void sig_process(int signum)
{
    for (int i = 0; i < N; i++)
    {
        kill(child_pids[i], signum);
        kill(child_pids[i + N], signum);
    }

    rmshm(shm_id, buffer_beg);

    exit(signum);
}

