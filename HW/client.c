#include <stdio.h>      // fprintf(), perror()
#include <stdlib.h>     // exit()
#include <string.h>     // memset()
#include <signal.h>    // signal()
#include <unistd.h> 

#include <sys/socket.h> // socket(), connect()
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h>  // htons()
#include <sys/wait.h>

#define SERVER_IP "127.0.0.1"
#define PORT "8080"
#define MAX_BUFFER_SIZE 256

int connfd, fd;

void sigint_handler(int signo) {
    close(fd);
    close(connfd);
}

// preocess signal for zombie
void pr_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void p_child(){
    char buffer[MAX_BUFFER_SIZE];
    while(1){
        if(recv(connfd, buffer, MAX_BUFFER_SIZE, 0)){
            buffer[strlen(buffer)-1] = '\0';
            printf("receive: \n%s\n\n\n", buffer);
        }        
    }
}

void p_parent(){
    char command[256];
    while(1){        
        fgets(command, sizeof(command), stdin);
        if(command[0] == '\n'){
            continue;
        }
        command[strlen(command)-1] = '\0';
        write(connfd, command, sizeof(command));            
        // printf("\n%s\n",command);        
    }
}

int main(int argc, char *argv[]) {
    struct sockaddr_in cli_addr;
    int pid;

    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, pr_handler);

    if ((connfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }
    
    memset(&cli_addr, 0, sizeof(cli_addr));
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    cli_addr.sin_port = htons((u_short)atoi(PORT));
    
    if(connect(connfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) == -1) {
        perror("connect()");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((pid = fork()) < 0) {
            // Fork child
            perror("Fork error");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child
            p_child();
            close(connfd);
            exit(EXIT_SUCCESS);
        } else {
            // Parent
            p_parent();
            close(connfd);
        }
    }

    return 0;
}