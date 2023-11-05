#include "common.h"


//从头结点开始拷贝 返回值为16len
//rc  03 04 16len 8lenfd+ip 8lenfd+ip ... crc
int copy_list_from_head(DuLinkList head,char *buf,int *bytecount)
{
    char ip[50] = {0};
    int len16 = 0;
    *bytecount = 0;
    unsigned char len8 = 0;
    if (NULL == head)
        return -1;
    char *pbuf = buf;
    DuLNode *t = head->next;
    printf("copy list \n");
    while (t != NULL)
    {
        memset(ip,0,50);
        sprintf(ip,"%s",inet_ntoa(t->cli_addr.sin_addr));
        len8=1+strlen(ip);
        len16++;
        *bytecount += (len8+1);
        *pbuf = len8;
        pbuf++;
        *pbuf = t->cli_fd;
        pbuf++;
        memcpy(pbuf,ip,strlen(ip));
        pbuf+=strlen(ip);
        //printf("cli_fd:%d ", t->cli_fd);
        //printf("cli_addr:%s ", inet_ntoa(t->cli_addr.sin_addr));
        //printf("cli_port:%d ", ntohs(t->cli_addr.sin_port));
        t = t->next;
    }
    //printf("\n");
    return len16;
}

//从头结点开始打印
void print_list_from_head(DuLinkList head)
{
    if (NULL == head)
        return;

    DuLNode *t = head->next;
    printf("print list ");
    while (t != NULL)
    {
        printf("cli_fd:%d ", t->cli_fd);
        printf("cli_addr:%s ", inet_ntoa(t->cli_addr.sin_addr));
        printf("cli_port:%d ", ntohs(t->cli_addr.sin_port));
        t = t->next;
    }
    printf("\n");
}

//从尾结点开始打印
void print_list_from_tail(DuLNode *tail)
{
    if (NULL == tail)
        return ;

    DuLNode *t = tail;
    //一直往前遍历直到遍历到了头结点
    while (t->pre != NULL)
    {
         printf("cli_fd:%d ", t->cli_fd);
        printf("cli_addr:%s ", inet_ntoa(t->cli_addr.sin_addr));
        t = t->pre;
    }
    printf("\n");
}

DuLNode *list_init()
{
    //创建一个头结点
    DuLinkList head = (DuLinkList)malloc(sizeof(DuLNode));
    head->next = NULL;
    head->pre = NULL;

    //将链表的头指针返回
    return head;
}

/*
 * @brief 双向链表头插法
 * @param head 双向链表头指针
 * @param pTail 双向链表尾指针地址
 * @param data 需要插入的元素的值
 * @return 成功返回TRUE 失败返回FALSE
 * */
int head_insert(DuLinkList head, DuLNode **pTail, int cli_fd)
{
    if (NULL == head)
    {
        printf("[%s %d] head point is NULL\n", __FUNCTION__ , __LINE__);
        return FALSE;
    }

    //创建一个新的结点
    DuLNode *p = (DuLNode *)malloc(sizeof(DuLNode));
    p->pre = NULL;
    p->next = NULL;
    p->cli_fd = cli_fd;

    //当链表为空时，第一个被插入的结点就是整个链表的尾结点
    if (NULL == head->next)
    {
        *pTail = p;
        p->pre = head;
        head->next = p;
        return TRUE;
    }

    //将新的结点的next指向原来链表上第一个数据结点
    p->next = head->next;
    //将原来链表上的第一个数据结点的前驱结点指针指向新的结点
    head->next->pre = p;
    //头结点的next指向新的结点
    head->next = p;
    //将新的结点的前驱结点指针指向头结点
    p->pre = head;

    return TRUE;
}

/*
 * @brief 双向链表尾插法
 * @param head 双向链表头指针
 * @param pTail 双向链表尾指针地址
 * @param data 需要插入的元素的值
 * @return 成功返回TRUE 失败返回FALSE
 * */
int tail_insert(DuLinkList head, DuLNode **pTail, int cli_fd,struct sockaddr_in cli_addr)
{
    if (NULL == head)
    {
        printf("[%s %d] head point is NULL\n", __FUNCTION__ , __LINE__);
        return FALSE;
    }

    //创建一个新的结点
    DuLNode *p = (DuLNode *)malloc(sizeof(DuLNode));
    p->pre = NULL;
    p->next = NULL;
    p->cli_fd = cli_fd;
    p->cli_addr = cli_addr;

    //判断链表是否为空
    if (NULL == head->next)
    {
        head->next = p;
        p->pre = head;
        *pTail = p;
        return TRUE;
    }

    (*pTail)->next = p;
    p->pre = *pTail;
    *pTail = p;

    return TRUE;
}

/*
 * @brief 在双向链表指定位置插入一个结点
 * @param head 双向链表头指针
 * @param index 需要插入的位置
 * @param data 需要插入的元素的值
 * @return 成功返回TRUE 失败返回FALSE
 * */
