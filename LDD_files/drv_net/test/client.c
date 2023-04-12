#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_IP_ADDR 	"192.168.0.2"
#define SERVER_PORT 	60000

int main()
{
    int n;
    char wbuffer[100];
    char rbuffer[100];
    int sockfd;
    struct sockaddr_in server_sock_addr;
    struct sockaddr_in client_sock_addr;
    int sock_addr_len;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&server_sock_addr, 0, sizeof (server_sock_addr));
    server_sock_addr.sin_family = AF_INET;
    server_sock_addr.sin_addr.s_addr = inet_addr(SERVER_IP_ADDR);
    server_sock_addr.sin_port = htons(SERVER_PORT);

    while(1)
    {
        printf("Enter message:");
        memset(wbuffer, 0, sizeof(wbuffer));
        gets(wbuffer);
        sendto (sockfd, wbuffer, strlen(wbuffer), 0,
                (struct sockaddr *)&server_sock_addr, sizeof (server_sock_addr));

        memset(rbuffer, 0, sizeof(rbuffer));
        sock_addr_len = sizeof (server_sock_addr);
        n = recvfrom (sockfd, rbuffer, sizeof(rbuffer), 0,
               	      (struct sockaddr *)&server_sock_addr, &sock_addr_len); 
        rbuffer[n]='\0';
        printf("Message read: %s\n", rbuffer);
    }
}
