#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT 4444
#define MAX_CLIENTS 10

int sockfd, newsockfd, clilen;

void process_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void socket_handler(int signum) {
    close(sockfd);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in serv_addr, cli_addr;
    pid_t pid;

    signal(SIGCHLD, process_handler);
    signal(SIGINT, socket_handler);

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Initialize server
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Bind error");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(sockfd, MAX_CLIENTS) < 0) {
        perror("Listen error");
        exit(EXIT_FAILURE);
    }

    printf("Server started. Waiting for incoming connections...\n");

    clilen = sizeof(cli_addr);

    while (1) {
        // Accept connection
        if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen)) < 0) {
            perror("Accept error");
            exit(EXIT_FAILURE);
        }

        // Fork child
        if ((pid = fork()) < 0) {
            perror("Fork error");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // Child
            close(sockfd);
            dup2(newsockfd, STDOUT_FILENO); 
            execlp("sl", "sl", "-l", NULL);
            exit(EXIT_SUCCESS);
        } else { // Parent
            printf("Train ID: %d\n", pid);
            close(newsockfd);
        }
    }

    return 0;
}