int insert_by_index(DuLinkList head, uint index, int cli_fd)
{
    if (NULL == head)
    {
        printf("[%s %d] head point is NULL\n", __FUNCTION__ , __LINE__);
        return FALSE;
    }

    //创建一个新的结点
    DuLNode *p = (DuLNode *)malloc(sizeof(DuLNode));
    p->pre = NULL;
    p->next = NULL;
    p->cli_fd = cli_fd;

    int i = 1;
    DuLNode *t = head->next;
    while (i < index && t != NULL)
    {
        i++;
        t = t->next;
    }

    if (NULL == t)
    {
        printf("[%s %d] index out of range ...\n", __FUNCTION__ , __LINE__);
        return FALSE;
    }

    //新的结点的next指向t
    p->next = t;
    //结点t的前驱结点的next指向新的结点
    t->pre->next = p;
    //新的结点的前驱结点指针pre指向t的前驱结点
    p->pre = t->pre;
    //t的前驱结点指针指向新的结点
    t->pre = p;

    return TRUE;
}

int ascending_insert(DuLinkList head, DuLNode **pTail, int cli_fd)
{
    if (NULL == head)
    {
        printf("[%s %d] head point is NULL\n", __FUNCTION__ , __LINE__);
        return FALSE;
    }

    //创建一个新的结点
    DuLNode *p = (DuLNode *)malloc(sizeof(DuLNode));
    p->pre = NULL;
    p->next = NULL;
    p->cli_fd = cli_fd;

    DuLNode *t = head->next;
    //判断是否为空
    if (NULL == head->next)
    {
        head->next = p;
        p->pre = head;
        *pTail = p;
        return TRUE;
    }

    while (t != NULL)
    {
        if (t->cli_fd <= cli_fd)
        {
            //继续往后遍历
            t = t->next;
            continue;
        }

        p->next = t;
        p->pre = t->pre;
        t->pre->next = p;
        t->pre = p;
        return TRUE;
    }
    //如果链表遍历完毕，说明需要插入的结点比链表上的所有结点都要大应该插入到链表的末尾
    (*pTail)->next = p;
    p->pre = *pTail;
    *pTail = p;

    /*
     * 假如没有尾指针，遍历链表的时候，当遍历到链表的最后一个结点的时候就要停止遍历
     * */
    /*
    while (t->next != NULL)
    {
        if (t->data <= data)
        {
            //继续往后遍历
            t = t->next;
            continue;
        }

        p->next = t;
        p->pre = t->pre;
        t->pre->next = p;
        t->pre = p;
        return TRUE;
    }

    if (t->data <= data){
        p->next = t;
        p->pre = t->pre;
        t->pre->next = p;
        t->pre = p;
    }
    else
    {
        t->next = p;
        p->pre = t;
    }
     */
    return TRUE;
}

/*
 * @brief 删除指定位置的元素
 * @param head 双向链表头指针
 * @param pTail 双向链表尾指针地址
 * @param index 需要删除的元素的位置
 * @return 成功返回TRUE 失败返回FALSE
 * */
int delete_by_index(DuLinkList head, DuLNode **pTail, uint index)
{
    if (NULL == head)
    {
        printf("[%s %d] head point is NULL\n", __FUNCTION__ , __LINE__);
        return FALSE;
    }

    if (0 == index)
    {
        printf("[%s %d] index must > 0\n", __FUNCTION__ , __LINE__);
        return FALSE;
    }

    int i = 1;
    DuLNode *t = head->next;
    while (i < index && t != NULL)
    {
        i++;
        t = t->next;
    }

    //如果整个链表都遍历完了依然没有找到对应的位置，说明这个结点位置超出了链表的长度
    if (NULL == t)
    {
        printf("[%s %d] index out of range ..\n", __FUNCTION__ , __LINE__);
        return FALSE;
    }
    //如果要删除的结点恰好是尾结点
    if (t->next == NULL)
    {
        t->pre->next = NULL;
        *pTail = t->pre;
        free(t);
        return TRUE;
    }
    //如果是中间结点
    //t的前驱结点的next 指向t的后继结点
    t->pre->next = t->next;
    //t的后继结点的前驱结点指针指向t的前驱结点
    t->next->pre = t->pre;
    free(t);

    return TRUE;
}

int getby_index(DuLinkList head, DuLNode **pTail, uint index,DuLNode **node)
{
    if (NULL == head)
    {
        printf("[%s %d] head point is NULL\n", __FUNCTION__ , __LINE__);
        return FALSE;
    }

    if (0 == index)
    {
        printf("[%s %d] index must > 0\n", __FUNCTION__ , __LINE__);
        return FALSE;
    }

    int i = 1;
    DuLNode *t = head->next;
    while (i < index && t != NULL)
    {
        i++;
        t = t->next;
    }

    //如果整个链表都遍历完了依然没有找到对应的位置，说明这个结点位置超出了链表的长度
    if (NULL == t)
    {
        printf("[%s %d] index out of range ..\n", __FUNCTION__ , __LINE__);
        return FALSE;
    }

    *node = t;
    return TRUE;
}

