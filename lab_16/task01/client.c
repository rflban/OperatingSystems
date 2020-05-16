#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SERVER_SOCKET_NAME "socket.soc"

static inline int get_input(char **buf, size_t *len);

int main(void)
{
    int socket_fd;
    char buf[BUFSIZ];
    char *input;
    size_t input_len;
    struct sockaddr server_name;

    if ((socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
        fprintf(stderr, "%s: %s\n", "signal sigint", strerror(errno));
        exit(1);
    }

    server_name.sa_family = AF_UNIX;
    strcpy(server_name.sa_data, SERVER_SOCKET_NAME);

    input = NULL;
    input_len = 0;

    while (get_input(&input, &input_len))
    {
        input[input_len - 1] = 0;
        snprintf(buf, sizeof(buf), "%d client: %s", getpid(), input);
        sendto(socket_fd, buf, strlen(buf), 0, &server_name,
               sizeof(server_name.sa_family) +
               strlen(server_name.sa_data) + 1);
    }

    if (close(socket_fd) != 0)
    {
        fprintf(stderr, "%s: %s\n", "close socket_fd", strerror(errno));
        exit(1);
    }

    free(input);

    return 0;
}

static inline int get_input(char **buf, size_t *len)
{
    printf("> ");
    getline(buf, len, stdin);

    return !feof(stdin);
}

