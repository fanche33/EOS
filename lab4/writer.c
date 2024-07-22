#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <fcntl.h>     
#include <unistd.h>    

int main(int argc, char **argv)
{
    char path[] = "/dev/mydev";
    char buf[1], clear_sign='0';
    int i = 0, fd = 0;
    
    if ((fd = open(path, O_RDWR)) < 0) {
        perror(path);
        exit(EXIT_FAILURE);
    }

    while(argv[1][i] != '\0'){
        strncpy(buf, &argv[1][i], 1);
        if (write(fd, buf, sizeof(buf)) == -1) {
            perror("write, set pin output");
            exit(EXIT_FAILURE);
        }        
        sleep(1);
        i++;
    }

    write(fd, &clear_sign, 1);

    return 0;
}