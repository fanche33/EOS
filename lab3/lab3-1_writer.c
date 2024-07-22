#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <fcntl.h>     
#include <unistd.h>    

int main(int argc, char **argv)
{
    char path[] = "/dev/etx_device";
    char buf[1], clear_sign='0';
    int i = 0, fd = 0;
    
    fd = open(path, O_WRONLY);
    if (fd == -1) {
        perror("Error opening GPIO pin");
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