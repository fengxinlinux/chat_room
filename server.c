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
#include<fcntl.h>
#include<time.h>
#include"message.h"

#define PORT   6666      //服务器端口
#define LISTEN_SIZE  1024   //连接请求队列的最大长度
#define EPOLL_SIZE    1024   //监听的客户端的最大数目
#define BUF_MAX     257     //读取缓存区最大字节 

time_t timep;     //记录当前时间


typedef struct cli_data   //记录在线用户信息的结构体
{
    int  cli_fd;    //记录连接的用户套接字
    char username[21];   //记录账号，最大20位
    //char ip[16];    //记录ip地址
    struct cli_data* next;
}cli_data;
typedef struct user      //记录用户账号信息的结构体
{
    char username[21];
    char pwd[21];
}user;
char* my_time() //输出当前时间
{
    time(&timep);
    char* p=ctime(&timep);
    return p;
}

cli_data* head;     //创建头结点
cli_data* create()  //创建一个带头节点的链表，储存已连接的用户
{
    cli_data* head;
    head=(cli_data*)malloc(sizeof(struct cli_data));
    head->next=NULL;
    return head;
}
cli_data*  insert()   //插入链表
{

    cli_data* temp=head;
    cli_data* new;
    new=(cli_data*)malloc(sizeof(cli_data));
    new->next=NULL;
    
    while(temp->next!=NULL)
    {
        temp=temp->next;
    }  
    temp->next=new;

    return new;

}
void delete(cli_data* head,int cli_fd)   //链表的删除
{
    cli_data* temp,*last;
    temp=head->next;
    last=head;
    while(temp!=NULL&&temp->cli_fd!=cli_fd)
    {
        last=temp;
        temp=temp->next;
    }
    if(temp!=NULL&&temp->cli_fd==cli_fd)
    {
        last->next=temp->next;
        free(temp);
    }
}
void my_err(char* string)
{
    perror(string);
    exit(1);
}
void my_path(char*dir,char* filename,char* path)  //将目录和文件名连接成一个路径
{
    strcpy(path,dir);
    strcat(path,"/");
    strcat(path,filename);
}
void send_message(struct message recv_buf,int conn_fd)   //向客户端发送信息
{
    struct message send_buf;
    send_buf=recv_buf;
    int n=send_buf.n;
    int fd;
    struct user user_data;
    int ret=0;  //用来表示注册账号是否存在或账号密码信息是否匹配
    FILE* fp;
    cli_data* temp; //在线用户链表临时指针
    char path[256];   //储存文件路径
    char friendname[21];  //储存用户好友用户名
    int i=0;
    fp=fopen("passwd.txt","a+");
    fclose(fp);


    switch(n)
    {
        case 0:  {  //注册账号
        if((fd=open("passwd.txt",O_RDWR|O_APPEND))<0)
        {
            my_err("open");
        }
        while(read(fd,&user_data,sizeof(struct user))>0)
        {
            if(strcmp(user_data.username,send_buf.username)==0)
            {
                send_buf.n=-2;
                ret=1;
                break;
            }
        }
        if(!ret)
        {
            strcpy(user_data.username,recv_buf.username);
            strcpy(user_data.pwd,recv_buf.pwd1);
            send_buf.n=22;
            if(write(fd,&user_data,sizeof(struct user))<0)
            {
                my_err("write");
            }
            close(fd);
            
        
            printf("a user register,username:%s,time:%s",user_data.username,my_time());
            
            mkdir(user_data.username,07777);  //创建用户所属目录
            my_path(user_data.username,user_data.username,path);
            fd=open(path,O_RDWR|O_CREAT|O_APPEND,0777);
            if(fd<0)
            {
                printf("创建用户好友文件失败\n");
            }
            close(fd);
        

            
            
        }
        if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
        {
            printf("向客户端发送数据失败\n");
        }
        break;
    }

        case 1:  {//登陆 
        if((fd=open("passwd.txt",O_RDONLY))<0)
        {
            my_err("open");
        }
        while(read(fd,&user_data,sizeof(struct user))>0)
        {
            if(strcmp(user_data.username,send_buf.username)==0)
            {
                if(strcmp(user_data.pwd,send_buf.pwd1)==0)
                {
                    ret=1;
                    break;
                }
            }
        }
        if(ret)
        send_buf.n=11;
        else
        send_buf.n=-1;
        if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
        {
            printf("向客户端发送数据失败\n");
            return;
        }
        if(send_buf.n==11)
        {
          printf("a user is login,username:%s,time:%s",send_buf.username,my_time()); 
          temp=insert();
          temp->cli_fd=conn_fd;
          strcpy(temp->username,send_buf.username);

          my_path("lixian",send_buf.username,path);  //发送离线消息
          fd=open(path,O_RDONLY);
          if(fd>0)
          {
              send_buf.n=6;
              while(read(fd,send_buf.time,sizeof(send_buf.time))!=0)
              {
                  read(fd,send_buf.chat,sizeof(send_buf.chat));
                  if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
                  {
                      printf("向客服端发送离线消息失败\n");
                      return;
                  }
              }
              close(fd);
              fd=open(path,O_RDWR|O_CREAT|O_TRUNC);
              close(fd);
          }
        }
        
        break;
    }


        case 3:   { //添加好友
        temp=head->next;

        my_path(send_buf.from,send_buf.from,path);           //防止重复添加好友
        fd=open(path,O_RDONLY);
        if(fd<0)
        {
            printf("打开用户好友文件失败\n");
            return;
        }
        while(read(fd,friendname,sizeof(friendname))!=0)
        {
            if(strcmp(friendname,send_buf.to)==0)
            {
                send_buf.n=-3;
                if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
                {
                    printf("向客户端发送数据失败\n");
                    return;
                }
                close(fd);
                return;
            }
        }
        while(temp!=NULL)
        {
            
            if(strcmp(temp->username,send_buf.to)==0)  //找到指定在线好友
            { 
               ret=1;
                break;
            }
            temp=temp->next;
        }
        if(ret)
        {
            
            
            int to_fd=temp->cli_fd;        //向另一用户征求是否同意添加好友


            send(to_fd,&send_buf,sizeof(struct message),0);
            recv(to_fd,&send_buf,sizeof(struct message),0);
        

            if(send_buf.n==33)
            {
                my_path(send_buf.from,send_buf.from,path);//写入用户好友文件
                fd=open(path,O_RDWR|O_APPEND);
                if(write(fd,&send_buf.to,sizeof(send_buf.to))<0)
                {
                  printf("写入用户好友失败\n");
                  send_buf.n=-3;
                }
                close(fd);
               my_path(send_buf.from,send_buf.to,path);   //创建用户与该好友的聊天记录文件
               fd=open(path,O_RDWR|O_CREAT|O_TRUNC,0777);
               close(fd);
            
            
                my_path(send_buf.to,send_buf.to,path);    //写入另一用户好友文件
                fd=open(path,O_RDWR|O_APPEND);
                write(fd,&send_buf.from,sizeof(send_buf.from));
                close(fd);
                my_path(send_buf.to,send_buf.from,path);  //创建另一用户与该好友的聊天记录文件
                fd=open(path,O_RDWR|O_CREAT|O_TRUNC,0777);
                close(fd);

            }
        }

        else
        send_buf.n=-3;
        if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
        {
            printf("send error\n");
        }
        break;
    }

        case 4:   {//查看好友列表 
        my_path(recv_buf.username,recv_buf.username,path);  //连接用户好友文件路径
        fd=open(path,O_RDONLY);
        while(read(fd,send_buf.friendname[i++],sizeof(recv_buf.username))!=0);
        send_buf.friendname[i][0]='\0';

        if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
        {
            printf("send error\n");
            
        }
        break;}
        
        case 44:    { //查看在线好友
        my_path(recv_buf.username,recv_buf.username,path);  //连接用户好友文件路径
        fd=open(path,O_RDONLY);
        
        
        while(read(fd,friendname,sizeof(friendname))!=0)
        {
            int n=0;
            temp=head->next;
            while(temp!=NULL)
            {
                if(strcmp(temp->username,friendname)==0)
                {
                    n=1;
                    break;
                }
                else
                temp=temp->next;
            }
            if(n)
            {
                strcpy(send_buf.friendname[i++],friendname);
            }
            
        }
        send_buf.friendname[i][0]='\0';
        if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
        {
           printf("send error\n");                                                    
        }
        break;
       }

        case 5:    //删除好友
       {
           int i=0;
           int ret=0;
           my_path(send_buf.from,send_buf.from,path);
           fd=open(path,O_RDONLY);
           if(fd<0)
           {
               printf("打开用户好友文件失败\n");
               send_buf.n=-5;
           }
           while(read(fd,send_buf.friendname[i++],sizeof(send_buf.username))!=0)
           send_buf.friendname[i][0]='\0';
           i=0;
           close(fd);
           while(strlen(send_buf.friendname[i])!=0)
           {
               if(strcmp(send_buf.friendname[i],send_buf.to)==0)
               {
                   strcpy(send_buf.friendname[i],"\n");
                   ret++;
               }
               i++;
           }
           fd=open(path,O_RDWR|O_CREAT|O_TRUNC);
           if(fd<0)
           {
               printf("打开用户好友文件失败\n");
               send_buf.n=-5;
               
           }
           i=0;
           while(strlen(send_buf.friendname[i])!=0)
           {
               if(strcmp(send_buf.friendname[i],"\n")!=0)
               {
                   if(write(fd,send_buf.friendname[i],sizeof(send_buf.username))<0)
                   {
                       printf("写入用户好友文件失败\n");
                       send_buf.n=-5;
                   }
               }
               i++;
               
           }
           close(fd);


           i=0;
           my_path(send_buf.to,send_buf.to,path);
           fd=open(path,O_RDONLY);      //读取删除对象的好友列表
           if(fd<0)
           {
               printf("打开删除对象好友文件失败\n");
               send_buf.n=-5;
           }
           while(read(fd,send_buf.friendname[i++],sizeof(send_buf.username))!=0);
           send_buf.friendname[i][0]='\0';
           close(fd);
           i=0;
        
           while(strlen(send_buf.friendname[i])!=0)
           {
               if(strcmp(send_buf.friendname[i],send_buf.from)==0)
               strcpy(send_buf.friendname[i],"\n");
               i++;
           }
           i=0;
           fd=open(path,O_RDWR|O_CREAT|O_TRUNC);
           if(fd<0)
           {
               printf("打开删除对象好友文件失败\n");
               send_buf.n=-5;
           }
           while(strlen(send_buf.friendname[i])!=0)
           {
               if(strcmp(send_buf.friendname[i],"\n")!=0)
               {
                   if(write(fd,send_buf.friendname[i],sizeof(send_buf.username))<0)
                   {
                       printf("写入删除对象好友文件失败\n");
                       send_buf.n=-5;
                   }
               }
               i++;
           }
           close(fd);
           ret++;
           if(ret==2)
           send_buf.n=55;
           else
           send_buf.n=-5;

           if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
           {
               printf("向客户端发送信息失败\n");
               return;
           }
           break;
       }

       case 6:   //私聊
       {
        
           int i=0,ret=0;
           my_path(send_buf.from,send_buf.from,path);
           fd=open(path,O_RDONLY);
           while(read(fd,send_buf.friendname[i++],sizeof(send_buf.username))!=0);
           send_buf.friendname[i][0]='\0';
           close(fd);
           i=0;
           while(strlen(send_buf.friendname[i])!=0)
           {
               if(strcmp(send_buf.friendname[i],send_buf.to)==0)
               {
                   ret=1;
                   break;

               }
               i++;
           }
           if(ret)
           {
               temp=head->next;
               while(temp!=NULL)
               {
                   if(strcmp(temp->username,send_buf.to)==0)
                   {
                       if(send(temp->cli_fd,&send_buf,sizeof(struct message),0)<0)
                       {
                           printf("发送用户消息失败\n");
                           return;
                       }
                       else
                       {
                           send_buf.n=66;
                           return;
                       }
                       
                   }
                   temp=temp->next;
               }
               my_path("lixian",send_buf.to,path);
               
               fd=open(path,O_RDWR|O_CREAT|O_APPEND,0777);
               if(fd<0)
               {
                   printf("打开指定用户离线消息文件\n");
                   return;
               }
               write(fd,send_buf.time,sizeof(send_buf.time));
               write(fd,send_buf.chat,sizeof(send_buf.chat));
               close(fd);

           }
           else
           {
               send_buf.n=-6;
               if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
               {
                   printf("向客户端发送信息失败\n");
                   return;
               }
           }
           
       }
    
    }
}
int main()
{
    int sock_fd,conn_fd;  //储存套接字
    struct sockaddr_in cli_addr,serv_addr;
    struct message recv_buf;  //读取缓存区
    socklen_t cli_len;
    int epollfd;
    struct epoll_event  event;
    struct epoll_event*  events;
    

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
    if(bind(sock_fd,(struct sockaddr*)&serv_addr,sizeof(struct sockaddr_in))<0)
    {
        my_err("bind");
    }
    //将套接字转化为监听套接字
    if(listen(sock_fd,LISTEN_SIZE)<0)
    {
        my_err("listen");
    }
    cli_len=sizeof(struct sockaddr_in);
    events=(struct epoll_event*)malloc(sizeof(struct epoll_event)*EPOLL_SIZE);
    //创建一个监听描述符epoll,并将监听套接字加入
    epollfd=epoll_create(EPOLL_SIZE);
    if(epollfd==-1)
    {
        my_err("epoll_create");
    }
    event.events = EPOLLIN;
    event.data.fd = sock_fd;
    if(epoll_ctl(epollfd,EPOLL_CTL_ADD,sock_fd,&event)<0)
    {
        my_err("epoll_ctl");
    }

    mkdir("lixian",0777);  //创建离线消息目录

    head=create();  //创建链表
    while(1)
    {
        int sum=0,i;
        sum=epoll_wait(epollfd,events,EPOLL_SIZE,-1);
        for(i=0;i<sum;i++)
        {
            if(events[i].data.fd==sock_fd)    //有请求连接
            {
            
                
                conn_fd=accept(sock_fd,(struct sockaddr*)&cli_addr,&cli_len);
                if(conn_fd<0)
                {
                    my_err("accept");
                }
                
                event.events = EPOLLIN | EPOLLRDHUP;
                event.data.fd = conn_fd;
                if(epoll_ctl(epollfd,EPOLL_CTL_ADD,conn_fd,&event)<0)   //将新连接的套接字加入监听
                {
                    my_err("epoll_ctl");
                }
                printf("a connet is connected,ip is %s,time:%s",inet_ntoa(cli_addr.sin_addr),my_time());
            }
            else if(events[i].events&EPOLLIN)    //有可读的套接字
            {
                
                memset(&recv_buf,0,sizeof(struct message));  
                recv(events[i].data.fd,&recv_buf,sizeof(struct message),0);
                send_message(recv_buf,events[i].data.fd);
                
            }
            if(events[i].events&EPOLLRDHUP)
            {
                
                delete(head,events[i].data.fd);
                printf("a connet is quit,ip is %s,time:%s",inet_ntoa(cli_addr.sin_addr),my_time());
                epoll_ctl(epollfd,EPOLL_CTL_DEL,events[i].data.fd,&event);
                close(events[i].data.fd);
            }
        }
    }
    return 0;
}
