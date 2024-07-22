#include <stdio.h>      // fprintf(), perror()
#include <stdlib.h>     // exit()
#include <string.h>     // memset()
#include <signal.h>     // signal()
#include <unistd.h>     // close()
#include <sys/socket.h> // socket(), connect()
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h>  // htons()

#define MAX_BUFFER_SIZE 256

int connfd;

void sigint_handler(int signo) {
    close(connfd);
    exit(0);
}

void send_commands(const char *action, const char *amount, int times) {
    char command[MAX_BUFFER_SIZE];
    for (int i = 0; i < times; i++) {
        snprintf(command, sizeof(command), "%s %s", action, amount);
        write(connfd, command, strlen(command));
        sleep(1); // optional, to space out the commands
    }
}

int main(int argc, char *argv[]) {
    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    const char *action = argv[3];
    const char *amount = argv[4];
    int times = atoi(argv[5]);

    struct sockaddr_in server_addr;

    signal(SIGINT, sigint_handler);

    if ((connfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons((u_short)port);

    if (connect(connfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("connect()");
        close(connfd);
        exit(EXIT_FAILURE);
    }

    send_commands(action, amount, times);

    close(connfd);
    return 0;
}