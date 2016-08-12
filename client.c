/*************************************************************************
> File Name: gtk.c
> Author: FengXin
> Mail: fengxinlinux@gmail.com
> Created Time: 2016年08月05日 星期五 16时46分07秒
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
#include<gtk/gtk.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<sys/epoll.h>
#include<time.h>
#include<signal.h>
#include"message.h"
#define  PORT  6666
#define  EPOLL_SIZE 10       //监听数量



GtkWidget* entry_username;  //记录登陆时文本框中的账号信息
GtkWidget* entry_pwd;

GtkWidget* dialog1,*window1;

int conn_fd;
time_t timep;    //获取当前时间的变量
char username1[21];   //记录当前客户端用户名
char from[21]; //记录发送信息的用户名
char to[21];  //记录被发送信息的用户名



GtkWidget *entry_username1; //记录注册文本框中账号信息
GtkWidget *entry_pwd1;
GtkWidget *entry_pwd2;   //记录注册时第二次输入的密码

void my_err(char* string)
{
    perror(string);
    exit(1);
}
char* my_time()    //获取当前时间
{
    time(&timep);
    char* p=ctime(&timep);
    return p;
}
    

void destroy(GtkWidget *widget,gpointer* data)  //退出图形界面
{
    gtk_main_quit();
}

void zhuce_no(GtkWidget*widget,gpointer* dialog)     //否。注册回调函数
{
    gtk_widget_destroy(GTK_WIDGET(dialog));
}
void zhuce_fail(GtkWidget* widget,gpointer* dialog)    //注册失败回调函数
{
    gtk_widget_destroy(GTK_WIDGET(dialog));

}
void zhuce_success(GtkWidget* widget,gpointer* dialog)  //注册成功回调函数
{
    gtk_widget_destroy(GTK_WIDGET(dialog));
    gtk_widget_destroy(GTK_WIDGET(dialog1));
}
void zhuce_yes()       //是。注册回调函数
{
    gchar* username=(char*)malloc(sizeof(char)*21);
    gchar* pwd1=(char*)malloc(sizeof(char)*21);
    gchar* pwd2=(char*)malloc(sizeof(char)*21);
    struct message  send_buf,recv_buf;
    int recv_len=sizeof(struct message);

    memset(&send_buf,0,sizeof(send_buf));
    memset(&recv_buf,0,sizeof(recv_buf));

    //获取输入信息
    strcpy(username,gtk_entry_get_text(GTK_ENTRY(entry_username1)));
    strcpy(pwd1,gtk_entry_get_text(GTK_ENTRY(entry_pwd1)));
    strcpy(pwd2,gtk_entry_get_text(GTK_ENTRY(entry_pwd2)));

    strcpy(send_buf.username,username);
    strcpy(send_buf.pwd1,pwd1);
    strcpy(send_buf.pwd2,pwd2);
    send_buf.n=0;

    if(strcmp(pwd1,pwd2)!=0)
    {
         GtkWidget* dialog;
         GtkWidget* label;
         GtkWidget* button;
         GtkWidget* vbox;
         GtkWidget* hbox;
         dialog=gtk_dialog_new();
         //向对话框中加入一个文本标签
         vbox=GTK_DIALOG(dialog)->vbox;
         label=gtk_label_new("输入的两次密码不匹配");
         gtk_box_pack_start(GTK_BOX(vbox),label,TRUE,TRUE,30);
        //向对话框中加入一个按钮     
        hbox=GTK_DIALOG(dialog)->action_area;   
        button=gtk_button_new_with_label("确定");
        gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
        g_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(zhuce_fail),dialog);

        gtk_widget_show_all(dialog);
        return;
    }

    if(strlen(pwd1)==0||strlen(username)==0)
    {
         GtkWidget* dialog;
         GtkWidget* label;
         GtkWidget* button;
         GtkWidget* vbox;
         GtkWidget* hbox;
         dialog=gtk_dialog_new();
         //向对话框中加入一个文本标签
         vbox=GTK_DIALOG(dialog)->vbox;
         label=gtk_label_new("账号或密码不能为空");
         gtk_box_pack_start(GTK_BOX(vbox),label,TRUE,TRUE,30);
        //向对话框中加入一个按钮     
         hbox=GTK_DIALOG(dialog)->action_area;   
        button=gtk_button_new_with_label("确定");
        gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
        g_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(zhuce_fail),dialog);
        gtk_widget_show_all(dialog);
        return;
        
    }
    


    if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
    {
        printf("服务器未响应\n");
        return;
    }
    if(recv(conn_fd,&recv_buf,recv_len,0)<0)
    {
        printf("从服务器接受数据失败\n");
        return;
    }
    if(recv_buf.n==-2)  //注册失败
    {

        GtkWidget* dialog;
        GtkWidget* label;
        GtkWidget* button;
        GtkWidget* vbox;
        GtkWidget* hbox;
        dialog=gtk_dialog_new();

        //向对话框中加入一个文本标签
        vbox=GTK_DIALOG(dialog)->vbox;
        label=gtk_label_new("注册失败，账号已存在");
        gtk_box_pack_start(GTK_BOX(vbox),label,TRUE,TRUE,30);

        //向对话框中加入一个按钮
        hbox=GTK_DIALOG(dialog)->action_area;
        button=gtk_button_new_with_label("确定");
        gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
        g_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(zhuce_fail),dialog);



        gtk_widget_show_all(dialog);
    }
    if(recv_buf.n==22)   //注册成功
   {

       GtkWidget* dialog;
       GtkWidget* label;
       GtkWidget* button;
       GtkWidget* vbox;
       GtkWidget* hbox;
       dialog=gtk_dialog_new();

       //向对话框中加入一个文本标签
       vbox=GTK_DIALOG(dialog)->vbox;
       label=gtk_label_new("注册成功");
       gtk_box_pack_start(GTK_BOX(vbox),label,TRUE,TRUE,30);

       //向对话框中加入一个按钮
       hbox=GTK_DIALOG(dialog)->action_area;
       button=gtk_button_new_with_label("确定");
       gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
       g_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(zhuce_success),dialog);

       gtk_widget_show_all(dialog);
   }

}

void zhuce()              /////////注册对话框
{
    GtkWidget* dialog;
    GtkWidget* label;
    GtkWidget* button;
    GtkWidget* vbox,*text;
    GtkWidget* hbox;
    GtkWidget* image;
    dialog=gtk_dialog_new();
    vbox=GTK_DIALOG(dialog)->vbox;
    dialog1=dialog;

    /*插入图片*/
    image=gtk_image_new_from_file("2.png");
    gtk_box_pack_start(GTK_BOX(vbox),image,FALSE,FALSE,0);
    gtk_widget_show(image);

    /*生成标签控件*/
    label=gtk_label_new("账号：");
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
    gtk_widget_show(label);

    /*生成编辑框*/
    text=gtk_entry_new_with_max_length(21);
    gtk_box_pack_start(GTK_BOX(vbox),text,FALSE,FALSE,0);

    //获取输入信息
    entry_username1=text;

    gtk_widget_show(text);



    /*生成标签控件*/
    label=gtk_label_new("密码：");
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
    gtk_widget_show(label);

    /*生成编辑框*/
    text=gtk_entry_new_with_max_length(21);          
    gtk_box_pack_start(GTK_BOX(vbox),text,FALSE,FALSE,0);
    //设置密码框不可见，用户输入时显示“*”
    gtk_entry_set_visibility(GTK_ENTRY(text),FALSE);
    gtk_entry_set_invisible_char(GTK_ENTRY(text),'*');


    //获取输入信息
    entry_pwd1=text;

    gtk_widget_show(text);




    /*生成标签控件*/
    label=gtk_label_new("确认密码：");
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
    gtk_widget_show(label);

    /*生成编辑框*/
    text=gtk_entry_new_with_max_length(21);          
    gtk_box_pack_start(GTK_BOX(vbox),text,FALSE,FALSE,0);
    //设置密码框不可见，用户输入时显示“*”
    gtk_entry_set_visibility(GTK_ENTRY(text),FALSE);
    gtk_entry_set_invisible_char(GTK_ENTRY(text),'*');
    gtk_widget_show(text);
    //获取输入信息
    entry_pwd2=text;


    gtk_widget_show(vbox);




    //向对话框中加入两个按钮
    hbox=GTK_DIALOG(dialog)->action_area;
    button=gtk_button_new_with_label("注册");
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    g_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(zhuce_yes),dialog);            
    button=gtk_button_new_with_label("取消");
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    g_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(zhuce_no),dialog);

    gtk_widget_show_all(dialog);

}

