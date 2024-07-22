#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>  // htons()
#include <fcntl.h>     // open()

#define PORT 8080
#define MAX_BUFFER_SIZE 256

#define DESSERT_SHOP     0
#define BEVERAGE_SHOP    1
#define DINER_SHOP       2

#define SEM_KEY0 1122334455
#define SEM_KEY1 1122334466
#define SEM_MODE 0666 /* rw(owner)-rw(group)-rw(other) permission */

int server_socket, client_socket;

int sem_deliver1, sem_deliver2;

const int distance[3] = {3, 5, 8};

const int dessert_manu[2] = {60, 80};
const int beverage_manu[2] = {40, 70};
const int diner_manu[2] = {120, 50};

const char* shop_list = "Dessert shop:3km\n- cookie:$60|cake:$80\nBeverage shop:5km\n- tea:$40|boba:$70\nDiner:8km\n- fried-rice:$120|Egg-drop-soup:$50\n";

typedef struct bill {
    int store;
    int item_1;
    int item_2;
} BILL;

// preocess signal for zombie
void sigpr_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// socket signal
void sigint_handler(int signo) {
    semctl(sem_deliver1, 0, IPC_RMID, 0);
    semctl(sem_deliver2, 0, IPC_RMID, 0);
    close(client_socket);
    close(server_socket);    
}


// semaphore functions
int P (int s)
{
    struct sembuf sop; /* the operation parameters */
    sop.sem_num = 0; /* access the 1st (and only) sem in the array */
    sop.sem_op = -1; /* wait..*/
    sop.sem_flg = 0; /* no special options needed */
    if (semop (s, &sop, 1) < 0) {
        fprintf(stderr,"P(): semop failed: %s\n",strerror(errno));
        return -1;
    } 
    else {
        return 0;
    }
}
/* V() - returns 0 if OK; -1 if there was a problem */
int V(int s)
{
    struct sembuf sop; /* the operation parameters */
    sop.sem_num = 0; /* the 1st (and only) sem in the array */
    sop.sem_op = 1; /* signal */
    sop.sem_flg = 0; /* no special options needed */
    if (semop(s, &sop, 1) < 0) {
        fprintf(stderr,"V(): semop failed: %s\n",strerror(errno));
        return -1;
    } 
    else {
        return 0;
    }
}

void send_shop_list() {
    send(client_socket, shop_list, strlen(shop_list), 0);
}

void send_msg(char msg[]){
    send(client_socket, msg, strlen(shop_list), 0);
    printf(":send %s\n",msg);
}

void order_system(BILL *order, char *item, int num){
    char buf[256];

    if(order->store == -1){
        if(strcmp(item, "cookie") == 0 || strcmp(item, "cake") == 0){
            order->store = DESSERT_SHOP;
        }
        else if(strcmp(item, "tea") == 0 || strcmp(item, "boba") == 0){
            order->store = BEVERAGE_SHOP;
        }
        else if(strcmp(item, "fried-rice") == 0 || strcmp(item, "Egg-drop-soup") == 0){
            order->store = DINER_SHOP;
        }
        else{
            send_msg("Invalid command\n");
        }
    }

    if(strcmp(item, "cookie") == 0 || strcmp(item, "cake") == 0){
        if(order->store == DESSERT_SHOP){
            if(strcmp(item, "cookie") == 0){
                order->item_1 += num;
            }else if(strcmp(item, "cake") == 0){
                order->item_2 += num;
            }else{
                send_msg("Invalid dessert item\n");
            }
        }
    }else if(strcmp(item, "tea") == 0 || strcmp(item, "boba") == 0){
        if(order->store == BEVERAGE_SHOP){
            if(strcmp(item, "tea") == 0){
                order->item_1 += num;
            }else if(strcmp(item, "boba") == 0){
                order->item_2 += num;
            }else{
                send_msg("Invalid beverage item\n");
            }
        }
    }else if(strcmp(item, "fried-rice") == 0 || strcmp(item, "Egg-drop-soup") == 0){
        if(order->store == DINER_SHOP){
            if(strcmp(item, "fried-rice") == 0){
                order->item_1 += num;
            }else if(strcmp(item, "Egg-drop-soup") == 0){
                order->item_2 += num;
            }else{
                send_msg("Invalid diner item\n");
            }
        }
    }

    if(order->store == DESSERT_SHOP){     
        if(order->item_1 != 0 && order->item_2 != 0){
            const char *format = "cookie %d|cake %d\n";
            sprintf(buf, format, order->item_1, order->item_2);
        }
        else if(order->item_1 != 0){
            const char *format = "cookie %d\n";
            sprintf(buf, format, order->item_1);            
        }
        else if(order->item_2 != 0){
            const char *format = "cake %d\n";
            sprintf(buf, format, order->item_2);                
        }                
    } 
    else if(order->store == BEVERAGE_SHOP){
        if(order->item_1 != 0 && order->item_2 != 0){
            const char *format = "tea %d|boba %d\n";
            sprintf(buf, format, order->item_1, order->item_2);
        }
        else if(order->item_1 != 0){
            const char *format = "tea %d\n";
            sprintf(buf, format, order->item_1);            
        }
        else if(order->item_2 != 0){
            const char *format = "boba %d\n";
            sprintf(buf, format, order->item_2);                
        }
    }
    else if(order->store == DINER_SHOP){
        if(order->item_1 != 0 && order->item_2 != 0){
            const char *format = "fried-rice %d|Egg-drop-soup %d\n";
            sprintf(buf, format, order->item_1, order->item_2);
        }
        else if(order->item_1 != 0){
            const char *format = "fried %d\n";
            sprintf(buf, format, order->item_1);            
        }
        else if(order->item_2 != 0){
            const char *format = "Egg-drop-soup %d\n";
            sprintf(buf, format, order->item_2);                
        }          
    }
    send_msg(buf);
}

