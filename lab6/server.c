#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 256

#define SEM_MODE 0666 /* rw(owner)-rw(group)-rw(other) permission */
#define SEM_KEY 1122334455

int server_socket, client_socket;
int sem, money=0;

void sigint_handler(int signo) {
    close(server_socket);

    // Remove semaphore
    if (semctl(sem, 0, IPC_RMID, 0) < 0) {
        fprintf(stderr, "Unable to remove semaphore %d\n", SEM_KEY);
        exit(1);
    }
    printf("Semaphore %d has been removed\n", SEM_KEY);

    exit(0);
}

/* P () - returns 0 if OK; -1 if there was a problem */
int P (int s){
    struct sembuf sop; /* the operation parameters */
    sop.sem_num = 0; /* access the 1st (and only) sem in the array */
    sop.sem_op = -1; /* wait..*/
    sop.sem_flg = 0; /* no special options needed */

    if (semop (s, &sop, 1) < 0) {
        fprintf(stderr,"P(): semop failed: %s\n",strerror(errno));
        return -1;
    } else {
        return 0;
    }
}

/* V() - returns 0 if OK; -1 if there was a problem */
int V(int s){
    struct sembuf sop; /* the operation parameters */
    sop.sem_num = 0; /* the 1st (and only) sem in the array */
    sop.sem_op = 1; /* signal */
    sop.sem_flg = 0; /* no special options needed */

    if (semop(s, &sop, 1) < 0) {
        fprintf(stderr,"V(): semop failed: %s\n",strerror(errno));
        return -1;
    } else {
        return 0;
    }
}


void bank(){
    char buffer[MAX_BUFFER_SIZE] = {0};
    const char delim[] = " ";
    char* cmd[2];
    int i;

    while (1) {
        recv(client_socket, buffer, MAX_BUFFER_SIZE,0);
  
        cmd[0] = strtok(buffer, delim);
        cmd[1] = strtok(NULL, delim);

        if(strcmp(cmd[0], "deposit") == 0){
            P(sem);
            money += atoi(cmd[1]);
            printf("​​​​​​​​After deposit: %d\n", money);
            V(sem);
        }else if(strcmp(cmd[0], "withdraw") == 0){
            P(sem);
            money -= atoi(cmd[1]);
            printf("​​​​​​​​After withdraw: %d\n", money);
            V(sem);
        }
    }
}


int main(int argc, char **argv){
    struct sockaddr_in server_address, client_address;
    int addrlen = sizeof(server_address);
    int pid;
    int port = atoi(argv[1]);

    signal(SIGINT, sigint_handler);

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    // Bind socket to address and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 30) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d...\n", port);


    /* create semaphore */
    sem = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
    if (sem < 0){
        fprintf(stderr, "Sem %d creation failed: %s\n", SEM_KEY,
        strerror(errno));
        exit(-1);
    }

    /* initial semaphore value to 1 (binary semaphore) */
    if ( semctl(sem, 0, SETVAL, 1) < 0 ){
        fprintf(stderr, "Unable to initialize Sem: %s\n", strerror(errno));
        exit(0);
    }

    printf("Semaphore %d has been created & initialized to 1\n", SEM_KEY);

    while (1) {
        // Accept incoming connection
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        printf("Client connected\n");

        // Handle client requests                

        // Fork a child process to handle client request
        if ((pid = fork()) < 0) {
            perror("Fork error");
            exit(EXIT_FAILURE);
        } 
        else if (pid == 0) { // Child process
            char buffer[256];

            close(server_socket);

            bank();    

            close(client_socket);
            exit(EXIT_SUCCESS);
        } 
        else { // Parent process
            close(client_socket);
        }     
    }

    /* remove semaphore */
    if (semctl (sem, 0, IPC_RMID, 0) < 0){
        fprintf (stderr, "%s: unable to remove sem %d\n", argv[0], SEM_KEY);
        exit(1);
    }
    printf("Semaphore %d has been remove\n", SEM_KEY);

    return 0;
}