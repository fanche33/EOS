#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

typedef struct {
    int guess;
    char result[8];
}data;

int shmid, shm_size = sizeof(data);

data *shm;

sig_atomic_t flag = 0;

void guess_handler(int signo, siginfo_t *info, void *context){
    /* show the process ID sent signal */
    flag = 1;
}

void int_handler(int signum) {
    /* Detach the share memory segment */
    shmdt(shm);

    /* Destroy the share memory segment */
    shmctl(shmid, IPC_RMID, NULL);
    exit(0);
}

int main(int argc, char *argv[]){
    char c;
    int ans, guess;
    key_t key;
    int ret; 

    // signal setting
    struct sigaction my_action;
    memset(&my_action, 0, sizeof (struct sigaction));
    my_action.sa_flags = SA_SIGINFO;
    my_action.sa_sigaction = guess_handler;
    sigaction(SIGUSR1, &my_action, NULL);

    // create shared memory
    key = atoi(argv[1]);

    /* Create the segment */
    if ((shmid = shmget(key, shm_size, IPC_CREAT | 0666)) < 0) {
    perror("shmget");
    exit(1);
    }

    /* Now we attach the segment to our data space */
    if ((shm = (data*)shmat(shmid, NULL, 0)) == (data *) -1) {
    perror("shmat");
    exit(1);
    }    

    printf("Game PID: %d\n",getpid());

    ans = atoi(argv[2]);

    while(1){
        if(flag == 1){
            flag = 0;
            guess = shm->guess;
            if(guess > ans){
                strcpy(shm->result, "smaller");
                printf("[Game] Guess %d, smaller\n", guess);
            }
            else if(guess < ans){
                strcpy(shm->result, "bigger");
                printf("[Game] Guess %d, bigger\n", guess);
            }
            else{
                strcpy(shm->result, "bingo");
                printf("[Game] Guess %d, bingo\n", guess);
            }
        }
    }

    /* Detach the share memory segment */
    shmdt(shm);

    /* Destroy the share memory segment */
    ret = shmctl(shmid, IPC_RMID, NULL);

    if (ret < 0){
        fprintf(stderr, "Server remove share memory failed\n");
        exit(1);
    }
    printf("Done.\n");

    return 0;
}