int get_by_value(DuLinkList head, DuLNode **pTail, int cli_fd,DuLNode **node)
{
    if (NULL == head || NULL == head->next)
    {
        printf("[%s %d] head point is NULL\n", __FUNCTION__ , __LINE__);
        return FALSE;
    }

    DuLNode *t = head->next;
    while (t != NULL)
    {
        if (t->cli_fd != cli_fd)
        {
            t= t->next;
            continue;
        }
        //判断需要删除的结点是否恰好是尾结点
        if (t->next == NULL)
        {
            *node = t;
            return TRUE;
        }
        *node = t;
        return TRUE;
    }

    return FALSE;
}

/*
 * @brief 删除指定位置的元素
 * @param head 双向链表头指针
 * @param pTail 双向链表尾指针地址
 * @param data 需要删除的元素的值
 * @return 成功返回TRUE 失败返回FALSE
 * */
int delete_by_value(DuLinkList head, DuLNode **pTail, int cli_fd)
{
    if (NULL == head || NULL == head->next)
    {
        printf("[%s %d] head point is NULL\n", __FUNCTION__ , __LINE__);
        return FALSE;
    }

    DuLNode *t = head->next;
    while (t != NULL)
    {
        if (t->cli_fd != cli_fd)
        {
            t= t->next;
            continue;
        }
        //判断需要删除的结点是否恰好是尾结点
        if (t->next == NULL)
        {
            *pTail = t->pre;
            t->pre->next = NULL;
            free(t);
            break;
        }

        DuLNode *bak = t->next;
        //如果需要删除的结点时中间结点
        t->pre->next = t->next;
        t->next->pre = t->pre;
        free(t);
        t = bak;
    }

    return TRUE;
}

int list_destroy(DuLinkList *head, DuLNode **pTail)
{
    if (NULL == head || NULL == *head || NULL == pTail || NULL == (*head)->next)
        return FALSE;

    DuLNode *t = (*head)->next;
    while (t != NULL)
    {
        (*head)->next = t->next;
        free(t);
        t = (*head)->next;
    }

    free(*head);
    *head = NULL;
    *pTail = NULL;

    return TRUE;
}

typedef struct infoNode
{
    DuLNode *head; //指向头结点的指针
    DuLNode *tail; //指向尾结点的指针
    uint len; //链表的长度
    char *desc; //存储链表描述信息
}infoNode, *PinfoNode;


infoNode *list_init1()
{
    infoNode *L;
    L = (infoNode *)malloc(sizeof(infoNode));

    //创建一个头结点
    L->head = (DuLNode *)malloc(sizeof(DuLNode));
    L->head->next = NULL;
    L->tail = NULL;
    L->len = 0;
    L->desc = (char *)malloc(32);
    strcpy(L->desc, "test list");

    return L;
}

#if 0
int main() {
    DuLinkList head = list_init();
    DuLNode *tail = NULL;

    int i;
    for (i = 0; i < 10; i++)
        head_insert(head, &tail, 100+i*2);

    infoNode *L = list_init1();

    head_insert(L->head, &L->tail, 100);

    print_list_from_head(head);
    print_list_from_tail(tail);

    for (i = 0; i < 10; i++)
        tail_insert(head, &tail, 100+i*2);

    print_list_from_head(head);

    insert_by_index(head, 2, 1000);
    print_list_from_head(head);

    insert_by_index(head, 1, 2000);
    print_list_from_head(head);

    insert_by_index(head, 0, 1000);
    print_list_from_head(head);


    DuLinkList head1 = list_init();
    DuLNode *tail1 = NULL;

    for (i = 0; i < 10; i++)
        //tail_insert(head1, &tail1, 100+i*2);
        ascending_insert(head1, &tail1, 100+i*2);
    print_list_from_head(head1);

    ascending_insert(head1, &tail1, 50);
    print_list_from_head(head1);

    ascending_insert(head1, &tail1, 200);
    print_list_from_head(head1);

    ascending_insert(head1, &tail1, 107);
    print_list_from_head(head1);

    delete_by_index(head1, &tail1, 1);
    print_list_from_head(head1);

    delete_by_index(head1, &tail1, 12);
    print_list_from_head(head1);

    delete_by_index(head1, &tail1, 2);
    print_list_from_head(head1);

    delete_by_index(head1, &tail1,0);
    print_list_from_head(head1);

    for (i = 9; i >= 0; i--)
        tail_insert(head1, &tail1, 100+i*2);

    print_list_from_head(head1);

    delete_by_value(head1, &tail1, 100);
    print_list_from_head(head1);

    delete_by_value(head1, &tail1, 118);
    print_list_from_head(head1);

    delete_by_value(head1, &tail1, 116);
    print_list_from_head(head1);

    delete_by_value(head1, &tail1, 108);
    print_list_from_head(head1);

    return 0;
}
#endif
