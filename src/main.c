#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <sys/epoll.h>
#include <errno.h>
#include "common.h"

#define dbgOut(arg...) \
do{ \
	char b__[1024]; \
	sprintf(b__, arg); \
	fprintf(stdout, "[%s,%s,%d] %s", __FILE__, __func__, __LINE__, b__); \
}while(0)


//初始化客户端链表
DuLinkList cli_list_head;
DuLNode *cli_list_tail;

int copy_iplist(char *buf,int *bytecount)
{
	return copy_list_from_head(cli_list_head,buf,bytecount);
}

int get_node_by_fd(int fd,DuLNode **node)
{
	return get_by_value(cli_list_head,&cli_list_tail, fd,node);
}

void signal_handler(int signum)
{
	dbgOut("signal: %d\n", signum);
}

//处理客户端接受的数据的线程函数
void *handler_cli_msg(void *arg){
	cli_info *info = (cli_info *)arg;
	int ret;
	char username[32];
	char password[10];
	//线程分离
	pthread_detach(pthread_self());
	//dbgOut("rec :%s\n",info->buf);
	//处理命令
	cmd_get(info->buf,info->len,info->fd);
	free(info->buf);
	free(info);
	pthread_exit(NULL);
}

int make_tcpserver()
{
	int server_fd; //serverFd;
	server_fd = socket(AF_INET, SOCK_STREAM, 0); //初始化socket套接字
	if (-1 == server_fd)
	{
		//perror 根据errno的值打印出出错的原因
		perror("socket() error");
		return -1;
	}
	
	//绑定地址和端口
	struct sockaddr_in in;
	in.sin_family = AF_INET;
	//设置端口号
	in.sin_port = htons(8080);
	//设置IP地址
	in.sin_addr.s_addr = inet_addr("0.0.0.0");
	
	int ret;
	ret = bind(server_fd, (struct sockaddr *)&in, sizeof(struct sockaddr));
	if (-1 == ret)
	{
		perror("bind error");
		return -1;
	}
	
	//监听客户端的连接请求
	listen(server_fd, 30);
	return server_fd;
}

/*
	@brief 保存客户端信息到链表
	@param fd 客户端文件描述符
	@param cli_addr客户端ip地址
	@return 成功true 失败false
*/
int save_client(int fd, struct sockaddr_in cli_addr)
{
	int ret = tail_insert(cli_list_head,&cli_list_tail, fd, cli_addr);
	//todo test
	print_list_from_head(cli_list_head);
	if(TRUE == ret)
		return ret;
	else
		return ret;
}

int remove_client(int fd)
{
	int ret = delete_by_value(cli_list_head,&cli_list_tail, fd);
	//todo test
	print_list_from_head(cli_list_head);
	if(TRUE == ret)
		return ret;
	else
		return ret;
}

int main(int argc, char *argv[])
{
	cli_list_head = list_init();
	cli_list_tail = NULL;
	//注册一个SIGPIPE信号处理函数
	signal(SIGPIPE, signal_handler);

	int server_fd = make_tcpserver();
	int cli_fd; //表示连接到服务端的客户端的文件描述符
	struct sockaddr_in cli_addr;
	socklen_t len;

	int max_fd =  1;//需要监听文件描述符个数

	printf("bind ok\n\r");

	int ep_fd = epoll_create(10);//10 是size
	if(ep_fd<0){
		perror("epoll_create: ");
		return 0;
	}

	struct epoll_event ev;
	ev.data.fd = server_fd;
	ev.events = EPOLLIN;
	//将服务器的socket文件描述符server_fd进行监听
	int ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, server_fd, &ev);
	if(0 == ret){
		//success
	}else if(-1==ret){
		//fail
		perror("epoll fail: ");
		return 0;
	}
	printf("epoll init\n");
	//定义一个数组来保存就绪的事件
	struct epoll_event evs[1024];
	cli_info *info;
	//char *buf;
	while(1)
	{
		ret = epoll_wait(ep_fd, evs, sizeof(evs)/sizeof(struct epoll_event), -1);//阻塞
		if (ret <= 0)
			continue;
			//dbgOut("ret:%d\n",ret);
			for(int i =0;i < ret ;i++)
			{
				//dbgOut("evs[i].data.fd:%d\n",evs[i].data.fd);
				//服务端是否就绪
				if(evs[i].data.fd == server_fd)
				{
					len = sizeof(struct sockaddr);
					cli_fd = accept(server_fd, (struct sockaddr *)&cli_addr, &len);
					dbgOut("new client: %s %d\n",inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port));
					//监听新客户端文件描述符，添加到epoll红黑树中
					struct epoll_event cli_ev;
					cli_ev.events = EPOLLIN |EPOLLET;//启用边沿触发模式
					cli_ev.data.fd = cli_fd;
					ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, cli_fd, &cli_ev);

					//把新客户端信息保存到链表(将新节点插入到链表)
					save_client(cli_fd,cli_addr);
				}
				else 
				{
					if (evs[i].events & EPOLLIN)
					{
						info = (cli_info *)malloc(sizeof(cli_info));
						info->buf = (char *)malloc(MSG_LEN);
						info->fd = evs[i].data.fd;
						memset(info->buf, 0, MSG_LEN);
						//fctl(evs[i].data.fd, F_SETFL, O_NONBLOCK);
						//int n = read(evs[i].data.fd, buf, sizeof(buf)-1);
						int n = 0;
						int m = 0;
						int rclen = 0;
						while (1)
						{
							n = recv(evs[i].data.fd, info->buf + m, MSG_LEN-1, MSG_DONTWAIT);
							//dbgOut("n: %d\n", n);
							if (n < 0 && errno == EAGAIN)
							{
								break;
							}
							else if (n > 0)
							{
								m += n;
								rclen+=n;
								continue;
							}	
							else if (n == 0)
							{
								dbgOut("client : %d closed\n", evs[i].data.fd);
								remove_client(evs[i].data.fd);
								ev.events = EPOLLIN;
								ev.data.fd = evs[i].data.fd;
								//将断开的客户端从监听的红黑树中删除
								epoll_ctl(ep_fd, EPOLL_CTL_DEL, evs[i].data.fd, &ev);
								close(evs[i].data.fd);								
								break;
							}
						}
						//dbgOut("%d recv: %s\n", evs[i].data.fd, buf);
						//free(buf);
						info->len = rclen;
						pthread_t tid;
						pthread_create(&tid,NULL,handler_cli_msg,(void *)info);
					}
						
				}
			}
	}
	
}