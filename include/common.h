#ifndef __ROOM_COMMON_H
#define __ROOM_COMMON_H
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


#define TRUE 0
#define FALSE -1
#define MSG_LEN 9024

typedef unsigned int uint;

typedef int ElemType;

typedef struct cli_info
{
	char *buf;
	int len;
	int fd;
}cli_info;
//描述双向链表上的一个结点的结构体
typedef struct DuLNode
{
    int cli_fd;//保存新连接的客户端描述符
    struct sockaddr_in cli_addr;
    struct DuLNode *pre;
    struct DuLNode *next;
}DuLNode, *DuLinkList;

DuLNode *list_init();
int tail_insert(DuLinkList head, DuLNode **pTail, int cli_fd,struct sockaddr_in cli_addr);
void print_list_from_head(DuLinkList head);
void print_list_from_tail(DuLNode *tail);
int get_by_value(DuLinkList head, DuLNode **pTail, int cli_fd,DuLNode **node);
int copy_list_from_head(DuLinkList head,char *buf,int *bytecount);
int copy_iplist(char *buf,int *bytecount);

int get_node_by_fd(int fd,DuLNode **node);

#define dbgOut(arg...) \
do{ \
	char b__[1024]; \
	sprintf(b__, arg); \
	fprintf(stdout, "[%s,%s,%d] %s", __FILE__, __func__, __LINE__, b__); \
}while(0)

#endif
