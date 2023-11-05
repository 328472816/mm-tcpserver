#include "modbus.h"


#define HOST_ID  3

//寄存器
//00 主机fd
int reg_list[REG_SIZE] = {0};

//错误 
// 01 00 cmd crc
/*cmd
00 无响应
01 fd不存在
*/

// fd  cmd [...] crc
// 读主机寄存器
// 03  01  寄存器地址00  crc   //rc  03 01  寄存器值  crc

//设置当前fd为主机 fd与当前客户端需要相同
// 03 02  fd  crc   //rc  03 02 fd crc

//获取我的fd和ip
// 03 03  crc      //rc  03 03  16len  fd+ip  crc

//获取所有fd和ip  //fd需要为主机fd
// 03 04 fd crc   //rc  03 04 16len 8lenfd+ip 8lenfd+ip ... crc

//获取fd当前目录文件
// fd 05 crc   //rc  fd 05 16len 8len+type+name ...  crc

//获取fd上级目录
// fd 06 crc   //rc  fd 06 16len 8len+type+name ...  crc

//获取fd下级目录
// fd 07 8len name crc   //rc  fd 07 16len 8len+type+name ...  crc

//下载fd文件
// fd 08 8len name crc  //rc fd 08 crc

//上传fd文件
// fd 09 8len name crc  //rc fd 09 crc

//执行fd文件
// fd 0a 8len name crc  //rc fd 0a crc

unsigned char sum_crc(char *buf,int len)
{
    unsigned char sum = 0;
    for(int i =0; i < len ;i++)
    {
        sum+=buf[i];
    }
    return sum;
}

int crc_check(char *buf,int len)
{
    unsigned char sum = sum_crc(buf,len-1);
    if(sum == (unsigned char)buf[len - 1])
        return 1;
    printf("crc len %d sum %d not %d\n",len,sum,buf[len - 1]);
    return 0;
}


int cmdErrsend(int fd,int err)
{
//错误 
// 01 00 cmd crc
/*cmd
00 无响应
01 fd不存在
*/
    char cmd[4] = {0};
    int index = 0;
    cmd[index] = 0x01; index++;
    cmd[index] = 0x00; index++;
    cmd[index] = err; index++;
    cmd[index] = sum_crc(cmd,index);index++;
    write(fd, cmd, index);
    return 1;
}

int cmd02send(int fd)
{
    //rc  03 02 fd crc
    char cmd[4] = {0};
    int index = 0;
    cmd[index] = 0x03; index++;
    cmd[index] = 0x02; index++;
    cmd[index] = fd; index++;
    cmd[index] = sum_crc(cmd,index);index++;
    write(fd, cmd, index);
    return 1;
}

int cmd02(char* cmd, int len,int fd)
{
    // 03 02  fd  crc   //rc  03 02 fd crc
    if(cmd[0]!=0x03|| cmd[1] != 0x02)
        return 0;
    printf("02 cmd get fd %d\n",fd);
    if(fd == cmd[2])
    {
        reg_list[0] = fd;
        //返回rc
        cmd02send(fd);
    }
    else
    {
        printf("02 cmd fd %d  not sets %d\n",fd,cmd[2]);
    }
    return 1;
}

int cmd03send(int fd,const DuLNode *node)
{
    //rc  03 03  16len  fd+ip  crc
    char ip[50] = {0};
    int len = 0,sendlen = 0;
    sprintf(ip,"%s",inet_ntoa(node->cli_addr.sin_addr));
    len = 1+strlen(ip);
    sendlen = len+5;
    char *cmd =(char*)malloc(sizeof(char)*(sendlen));
    int index = 0;
    cmd[index] = 0x03; index++;
    cmd[index] = 0x03; index++;
    cmd[index] = len&0xff; index++;
    cmd[index] = (len>>8)&0xff; index++;
    cmd[index] = fd; index++;
    memcpy(cmd+index,ip,strlen(ip));
    cmd[sendlen-1] = sum_crc(cmd,sendlen-1);
    write(fd, cmd, sendlen);
    free(cmd);
    return 1;
}

int cmd03(char* cmd, int len,int fd)
{
    // 03 03  crc
    if(cmd[0]!=0x03|| cmd[1] != 0x03)
        return 0;
    printf("03 cmd get fd %d\n",fd);
    //获取当前fd的ip地址
    DuLNode *node;
    int res = get_node_by_fd(fd,&node);
    if(TRUE == res)
    {
        printf("cli_fd:%d ", node->cli_fd);
        printf("cli_addr:%s ", inet_ntoa(node->cli_addr.sin_addr));
        printf("cli_port:%d ", ntohs(node->cli_addr.sin_port));
        printf("\n");
        cmd03send(fd,node);
    }
    else
    {
        printf("03 cmd fd %d  not node\n",fd);
    }
    return 1;
}

int cmd04send(int fd,char *ipbuf, int len16,int  bytecount)
{
    int sendlen = 0;
    //rc  03 04 16len 8lenfd+ip 8lenfd+ip ... crc
    sendlen = 2+2+bytecount+1;
    char *cmd =(char*)malloc(sizeof(char)*(sendlen));
    int index = 0;
    cmd[index] = 0x03; index++;
    cmd[index] = 0x04; index++;
    cmd[index] = len16&0xff; index++;
    cmd[index] = (len16>>8)&0xff; index++;
    memcpy(cmd+index,ipbuf,bytecount);
    cmd[sendlen-1] = sum_crc(cmd,sendlen-1);
    write(fd, cmd, sendlen);
    free(cmd);
    return 1;
}

int cmd04(char* cmd, int len,int fd)
{
    char ipbuf[300] = {0};
    int  bytecount;
    //rc  03 04 16len 8lenfd+ip 8lenfd+ip ... crc
    if(cmd[0]!=0x03|| cmd[1] != 0x04 || fd != reg_list[0])
        return 0;
    printf("04 cmd get fd %d\n",fd);
    //获取当前fd的ip地址
    DuLNode *node;
    int len16 = copy_iplist(ipbuf,&bytecount);
    if(len16 > 0)
    {
        cmd04send(fd,ipbuf,len16,bytecount);
    }
    else
    {
        printf("04 cmd not list\n");
    }
    return 1;
}

int cmd05(char* cmd, int len,int fd)
{
    // fd 05 crc   //rc  fd 05 16len 8len+type+name ...  crc 转发
    DuLNode *node;
    if(fd == reg_list[0])
    {
        //转发目标
        printf("cmd get fd %d send obj %d len %d\n",fd,cmd[0],len);  
        int res = get_node_by_fd(cmd[0],&node);
        if(TRUE == res)
        {
            write(cmd[0], cmd, len);
         }else
         {
            printf("not pc %d\n",cmd[0]);
            cmdErrsend(fd,1);
         }
    }
    else
    {
        //转发主机
        printf("cmd get fd %d send host %d\n",fd,reg_list[0]);
        int res = get_node_by_fd(reg_list[0],&node);
        if(TRUE == res)
        {
            write(reg_list[0], cmd, len);
         }else
         {
            printf("not pc %d\n",reg_list[0]);
            cmdErrsend(fd,1);
         }
    }
    return 1;
}

int cmd_get(char *buf,int len,int fd)
{
    
    if(0 == crc_check(buf,len))
    {
        printf("crc err\n");
        return 0;
    }
    
    switch(buf[1])
    {
        case 2:
            cmd02(buf,len,fd);
            break;
        case 3:
            cmd03(buf,len,fd);
            break;
        case 4:
            cmd04(buf,len,fd);
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
            cmd05(buf,len,fd);
            break;
    }

}


