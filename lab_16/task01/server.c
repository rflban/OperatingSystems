#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SOCKET_NAME "socket.soc"

int socket_fd;

void sigint_handler(int signum);

int main(void)
{
    int bytes;
    char buf[BUFSIZ];
    struct sockaddr server_name;

    if (signal(SIGINT, &sigint_handler) == SIG_ERR)
    {
        fprintf(stderr, "%s: %s\n", "signal sigint", strerror(errno));
        exit(1);
    }

    if ((socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
        fprintf(stderr, "%s: %s\n", "socket", strerror(errno));
        exit(1);
    }

    server_name.sa_family = AF_UNIX;
    strcpy(server_name.sa_data, SOCKET_NAME);

    if (bind(socket_fd, &server_name, sizeof(server_name.sa_family) +
                                      strlen(server_name.sa_data) + 1) < 0)
    {
        fprintf(stderr, "%s: %s\n", "socket", strerror(errno));
        exit(1);
    }

    for(;;)
    {
        bytes = recvfrom(socket_fd, buf, sizeof(buf), 0, NULL, NULL);

        if (bytes < 0)
        {
            fprintf(stderr, "%s: %s\n", "recvfrom", strerror(errno));
            exit(1);
        }

        buf[bytes] = 0;
        printf("%s", buf);
    }
}

void sigint_handler(int signum)
{
    if (close(socket_fd) != 0)
    {
        fprintf(stderr, "%s: %s\n", "close socket_fd", strerror(errno));
        exit(1);
    }

    if (unlink(SOCKET_NAME) != 0)
    {
        fprintf(stderr, "%s: %s\n", "unlink " SOCKET_NAME, strerror(errno));
        exit(1);
    }

    exit(0);
}