void denglu_fail(GtkWidget* widget,gpointer* dialog)    //登陆失败回调函数
{
    gtk_widget_destroy(GTK_WIDGET(dialog));

}
void denglu_success(GtkWidget* widget,gpointer* dialog)  //登陆成功回调函数
{
    gtk_widget_destroy(GTK_WIDGET(dialog));
    gtk_widget_destroy(window1);   
    gtk_main_quit();
}
void denglu()  //登陆回调函数
{
    gchar* username;
    gchar* pwd;
    struct message send_buf;
    struct message recv_buf;

    username=(char*)malloc(sizeof(char)*21);
    pwd=(char*)malloc(sizeof(char)*21);




    memset(&send_buf,0,sizeof(struct message));
    memset(&recv_buf,0,sizeof(struct message));
   

    //获取输入信息
    strcpy(username,gtk_entry_get_text(GTK_ENTRY(entry_username)));
    strcpy(pwd,gtk_entry_get_text(GTK_ENTRY(entry_pwd)));

 
  
    strcpy(username1,username);
    
    


    strcpy(send_buf.username,username);
    strcpy(send_buf.pwd1,pwd);
    send_buf.n=1;
   
    if(strlen(send_buf.username)==0||strlen(send_buf.pwd1)==0)
    {
         GtkWidget* dialog;
         GtkWidget* label;
         GtkWidget* button;
         GtkWidget* vbox;
         GtkWidget* hbox;
         dialog=gtk_dialog_new();
        //向对话框中加入一个文本标签
         vbox=GTK_DIALOG(dialog)->vbox;
        label=gtk_label_new("账号或密码不能为空");
        gtk_box_pack_start(GTK_BOX(vbox),label,TRUE,TRUE,30);
        //向对话框中加入一个按钮     
        hbox=GTK_DIALOG(dialog)->action_area;   
        button=gtk_button_new_with_label("确定");
        gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
        g_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(denglu_fail),dialog);
        gtk_widget_show_all(dialog);
        return;
    }


    
    
    if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
    {
        printf("服务器未响应\n");
        return;
    }
    
    int recv_len=sizeof(struct message);
    if(recv(conn_fd,&recv_buf,recv_len,0)<0)
    {
        printf("从服务器接受数据失败\n");
        return;

    }
    
    if(recv_buf.n==-1)   //登陆失败
    {
        GtkWidget* dialog;
        GtkWidget* label;
        GtkWidget* button;
        GtkWidget* vbox;
        GtkWidget* hbox;
        dialog=gtk_dialog_new();

        //向对话框中加入一个文本标签
        vbox=GTK_DIALOG(dialog)->vbox;
        label=gtk_label_new("登陆失败，账号或密码错误");
        gtk_box_pack_start(GTK_BOX(vbox),label,TRUE,TRUE,30);

        //向对话框中加入一个按钮
        hbox=GTK_DIALOG(dialog)->action_area;
        button=gtk_button_new_with_label("确定");
        gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
        g_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(denglu_fail),dialog);



        gtk_widget_show_all(dialog);
    }
    if(recv_buf.n==11) //登陆成功
   {

       GtkWidget* dialog;
       GtkWidget* label;
       GtkWidget* button;
       GtkWidget* vbox;
       GtkWidget* hbox;
       dialog=gtk_dialog_new();

       //向对话框中加入一个文本标签
       vbox=GTK_DIALOG(dialog)->vbox;
       label=gtk_label_new("登陆成功");
       gtk_box_pack_start(GTK_BOX(vbox),label,TRUE,TRUE,30);

       //向对话框中加入一个按钮
       hbox=GTK_DIALOG(dialog)->action_area;
       button=gtk_button_new_with_label("确定");
       gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
       g_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(denglu_success),dialog);



       gtk_widget_show_all(dialog);
       system("clear");  //清屏
       system("clear");
       system("clear");
    
   }
}
GtkWidget* makebuttonbox()
{
    GtkWidget* hbox;
    GtkWidget* button;
    hbox=gtk_hbox_new(FALSE,0);

    button=gtk_button_new_with_label("登陆");
    gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,20);
    g_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(denglu),NULL);           ////////////连接登陆回调函数
    gtk_widget_show(button);

    button=gtk_button_new_with_label("注册");
    gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,60);    
    g_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(zhuce),NULL);
    gtk_widget_show(button);

    return hbox;

}
GtkWidget* maketextentry()
{
    GtkWidget* vbox;
    GtkWidget* label;
    GtkWidget* text;
    GtkWidget* image;

    vbox=gtk_vbox_new(FALSE,5);

    /*插入图片*/
    image=gtk_image_new_from_file("1.png");
    gtk_box_pack_start(GTK_BOX(vbox),image,FALSE,FALSE,0);
    gtk_widget_show(image);

    /*生成标签控件*/
    label=gtk_label_new("账号：");
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
    gtk_widget_show(label);

    /*生成编辑框*/
    text=gtk_entry_new_with_max_length(21);
    gtk_box_pack_start(GTK_BOX(vbox),text,FALSE,FALSE,0);
    /*获取输入信息*/
    entry_username=text;

    gtk_widget_show(text);



    /*生成标签控件*/
    label=gtk_label_new("密码：");
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
    gtk_widget_show(label);

    /*生成编辑框*/
    text=gtk_entry_new_with_max_length(21);          
    gtk_box_pack_start(GTK_BOX(vbox),text,FALSE,FALSE,0);
    //设置密码框不可见，用户输入时显示“*”
    gtk_entry_set_visibility(GTK_ENTRY(text),FALSE);
    gtk_entry_set_invisible_char(GTK_ENTRY(text),'*');

    //获取输入信息
    entry_pwd=text;

    gtk_widget_show(text);



    return vbox;



}
int  explain_buf(char* buf,char buflist[5][21])       //解析用户输入的命令
{
    int i=0,j=0;
    char* p=buf;
    int n=1;  //记录命令格式是否正确

    for(i=0;i<5;i++)   //初始化
    buflist[i][0]='\0';
    i=0;

    while(*p!='\0')
    {
        if(*p!=' '&&i<5)
        {
            buflist[i][j++]=*p;
            p++;
            if(j==20)
            {
                n=0;
                break;
            }
        }
        else
        {
            buflist[i][j]='\0';
            p++;
            i++;
            j=0;
        }
    }
    buflist[i][j]='\0';
    return n;

}

