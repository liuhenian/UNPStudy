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
    int i  ,connfd , listenfd , sockfd , maxfd , nready;
	ssize_t n;
    int client[FD_SETSIZE];
    char buf[1024] = {0};
    fd_set rset , allset;
    socklen_t clilen;
    struct sockaddr_in srvaddr , cliaddr;

    listenfd = socket(AF_INET , SOCK_STREAM , 0);
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    srvaddr.sin_port = htons(5188);
	int on = 1;
	setsockopt(listenfd , SOL_SOCKET , SO_REUSEADDR , &on , sizeof(on));
    if(bind(listenfd , (struct sockaddr*)&srvaddr , sizeof(srvaddr)) < 0)
        ERR_EXIT("bind");
    if(listen(listenfd , 1024) < 0)
        ERR_EXIT("listen");

    maxfd = listenfd;
    for(i = 0 ; i < FD_SETSIZE ; i++)
        client[i] = -1;

    FD_ZERO(&allset);
    FD_SET(listenfd , &allset);
	maxfd = listenfd;
    for(;;)
    {
        rset = allset;
        if((nready = select(maxfd + 1 , &rset , NULL , NULL , NULL)) < 0 )
            ERR_EXIT("select");
        if(FD_ISSET(listenfd , &rset))
        {
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd , (struct sockaddr*)&cliaddr , &clilen);
            for(i = 0 ; i < FD_SETSIZE ; i++)
            {
                if(client[i] < 0)
                {
                    client[i] = connfd;
					FD_SET(connfd , &allset);
                    break;
                }
            }
            if(i == FD_SETSIZE)
				ERR_EXIT("too many clients\n");
            if(client[i] > maxfd)
                maxfd = client[i];
            if(--nready <= 0)
                continue;
        }
    
		for(i = 0 ; i <= maxfd ; i++)
		{
			if((sockfd = client[i]) < 0)
				continue;
			if(FD_ISSET(sockfd , &rset))
			{
				memset(buf , 0 , sizeof(buf));
				if((n = read(sockfd , buf , 1024)) < 0)
					ERR_EXIT("read");
				if(n == 0)
				{
					close(sockfd);
					FD_CLR(sockfd , &allset);
					client[i] = -1;
				}
				else
				{
					fputs(buf , stdout);
					write(sockfd , buf , n);
				}
			}
			if(--nready <= 0)
				break;	
		}
	}
    return 0;
}
