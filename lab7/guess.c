#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

sig_atomic_t flag = 0;

typedef struct {
    int guess;
    char result[8];
}data;

int shmid, shm_size = sizeof(data);
data *shm;

void timer_handler(int signal_number){
    flag = 1;
}

void int_handler(int signum) {
    /* Detach the share memory segment */
    shmdt(shm);

    /* Destroy the share memory segment */
    shmctl(shmid, IPC_RMID, NULL);
}

int main(int argc, char *argv[]) {    
    int shmid, guess = 0, bound = 0, result;
    pid_t pid_game;
    key_t key;
    data *shm;
    struct sigaction sa;
    struct itimerval timer;    
    int ret;

    key = atoi(argv[1]);
    bound = atoi(argv[2]);
    guess = bound/2;
    pid_game = atoi(argv[3]);

    /* Install timer_handler as the signal handler for SIGVTALRM */
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = timer_handler;
    sigaction (SIGVTALRM, &sa, NULL);

    /* Configure the timer to expire after 1 sec */
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;

    /* Reset the timer back to 1 sec after expired */
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0;

    /* Start a virtual timer */
    setitimer (ITIMER_VIRTUAL, &timer, NULL);

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

    // guessing time
    while(1){      
        if(flag == 1){
            flag = 0;   
            if(strcmp(shm->result, "bigger") == 0){
                guess = (bound - guess)/2 + guess;
            }
            else if(strcmp(shm->result, "smaller") == 0){
                bound = guess;
                guess /= 2;
            }
            else if(strcmp(shm->result, "bingo") == 0){
                break;
            }    
            printf("[Game] Guess: %d\n", guess);
            shm->guess = guess;
            kill(pid_game, SIGUSR1);             
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

    return 0;
}