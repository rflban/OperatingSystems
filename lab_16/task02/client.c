#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

static void show_usage(char  *pname);
static int get_input(char *buf, size_t bufsize);

int main(int argc, char **argv)
{
    int port;
    int socket_fd;
    struct hostent *host;
    struct sockaddr_in server_addr;

    if (argc < 3)
    {
        show_usage(argv[0]);
        exit(1);
    }
    else
    {
        char *endptr;

        port = strtol(argv[2], &endptr, 10);

        if (*argv[2] == 0 || *endptr != 0)
        {
            show_usage(argv[0]);
            exit(1);
        }

        host = gethostbyname(argv[1]);

        if (!host)
        {
            fprintf(stderr, "%s: %s\n", "gethostbyname", strerror(errno));
            exit(1);
        }
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0)
    {
        fprintf(stderr, "%s: %s\n", "socket", strerror(errno));
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr, host->h_addr_list[0], host->h_length);
    server_addr.sin_port = htons(port);

    if (connect(socket_fd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        fprintf(stderr, "%s: %s\n", "connect", strerror(errno));
        exit(1);
    }

    int read_cnt;
    char buf[BUFSIZ] = { 0 };

    snprintf(buf, sizeof(buf), "%d", getpid());
    write(socket_fd, buf, strlen(buf));

    while (get_input(buf, sizeof(buf)))
    {
        write(socket_fd, buf, strlen(buf));

        read_cnt = read(socket_fd, buf, sizeof(buf) - 1);
        buf[read_cnt] = '\0';
        printf("feedback: %s\n", buf);
    }

    close(socket_fd);
}

static int get_input(char *buf, size_t bufsize)
{
    printf("> ");
    fgets(buf, bufsize, stdin);

    return !feof(stdin);
}

static void show_usage(char *pname)
{
    fprintf(stderr, "Usage:\n\t%s <host-name> <port-number>\n", pname);
}

