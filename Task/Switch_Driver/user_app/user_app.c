#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<string.h>

int main(){
    int fd1;
    fd1 = open("/dev/switch_device", O_RDWR);

    if(fd1 < 0){
        printf("Failed to open device file\n");
        return -1;
    }

    int buff;
    while(1){
        if(read(fd1,&buff,sizeof(int)) < 0){
            printf("Failed to read from dev file\n");
            return -1;
        }
        printf("from buffer <%d>\n",buff);
    }
    
    close(fd1);
    return 0;
}
