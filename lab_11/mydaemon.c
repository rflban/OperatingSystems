#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

void daemonize();

int main(int argc, const char **argv)
{
    daemonize(argv[0]);
}

void
daemonize(const char *cmd)
{
    umask(0);

    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    {
        fprintf(stderr, "%s: getrlimit, %s\n", cmd, strerror(errno));
        exit(1);
    }

    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "%s: fork, %s\n", cmd, strerror(errno));
        exit(1);
    }
    else if (pid != 0)
    {
        exit(0);
    }
    setsid();

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        fprintf(stderr, "%s: can not ignore SIGHUP, %s\n",
                cmd, strerror(errno));
        exit(1);
    }

    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "%s: fork, %s\n", cmd, strerror(errno));
        exit(1);
    }
    else if (pid != 0)
    {
        exit(0);
    }

    if (chdir("/") < 0)
    {
        fprintf(stderr, "%s: can not cd to \"/\", %s\n",
                cmd, strerror(errno));
        exit(1);
    }

    if (rl.rlim_max == RLIM_INFINITY)
    {
        rl.rlim_max = 1024;
    }
    for (int i = 0; i < rl.rlim_max; ++i)
    {
        close(i);
    }

    int fd0 = open("/dev/null", O_RDWR);
    int fd1 = dup(0);
    int fd2 = dup(0);

    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "wrong file descriptors %d %d %d", fd0, fd1, fd2);
        exit(1);
    }

    while (1)
    {
        sleep(30);
    }
}