int deliver(BILL *order){
    char buffer[MAX_BUFFER_SIZE];
    char moeny_str[10];
    char msg[100] = "Delivery has arrived and you need to pay ";
    int money=0;
    int f_deliver1, f_deliver2;
    int t1=0, t2=0, t=0;
    int i;

    switch(order->store){
        case DESSERT_SHOP:
            money = order->item_1 * dessert_manu[0] + order->item_2 * dessert_manu[1];
            break;
        case BEVERAGE_SHOP:
            money = order->item_1 * beverage_manu[0] + order->item_2 * beverage_manu[1];
            break;
        case DINER_SHOP:
            money = order->item_1 * diner_manu[0] + order->item_2 * diner_manu[1];
            break;
    }
    
    if(money == 0){
        send_msg("Please order some meals\n");
        //V(sem_confirm);
        return 0;
    }
    else{
        // read deliver1 time
        P(sem_deliver1);   
        f_deliver1 = open("./deliver1.txt", O_RDWR); 
        read(f_deliver1, buffer, sizeof(buffer));
        t1 = atoi(buffer);
        close(f_deliver1);

        // read deliver2 time
        P(sem_deliver2);
        f_deliver2 = open("./deliver2.txt", O_RDWR);
        read(f_deliver2, buffer, sizeof(buffer));
        t2 = atoi(buffer);
        close(f_deliver2);

        // check if both deliver1 and deliver2 will take more than 30 minutes
        if(t1 + distance[order->store] >= 30 && t2 + distance[order->store] >= 30){ 
            send_msg("Your delivery will take a long time, do you want to wait?\n");
            read(client_socket, buffer, MAX_BUFFER_SIZE);
            //printf("%s\n","Your delivery will take a long time, do you want to wait?\n");        
            printf("%s\n",buffer);
            if(strcmp(buffer, "No") == 0){
                printf("No\n");
                return 0;
            }        
        }

        send_msg("Please wait a few minutes...\n");

        // if deliver1 is faster
        if(t1 < t2){
            f_deliver1 = open("./deliver1.txt", O_RDWR);
            read(f_deliver1, buffer, sizeof(buffer));
            t1 = atoi(buffer);
            t1 += distance[order->store];
            sprintf(buffer, "%d", t1);
            lseek(f_deliver1, 0, SEEK_SET);
            write(f_deliver1, buffer, sizeof(buffer));
            close(f_deliver1);
            V(sem_deliver1);
            V(sem_deliver2);

            for(i=t1; i>0; i--){
                sleep(1);

                if(i <= distance[order->store]){
                    P(sem_deliver1);
                    f_deliver1 = open("./deliver1.txt", O_RDWR);
                    read(f_deliver1, buffer, sizeof(buffer));
                    t1 = atoi(buffer);
                    t1 -= 1;
                    sprintf(buffer, "%d", t1);
                    lseek(f_deliver1, 0, SEEK_SET);
                    write(f_deliver1, buffer, sizeof(buffer));
                    close(f_deliver1);
                    V(sem_deliver1);
                }
            }
        }
        else{     // if deliver2 is faster
            f_deliver2 = open("./deliver2.txt", O_RDWR);
            read(f_deliver2, buffer, sizeof(buffer));
            t2 = atoi(buffer);
            t2 += distance[order->store];
            sprintf(buffer, "%d", t2);
            lseek(f_deliver2, 0, SEEK_SET);
            write(f_deliver2, buffer, sizeof(buffer));
            close(f_deliver2);
            V(sem_deliver1);
            V(sem_deliver2);

            for(i=t2; i>0; i--){
                sleep(1);
                
                if(i <= distance[order->store]){
                    P(sem_deliver2);
                    f_deliver2 = open("./deliver2.txt", O_RDWR);
                    read(f_deliver2, buffer, sizeof(buffer));
                    t2 = atoi(buffer);
                    t2 -= 1;
                    sprintf(buffer, "%d", t2);
                    lseek(f_deliver2, 0, SEEK_SET);
                    write(f_deliver2, buffer, sizeof(buffer));
                    close(f_deliver2);
                    V(sem_deliver2);
                }
            }
        }

        sprintf(moeny_str, "%d$", money);
        strcat(msg, moeny_str);

        strcat(msg, "\n");
        send_msg(msg);
        return 1;
    }    
}

