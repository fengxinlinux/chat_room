/*************************************************************************
	> File Name: server.c
	> Author: FengXin
	> Mail: fengxinlinux@gmail.com
	> Created Time: 2016年08月08日 星期一 14时13分13秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<sys/signal.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include"message.h"

#define PORT   6666      //服务器端口
#define LISTEN_SIZE  1024   //连接请求队列的最大长度
#define EPOLL_SIZE    1024   //监听的客户端的最大数目
#define BUF_MAX     257     //读取缓存区最大字节 

type struct cli_data
{
    int  cli_fd;    //记录连接的用户套接字
    char username[21];   //记录账号，最大20位
    char ip[16];    //记录ip地址
    struct cli_data* next;



}cli_data;

cli_data* create()  //创建一个带头节点的链表，储存已连接的用户
{
    cli_data* head;
    head=(cli_data*)malloc(sizeof(struct cli_data));
    head->next=NULL;
    return head;

}
void insert(cli_data* temp)   //插入链表
{
    cli_data* new;
    new=(cli_data*)malloc(sizeof(cli_data));
    new->next=NULL;
    new->next=temp->next;
    temp->next=new;
    
}
void delete(cli_data* head,int cli_fd)   //链表的删除
{
    cli_data* temp,*last;
    temp=head->next;
    last=head;
    while(temp->cli_fd!=cli_fd)
    {
        last=temp;
        temp=temp->next;
        
    }
    if(temp->cli_fd==cli_fd)
    {
        last->next=temp->next;
        free(temp);
    }
}


void my_err(char* string)
{
    perror("string");
    exit(1);
}

int main()
{
    int sock_fd,conn_fd;  //储存套接字
    struct sockaddr_in cli_addr,serv_addr;
    struct message recv_buf  //读取缓存区
    socklen_t cli_len;
    int epollfd;
    struct epoll_event* event;
    struct cli_data* head=NULL,*temp=NULL;


    //创建一个套接字
    sock_fd=socket(AF_INET,SOCK_STREAM,0);
    if(sock_fd<0)
    {
        my_err("socket");
    }

    //设置该套接字使之可以重新绑定端口
    int optval=1;
    if(setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,(void*)&optval,sizeof(int))<0)
    {
        my_err("setsockopt");
    }

    //初始化服务器端地址结构
    memset(&serv_addr,0,sizeof(struct sockaddr_in));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(PORT);
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);

    if(bind(sock_fd,(struct sockaddr*)serv_addr,sizeof(struct sockaddr_in))<0)
    {
        my_err("bind");
    }

    //将套接字转化为监听套接字
    if(listen(sock_fd,LISTEN_SIZE)<0)
    {
        my_err("listen");
    }
    

    cli_len=sizeof(struct sockaddr_in);
    event=(struct epoll_event*)malloc(sizeof(struct epoll_event)*EPOLL_SIZE);
   
    //创建一个监听描述符epoll,并将监听套接字加入
    epollfd=epoll_create(EPOLL_SIZE);
    if(epollfd==-1)
    {
        my_err("epoll_create");
    }
    if(epoll_ctl(epollfd,EPOLL_CTL_ADD,sock_fd,&event)<0)
    {
        my_err("epoll_ctl");
    }

   
    head=create();  //创建链表
    temp=head;

    while(1)
    {
        int sum=0,i;
        sum=epoll_wait(epollfd,event,EPOLL_SIZE,-1);
        for(i=0;i<sum;i++)
        {
            if(event[i].data.fd==sock_fd)    //有请求连接
            {
                insert(temp);
                temp=temp->next;
                cli_fd=accept(sock_fd,(struct sockaddr*)&cli_addr,&cli_len);
                if(cli_fd<0)
                {
                    my_err("accept");
                }
                temp->cli_fd=cli_fd;
                temp->ip=inet_ntoa(cli_addr);

                if(epoll_ctl(epollfd,EPOLL_CTL_ADD,cli_fd,&event)<0)   //将新连接的套接字加入监听
                {
                    my_err("epoll_ctl");
                }
                printf("a connet is connected,ip is %s\n",inet_ntoa(cli_addr));
                


                
            }
            else if(event[i].events&EPOLLIN)    //有可读的套接字
            {
               memset(&recv_buf,0,sizeof(struct message));  
               if(recv(cli_fd,&recv_buf,sizeof(struct message))<0)
                   {
                       my_err("recv");
                   }




                
            }
            else if(event[i].events&EPOLLRDHUP)
            {
                delete(head,event[i].data.fd);
                close(event[i].data.fd);
                
            }
            else
            continue;


        }



    }

    return 0;





}
