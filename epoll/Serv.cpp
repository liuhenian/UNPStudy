#include<sys/socket.h>
#include<unistd.h>
#include<algorithm>
#include<sys/times.h>
#include<fcntl.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<errno.h>
#include<sys/epoll.h>
#include<vector>
#define ERR_EXIT(m) \
	do \
	{\
		perror(m); \
		exit(EXIT_FAILURE);\
	}while(0)

void active_nonblock(int fd)
{
	int ret ;
	int flags = fcntl(fd , F_GETFL);
	if(flags == -1)
		ERR_EXIT("fcntl");
	flags |= O_NONBLOCK;
	ret = fcntl(fd , F_GETFL , flags);
	if(ret == -1)
		ERR_EXIT("fcntl");
}




typedef std::vector<struct epoll_event>EventList;

int main()
{
    int count = 0;
	int i , sockfd , listenfd , epollfd ;
	struct sockaddr_in servaddr , cliaddr;
	listenfd = socket(AF_INET ,SOCK_STREAM , 0 );
	memset(&servaddr , 0 , sizeof(servaddr));
	memset(&cliaddr , 0 , sizeof(cliaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(5188);
	int n = 1;
	setsockopt(listenfd , SOL_SOCKET , SO_REUSEADDR , &n , sizeof(n));
	if(bind(listenfd , (struct sockaddr*)&servaddr , sizeof(servaddr)) < 0)
		ERR_EXIT("bind\n");
	if(listen(listenfd , 1024) < 0)
		ERR_EXIT("listen\n");
	epollfd = epoll_create1(EPOLL_CLOEXEC);
	struct epoll_event event;
	event.data.fd = listenfd;
	event.events = EPOLLIN | EPOLLET;

	std::vector<int>clients;
	epoll_ctl(epollfd , EPOLL_CTL_ADD , listenfd , &event);

	EventList events(16);
	struct sockaddr_in peeraddr;
	socklen_t peerlen;
	int connfd ;
	int nready;

	while(1)
	{
		nready = epoll_wait(epollfd , &*events.begin() , static_cast<int>(events.size()) , -1);
		if(nready == -1)
		{
			if(errno == EINTR)
				continue;
			ERR_EXIT("epoll_wait");
		}
		if(nready == 0)
			continue;
		if((size_t)nready == events.size())
			events.resize(events.size()*2);

		for(i = 0 ; i < nready ; i++)
		{
			if(events[i].data.fd == listenfd)
			{
				peerlen = sizeof(peeraddr);
				connfd = accept(listenfd, (struct sockaddr*)&peeraddr , &peerlen);
				if(connfd == -1)
					ERR_EXIT("connfd");
				printf("count = %d\n" , ++count);

				active_nonblock(connfd);

				event.data.fd = connfd;
				event.events = EPOLLIN | EPOLLET;
				epoll_ctl(epollfd , EPOLL_CTL_ADD , connfd , &event);
			}
			else if(events[i].events & EPOLLIN)
			{
				connfd = events[i].data.fd;
				if(connfd < 0)
					continue;

				char recvbuf[1024] = {0};
				int ret = read(connfd , recvbuf , 1024);
				if(ret == -1)
					ERR_EXIT("read");
				if(ret == 0)
				{
					printf("client close\n");
					close(connfd);

					event = events[i];
					epoll_ctl(epollfd , EPOLL_CTL_DEL , connfd , &event);

					clients.erase(std::remove(clients.begin() , clients.end() , connfd) , clients.end());
				}
				fputs(recvbuf , stdout);
				write(connfd , recvbuf , strlen(recvbuf));
			}

		}
	}
	return 0;
}







