void main_system(){
    const char delim[] = " ";
    BILL *order;
    char* cmd[3];
    char buffer[MAX_BUFFER_SIZE] = {0};
    int sent=0;
    int i;

    order = (struct bill *)malloc(sizeof(struct bill));
    
    if (order == NULL) {
        printf("Memory allocation failed\n");
        return;
    } 

    order->store = -1;
    order->item_1 = 0;
    order->item_2 = 0;

    while (1) {
        // Receive message from client
        read(client_socket, buffer, MAX_BUFFER_SIZE);
        // buffer[strlen(buffer)-1] = '\0';      
        printf("pid: %d %s\n",getpid() ,buffer);

        // Continue getting tokens until no more tokens are available
        cmd[0] = strtok(buffer, delim);

        for(i=1; i<3; i++){
            cmd[i] = strtok(NULL, delim);
            if(cmd[i] == NULL) {
                cmd[i] = "\0";
                continue;
            }
        }
        
        // Process client request
        if (strcmp(cmd[0], "shop") == 0 && strcmp(cmd[1], "list") == 0) {
            send_shop_list(client_socket);
        }else if (strcmp(cmd[0], "order") == 0) {
            order_system(order, cmd[1], atoi(cmd[2]));
        } 
        else if (strcmp(cmd[0], "confirm\n") == 0) {
            sent = deliver(order);
            if(sent){
                close(client_socket);
                free(order);
                break;                            
            }
        }     
        else if (strcmp(cmd[0], "cancel") == 0) {    
            // Close socket
            close(client_socket);
            free(order);
            break;
        }     
        else{
            //illegal command
            send_msg("Invalid command\n");
        }      
    }
}

void sem_init(){
    sem_deliver1 = semget(SEM_KEY0, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
    if (sem_deliver1 < 0)
    {
        fprintf(stderr, "Sem %d creation failed: %s\n", SEM_KEY0,
        strerror(errno));
        exit(-1);
    }
    /* initial semaphore value to 1 (binary semaphore) */
    if ( semctl(sem_deliver1, 0, SETVAL, 1) < 0 )
    {
        fprintf(stderr, "Unable to initialize Sem: %s\n", strerror(errno));
        exit(0);
    }   
    
    sem_deliver2 = semget(SEM_KEY1, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
    if (sem_deliver2 < 0)
    {
        fprintf(stderr, "Sem %d creation failed: %s\n", SEM_KEY1,
        strerror(errno));
        exit(-1);
    }
    /* initial semaphore value to 1 (binary semaphore) */
    if ( semctl(sem_deliver2, 0, SETVAL, 1) < 0 )
    {
        fprintf(stderr, "Unable to initialize Sem: %s\n", strerror(errno));
        exit(0);
    }   
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_address, client_address;
    int addrlen = sizeof(server_address);
    int pid;
    FILE* f = fopen("deliver1.txt", "w");
    fprintf(f, "0");
    fclose(f);
    f = fopen("deliver2.txt", "w");
    fprintf(f, "0");
    fclose(f);

    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigpr_handler);

    sem_init();

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(atoi(argv[1]));

    // Bind socket to address and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept incoming connection
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t*)&addrlen)) < 0) {
            // perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("Client connected\n");

        // Fork a child process to handle client request
        if ((pid = fork()) < 0) {
            perror("Fork error");
            exit(EXIT_FAILURE);
        } 
        else if (pid == 0) { // Child process
            close(server_socket);
            // Handle client requests
            main_system();                       
            close(client_socket);
            printf("over\n");
            exit(EXIT_SUCCESS);
        } 
        else { // Parent process
            close(client_socket);
        } 
    }

    // Close server socket
    close(server_socket);

    return 0;
}