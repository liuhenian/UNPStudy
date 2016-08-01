#include<sys/select.h>
#include<sys/times.h>
#include<stdio.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<string.h>

#include <sys/types.h>
#include <sys/socket.h>

#define ERR_EXIT(m) \
    do \
    {   \
        perror(m); \
        exit(EXIT_FAILURE); \
    }while(0)

int main()
{
    int sockfd,connfd;
    struct sockaddr_in svraddr;
    char recvbuf[1024] = {0};
    char sendbuf[1024] = {0};
    memset(&svraddr , 0 , sizeof(svraddr));
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    svraddr.sin_family = AF_INET;
    svraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    svraddr.sin_port = htons(5188);
   if( connect(sockfd , (struct sockaddr *)&svraddr , sizeof(svraddr)) < 0)
	   ERR_EXIT("connect");
		memset(sendbuf , 0 , sizeof(sendbuf));
		memset(recvbuf , 0 , sizeof(sendbuf));
    while(fgets(sendbuf , sizeof(sendbuf), stdin) != NULL)
    {
        write(sockfd , sendbuf , 1024);
        if(read(sockfd , recvbuf , 1024) == 0)
            ERR_EXIT("ERROR");
        fputs(recvbuf , stdout);
		memset(sendbuf , 0 , sizeof(sendbuf));
		memset(recvbuf , 0 , sizeof(sendbuf));
    }


    return 0;
}
