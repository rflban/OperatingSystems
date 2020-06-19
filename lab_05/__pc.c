#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SEMFLAGS (SEM_UNDO)
#define FULLPERM (S_IRWXU | S_IRWXG | S_IRWXO)

enum
{
    SB = 0,
    SE = 1,
    SF = 2
};

enum
{
    P = -1,
    V = +1
};

const int N = 3;
const int COUNT = 6;

int* shared_buffer;
int* sh_pos_cons;
int* sh_pos_prod;

struct sembuf pstart[2] = { {SE, P, SEMFLAGS}, {SF, P, SEMFLAGS} };
struct sembuf cstart[2] = { {SB, P, SEMFLAGS}, {SF, P, SEMFLAGS} };

struct sembuf pstop[2] = { {SB, V, SEMFLAGS}, {SF, V, SEMFLAGS} };
struct sembuf cstop[2] = { {SE, V, SEMFLAGS}, {SF, V, SEMFLAGS} };

void producer(const int semid, const int value)
{
    sleep(rand() % 5);
    int sem_op_p = semop(semid, pstart, 2);
    if ( sem_op_p == -1 )
    {
        perror("sem error prod func. #1");
        exit(1);
    }

    shared_buffer[*sh_pos_prod] = *sh_pos_prod;
    printf("producer № %d -> %d\n", value, shared_buffer[*sh_pos_prod]);
    (*sh_pos_prod)++;


    int sem_op_v = semop(semid, pstop, 2);
    if ( sem_op_v == -1 )
    {
        perror("sem error prod func. #2");
        exit(1);
    }
}

void consumer(const int semid, const int value)
{
    sleep(rand() % 2);
    int sem_op_p = semop(semid, cstart, 2);
    if ( sem_op_p == -1 )
    {
        perror( "sem error cons #1 \n" );
        exit(1);
    }

    printf("consumer № %d <- %d\n", value, shared_buffer[*sh_pos_cons]);
    (*sh_pos_cons)++;
    int sem_op_v = semop(semid, cstop, 2);
    if ( sem_op_v == -1 )
    {
        perror( "sem error cons #2 \n" );
        exit(1);
    }

}
int cpid[6];

void callback(int n)
{
    for (int i=0;i<COUNT;i++){
    kill(cpid[i],SIGTERM);
    }
}


int main()
{

signal(SIGINT, callback);
    int shmid, semid;
    int s;
    int parent_pid = getpid();
      printf("parent pid: %d\n", parent_pid);

    if ((shmid = shmget(IPC_PRIVATE, (N + 1) * sizeof(int), IPC_CREAT | FULLPERM)) == -1)
    {
        perror("shared mem error.\n");
        exit(1);
    }

    sh_pos_prod = shmat(shmid, 0, 0);
    if (*sh_pos_prod == -1)
    {
        perror("mem error \n");
        exit(1);
    }


    shared_buffer = sh_pos_prod + 2 * sizeof(int);
    sh_pos_cons = sh_pos_prod + sizeof(int);

    (*sh_pos_prod) = 0;
    (*sh_pos_cons) = 0;


    if ((semid = semget(IPC_PRIVATE, 3, IPC_CREAT | FULLPERM)) == -1)
    {
        perror("sem create error\n");
        exit(1);
    }

    int ctrl_sb = semctl(semid, SB, SETVAL, 0);
    int ctrl_se = semctl(semid, SE, SETVAL, N);
    int ctrl_sf = semctl(semid, SF, SETVAL, 1);

    if ( ctrl_se == -1 || ctrl_sf == -1 || ctrl_sb == -1)
    {
        perror( "set error \n" );
        exit(1);
    }

for (int i = 0; i < COUNT; ++i) {
    cpid[i] = fork();
    if (cpid[i] > 0) {
        printf("child pid: %d\n", cpid[i]);
    }
    else if (cpid[i] == 0) {
        if (i < 3)
        {   while (1){
            producer(semid, i);
            }
            exit(0);
        }
        else
        {   while (1){
            consumer(semid, i);
            }
            exit(0);
        }
    }
    else {
        printf("fork error\n");
        exit(1);
    }
}
wait(&s);
}

