#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#define LISTENQTY 256
#define FEEDBACK_MSG "Ok"

static int socket_fd;
static int greatest_fd;
static int greatest_idx;

static void show_usage(char  *pname);
static void handle_sigint(int signum);

static int _connect(int *client_fds, int *client_pids,
                    fd_set *fdset);
static int _serve_clients(int *client_fds, int *client_pids,
                          fd_set *fdset, fd_set *reset);

int main(int argc, char **argv)
{
    int port;
    struct sockaddr_in server_addr;

    signal(SIGINT, &handle_sigint);

    if (argc < 2)
    {
        show_usage(argv[0]);
        exit(1);
    }
    else
    {
        char *endptr;

        port = strtol(argv[1], &endptr, 10);

        if (*argv[1] == 0 || *endptr != 0)
        {
            show_usage(argv[0]);
            exit(1);
        }
    }

    greatest_fd = socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0)
    {
        fprintf(stderr, "%s: %s\n", "socket", strerror(errno));
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(socket_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        fprintf(stderr, "%s: %s\n", "bind", strerror(errno));
        exit(1);
    }

    listen(socket_fd, LISTENQTY);

    int client_fds[FD_SETSIZE];
    int client_pids[FD_SETSIZE];
    fd_set fdset;
    fd_set reset;

    for (int i = 0; i < FD_SETSIZE; ++i)
        client_fds[i] = -1;

    FD_ZERO(&fdset);
    FD_SET(socket_fd, &fdset);

    for (;;)
    {
        reset = fdset;
        select(greatest_fd + 1, &reset, NULL, NULL, NULL);

        if (FD_ISSET(socket_fd, &reset))
            if (_connect(client_fds, client_pids, &fdset) != 0)
                {
                    fprintf(stderr, "%s: %s\n", "_connect", strerror(errno));
                    exit(1);
                }

        if (_serve_clients(client_fds, client_pids, &fdset, &reset) != 0)
        {
            fprintf(stderr, "%s: %s\n", "_serve_clients", strerror(errno));
            exit(1);
        }
    }
}

static int _connect(int *client_fds, int *client_pids, fd_set *fdset)
{
    int idx;
    int accepted_fd;
    char buf[BUFSIZ] = { 0 };

    accepted_fd = accept(socket_fd, NULL, NULL);

    if (accepted_fd < 0)
        return 1;

    for (idx = 0; idx < FD_SETSIZE && client_fds[idx] >= 0; ++idx);
    if (idx == FD_SETSIZE)
    {
        errno = ENOMEM;
        return 1;
    }

    FD_SET(accepted_fd, fdset);
    client_fds[idx] = accepted_fd;

    if (greatest_fd < accepted_fd)
        greatest_fd = accepted_fd;
    if (greatest_idx < idx)
        greatest_idx = idx;

    client_pids += idx;
    idx = read(accepted_fd, buf, sizeof(buf) - 1);
    *client_pids = atoi(buf);

    printf("Client (pid %d) just connected\n", *client_pids);

    return 0;
}

static int _serve_clients(int *client_fds, int *client_pids,
                          fd_set *fdset, fd_set *reset)
{
    int read_cnt;
    char buf[BUFSIZ] = { 0 };

    for (int i = 0; i <= greatest_idx; ++i)
        if (FD_ISSET(client_fds[i], reset))
        {
            read_cnt = read(client_fds[i], buf, sizeof(buf) - 1);

            if (read_cnt > 0)
            {
                write(client_fds[i], FEEDBACK_MSG, strlen(FEEDBACK_MSG));

                buf[read_cnt] = '\0';
                printf("Client (pid %d): %s", client_pids[i], buf);
            }
            else
            {
                FD_CLR(client_fds[i], fdset);
                close(client_fds[i]);
                client_fds[i] = -1;
                printf("Client (pid %d) just disconnected\n", client_pids[i]);
            }
        }

    return 0;
}

static void show_usage(char *pname)
{
    fprintf(stderr, "Usage:\n\t%s <port-number>\n", pname);
}

static void handle_sigint(int signum)
{
    close(socket_fd);

    exit(0);
}