void do_buf(char buflist[5][21],int conn_fd)   //执行用户命令
{
    struct message send_buf,recv_buf;
    memset(&send_buf,0,sizeof(struct message));
    memset(&recv_buf,0,sizeof(struct message));


    if(strcmp(buflist[0],"add")==0)    //添加好友命令
    {
        send_buf.n=3;
        strcpy(send_buf.to,buflist[1]);
        strcpy(send_buf.from,username1);
    
        if(strcmp(send_buf.from,send_buf.to)==0)   
        {
            printf("不能添加自己为好友\n");
            return;
        }
        if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
        {
            printf("服务器未响应\n");
            return;
        }
    }

   else if(strcmp(buflist[0],"yes")==0)   //确认添加好友命令
    {
        send_buf.n=33;
        strcpy(send_buf.from,from);
        strcpy(send_buf.to,to);
        if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
        {
            printf("服务器未响应\n");
            return;
        }

    }
    else if(strcmp(buflist[0],"no")==0)   //拒绝添加好友命令
    {
        send_buf.n=-3;
        strcpy(send_buf.from,from);
        strcpy(send_buf.to,to);

        if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
        {
            printf("服务器未响应\n");
            return;
        }
    }
    else if(strcmp(buflist[0],"ls")==0)  //显示好友
    {
        strcpy(send_buf.username,username1);
        if(strcmp(buflist[1],"-a")==0)  //显示所有好友
        {
            send_buf.n=4;
        }
        else if(strlen(buflist[1])==0)
        {
            send_buf.n=44;
        }
        else
        {
            printf("未找到该命令\n");
            return;
        }
        if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
        {
            printf("服务器未响应\n");
            return;
        }
    }


    else if(strcmp(buflist[0],"clear")==0&&strlen(buflist[1])==0)  //清屏
    {
        system("clear");
    }
    else
    {
        printf("未找到该命令\n");
        return;
    }


}
void do_recv(struct message recv_buf)    //执行处理从服务器发来的数据
{
    int n=recv_buf.n;
    struct message send_buf=recv_buf;
    char username[21];  
    if(n==3)   //有用户申请添加好友
    {
        char choice[10];
        int i=0;
        strcpy(username,recv_buf.from);
        strcpy(from,recv_buf.from);
        strcpy(to,recv_buf.to);
        printf("%s请求添加您为好友，是否同意？yes/no.\n",username);
      
    }
    if(n==33)
    {
        printf("添加好友成功\n");
    }
    if(n==-3)
    {
        printf("好友不在线或对方拒绝添加您为好友\n");
    }
    if(n==4)  //显示所有好友
    {
        int i=0;
        printf("所有好友:\n");
        while(strlen(recv_buf.friendname[i])!=0)
        {
            printf("%s\n",recv_buf.friendname[i]);
            i++;
        }

    }
    if(n==44) //显示在线好友
    {
        int i=0;
        printf("在线好友:\n");
        while(strlen(recv_buf.friendname[i])!=0)
        {
            printf("%s\n",recv_buf.friendname[i]);
            i++;        
        }
    }

}
void recv_pthread()   //接收服务器数据线程
{
    struct message send_buf,recv_buf;
    int ret; 
    memset(&send_buf,0,sizeof(struct message));
    memset(&recv_buf,0,sizeof(struct message));

    while(1)
    {

        if((ret=recv(conn_fd,&recv_buf,sizeof(struct message),0))==0)
        {
            printf("从服务器接收数据失败\n");
            exit(1);
        }
        else if(ret<0)
        {
            printf("从服务器接受数据失败\n");
            continue;
        }
        do_recv(recv_buf);

    }
}
void serv_quit()  //监控服务器退出
{
    int epollfd=epoll_create(1);
    struct epoll_event event;
    struct epoll_event events[1];
    event.data.fd=conn_fd;
    event.events=EPOLLRDHUP;
    if(epoll_ctl(epollfd,EPOLL_CTL_ADD,conn_fd,&event)<0)
    {
        printf("epoll_ctl error");
        pthread_exit(NULL);
    }
    while(1)
    {
        epoll_wait(epollfd,events,1,-1);
        if(events[0].events&EPOLLRDHUP)
        {
            printf("服务器退出，程序退出\n");
            //shutdown(conn_fd,2);
          //  close(conn_fd);
            exit(1);
        }
    }
}
int main(int argc,char** argv)
{
    GtkWidget* window;
    GtkWidget* table;
    GtkWidget* textentry;
    GtkWidget* buttonbox;


    
    struct sockaddr_in serv_addr;
    int serv_port;
    int i;


    if(argc!=3)
    {
        printf("Usage: [-a] [serv_address]\n");
        exit(1);
    }


    

    //初始化服务器地址结构
    memset(&serv_addr,0,sizeof(struct sockaddr_in));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(PORT);


    //从命令行服务器地址
    for(i=0;i<argc;i++)
    {
        if(strcmp("-a",argv[i])==0)
        {
            
            if(inet_aton(argv[i+1],&serv_addr.sin_addr)==0)
            {
                printf("invaild server ip address\n");
                exit(1);
            }
            break;
        }
    }

    //检查是否少输入了某项参数
    if(serv_addr.sin_addr.s_addr==0)
    {
        printf("Usage: [-a] [serv_address]\n");
        exit(1);
    }

    //创建一个TCP套接字
    conn_fd=socket(AF_INET,SOCK_STREAM,0);


    if(conn_fd<0)
    {
        my_err("connect");
    }

    //向服务器发送连接请求
    if(connect(conn_fd,(struct sockaddr*)&serv_addr,sizeof(struct sockaddr))<0)
    {
        my_err("connect");
    }




    /*创建主窗口*/
    gtk_init(&argc,&argv);
    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(GTK_OBJECT(window),"destroy",GTK_SIGNAL_FUNC(destroy),NULL);
    gtk_container_border_width(GTK_CONTAINER(window),30); //主窗口大小
    gtk_widget_show(window);
    window1=window; 

    /*创建table控件*/
    table=gtk_table_new(2,2,FALSE);
    gtk_widget_show(table);

    /*创建标签和编辑框*/
    textentry=maketextentry();
    gtk_table_attach(GTK_TABLE(table),textentry,0,1,0,1,GTK_FILL|GTK_EXPAND|GTK_SHRINK,0,0,0);
    gtk_widget_show(textentry);


    /*创建两个按钮*/
    buttonbox=makebuttonbox();
    gtk_table_attach(GTK_TABLE(table),buttonbox,0,2,1,2,GTK_EXPAND|GTK_FILL|GTK_SHRINK,0,5,10);
    gtk_widget_show(buttonbox);

    gtk_container_add(GTK_CONTAINER(window),table);


    gtk_main();



    
    
    char buf[101];  //储存用户输入的命令
    struct message send_buf;
    struct message recv_buf;
    char buflist[5][21];  //储存将命令分解
    pthread_t thid,thid2; 

    memset(&send_buf,0,sizeof(struct message));
    memset(&recv_buf,0,sizeof(struct message));

    pthread_create(&thid,NULL,(void*)recv_pthread,NULL);
    pthread_create(&thid2,NULL,(void*)serv_quit,NULL);

    while(1)
    {
        int j=0;
        while(j!=101&&(buf[j++]=getchar())!='\n');
        if(j==101)
        {
           printf("输入的字符过长,请重新输入\n");
           continue;   
        }
        else
        {
            buf[--j]='\0';
            if(explain_buf(buf,buflist)==0)
            {
                printf("输入的命令格式不正确，请重新输入\n");
                continue;                    
            }
            do_buf(buflist,conn_fd);                    
        }
    }

   

   

}



