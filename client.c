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

#define NONE           "\e[0m"     //不加色，恢复终端颜色
#define L_CYAN         "\e[1;36m"  //深淡蓝色
#define L_PURPLE       "\e[1;35m"  //深紫色
#define L_RED          "\e[1;31m"  //红色
#define L_YELLOW       "\e[1;33m"  //黄色
#define L_GREEN        "\e[1;32m"  //绿色



GtkWidget* entry_username;  //记录登陆时文本框中的账号信息
GtkWidget* entry_pwd;

GtkWidget* dialog1,*window1;

int groupi=0;  //记录群号
char groupname[21];  //记录群名称

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
    
    int ret;
     if((ret=send(conn_fd,&send_buf,sizeof(struct message),0))<0)
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
        label=gtk_label_new("登陆失败，账号信息错误或该账号已登陆");
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
       printf(L_GREEN"欢迎使用聊天系统,输入help查看命令帮助"NONE);
       printf("\n");
    
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
            while(*p==' ')
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
            printf(L_RED"不能添加自己为好友\n"NONE);
            return;
        }
    
        if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
        {
            printf(L_RED"服务器未响应\n"NONE);
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
            printf(L_RED"服务器未响应\n"NONE);
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
            printf(L_RED"服务器未响应\n"NONE);
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
            printf(L_RED"未找到该命令\n"NONE);
            return;
        }

    
        if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
        {
            printf(L_RED"服务器未响应\n"NONE);
            return;
        }
    }
    else if(strcmp(buflist[0],"delete")==0)    //删除好友
    {
        if(strlen(buflist[1])!=0)
        {
            strcpy(send_buf.to,buflist[1]);
            strcpy(send_buf.from,username1);
            send_buf.n=5;

            
            if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
            {
                printf(L_RED"服务器未响应\n"NONE);
                return;
            }
        }
        else
        {
            printf(L_RED"命令格式错误，请重新输入\n"NONE);
            return;
        }
    }
    
    else if(strcmp(buflist[0],"chat")==0)  //聊天
    {
        if(strcmp(buflist[1],"to")==0&&strlen(buflist[2])!=0&&strlen(buflist[3])!=0&&strlen(buflist[4])==0)  //私聊
        {
            send_buf.n=6;
            strcpy(send_buf.to,buflist[2]);
            strcpy(send_buf.chat,buflist[3]);
            strcpy(send_buf.time,my_time());
            strcpy(send_buf.from,username1);

            if(strcmp(send_buf.to,username1)==0)  //不能对自己发送聊天消息
            {
                printf(L_RED"您不能对自己发送消息哦"NONE);
                printf("\n");
                return;
            }

            
            
            if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
            {
                printf("服务器未响应\n");
                return;
            }
            
            printf(L_PURPLE"%s"NONE,send_buf.time);   //将自己的输入内容显示
            printf(L_PURPLE"[私聊]我@%s:"NONE,send_buf.to);
            printf(L_PURPLE"%s"NONE,send_buf.chat);
            printf("\n");

        }
        else if(strcmp(buflist[1],"all")==0&&strlen(buflist[2])!=0&&strlen(buflist[3])==0)  //群聊
        {
            send_buf.n=7;
            if(groupi==0)
            {
                printf(L_RED"您未加入任何群"NONE);
                printf("\n");
                return;
            }
            strcpy(send_buf.from,username1);
            strcpy(send_buf.groupname,groupname);
            strcpy(send_buf.chat,buflist[2]);
            strcpy(send_buf.time,my_time());
            send_buf.groupi=groupi;

        
            if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
            {
                printf(L_RED"服务器未响应"NONE);
                printf("\n");
                return;
            }

            printf(L_PURPLE"%s"NONE,send_buf.time);
            printf(L_PURPLE"[群聊]我@%s:"NONE,send_buf.groupname);
            printf(L_PURPLE"%s"NONE,send_buf.chat);
            printf("\n");

        }
       
        else
        {
            printf(L_RED"未找到该命令\n"NONE);
            return;
        }
    }


    else if(strcmp(buflist[0],"clear")==0&&strlen(buflist[1])==0)  //清屏
    {
        system("clear");
    }
    else if(strcmp(buflist[0],"exit")==0&&strlen(buflist[1])==0)  //退出客户端
    {
        close(conn_fd);
        printf(L_GREEN"退出聊天程序，欢迎下次使用"NONE);
        printf("\n");
        exit(1);
        
    }
    else if(strcmp(buflist[0],"group")==0)    //讨论组
    {
        if(strcmp(buflist[1],"create")==0&&strlen(buflist[2])!=0&&strlen(buflist[3])==0)  //创建讨论组
        {
            if(groupi!=0)
            {
                printf(L_RED"您已加入一个讨论组，创建失败\n"NONE);
                printf("\n");
                return;
            }
            send_buf.n=77;
            strcpy(send_buf.groupname,buflist[2]);
            strcpy(send_buf.from,username1);

            
            if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
            {
                printf(L_RED"服务器未响应\n"NONE);
                return;
            }
        

        }
        else if(strcmp(buflist[1],"add")==0&&strlen(buflist[2])!=0&&strlen(buflist[3])==0)  //邀请好友加入讨论组
        {
            
            if(groupi==0)
            {
                printf(L_RED"您未加入任何群"NONE);
                printf("\n");
                return;
            }
            send_buf.n=777;
            send_buf.groupi=groupi;
            strcpy(send_buf.from,username1);
            strcpy(send_buf.to,buflist[2]);
            strcpy(send_buf.groupname,groupname);

            
            if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
            {
                printf(L_RED"服务器未响应\n"NONE);
                return;
            }
            
        }
        else if(strcmp(buflist[1],"exit")==0&&strlen(buflist[2])==0)  //退出讨论组
        {
            if(groupi==0)
            {
                printf(L_RED"您未加入任何群"NONE);
                printf("\n");
                return;
            }
            send_buf.n=-7;
            send_buf.groupi=groupi;

            if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
            {
                printf(L_RED"退出群失败"NONE);
                printf("\n");
                return;
            }
            else
            {
                printf(L_YELLOW"退出群成功"NONE);
                printf("\n");
                groupi=0;
                groupname[0]='\0';
                return;
            }
        }
        else if(strcmp(buflist[1],"ls")==0&&strlen(buflist[2])==0)  //查看讨论组成员
        {
            if(groupi==0)
            {
                printf(L_RED"您未加入任何群"NONE);
                printf("\n");
                return;
            }
            send_buf.n=7777;
            send_buf.groupi=groupi;

    
            if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
            {
                printf(L_RED"查看群成员失败"NONE);
                printf("\n");
                return;
            }
        }
        else
        {
            printf(L_RED"未找到该命令"NONE);
            printf("\n");
            return;
        }

    }
    else if(strcmp(buflist[0],"history")==0&&strlen(buflist[1])!=0&&strlen(buflist[2])==0)  //查看与好友聊天记录
    {
    
        send_buf.n=8;
        strcpy(send_buf.from,username1);
        strcpy(send_buf.to,buflist[1]);


        if(send(conn_fd,&send_buf,sizeof(struct message),0)<0)
        {
            printf(L_RED"服务器未响应"NONE);
            return;            
        }

        printf(L_YELLOW"与好友%s的聊天记录:"NONE,send_buf.to);
        printf("\n");
   
    }
    else if(strcmp(buflist[0],"help")==0&&strlen(buflist[1])==0)   //查看帮助命令
    {
        printf(L_YELLOW"命令格式如下:"NONE);
        printf("\n");
        printf(L_GREEN"查看所有好友: ls -a "NONE);
        printf("\n");
        printf(L_GREEN"查看在线好友：ls");
        printf("\n");
        printf(L_GREEN"添加好友: add  好友名称"NONE);
        printf("\n");
        printf(L_GREEN"删除好友：delete  好友名称"NONE);
        printf("\n");
        printf(L_GREEN"私聊：chat to 好友名称　内容"NONE);
        printf("\n");
        printf(L_GREEN"群聊：chat all  内容"NONE);
        printf("\n");
        printf(L_GREEN"创建群：group create 群名称"NONE);
        printf("\n");
        printf(L_GREEN"邀请好友进群：group add 好友名称"NONE);
        printf("\n");
        printf(L_GREEN"退出群(群主退出群会自动解散)：group exit"NONE);
        printf("\n");
        printf(L_GREEN"查看聊天记录：history 好友名称"NONE);
        printf("\n");
        printf(L_GREEN"查看帮助：help"NONE);
        printf("\n");
        printf(L_GREEN"清屏：clear"NONE);
        printf("\n");
        printf(L_GREEN"退出程序：exit"NONE);
        printf("\n");
    }
    else
    {
        printf(L_RED"未找到该命令\n"NONE);
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
        printf(L_YELLOW"用户:%s 请求添加您为好友，是否同意？yes/no."NONE,username);
        printf("\n");
      
    }
    if(n==33)
    {
        printf(L_YELLOW"添加好友成功"NONE);
        printf("\n");
    }
    if(n==-3)
    {
        printf(L_YELLOW"好友不在线或对方拒绝添加您为好友"NONE);
        printf("\n");
    }
    if(n==4)  //显示所有好友
    {
        int i=0;
        printf(L_YELLOW"所有好友:"NONE);
        printf("\n");
        while(strlen(recv_buf.friendname[i])!=0)
        {
            printf("%s\n",recv_buf.friendname[i]);
            i++;
        }
        printf("\n");

    }
    if(n==44) //显示在线好友
    {
        int i=0;
        printf(L_YELLOW"在线好友:"NONE);
        printf("\n");
        while(strlen(recv_buf.friendname[i])!=0)
        {
            printf("%s\n",recv_buf.friendname[i]);
            i++;        
        }
        printf("\n");
    }
    if(n==55)  //删除好友成功
    {
        printf(L_YELLOW"删除好友:%s　成功"NONE,recv_buf.to);
        printf("\n");
    }
    if(n==-5)  //删除好友失败
    {
        printf(L_YELLOW"删除好友失败"NONE);
        printf("\n");
    }
    if(n==6)   //私聊
    {
        printf(L_CYAN"%s"NONE,recv_buf.time);
        printf(L_CYAN"[私聊]"NONE);
        printf(L_CYAN"%s@我:"NONE,recv_buf.from);
        printf(L_CYAN"%s"NONE,recv_buf.chat);
        printf("\n");

    }
    if(n==-6)  //发送消息失败
    {
        printf(L_RED"发送消息失败,未找到该好友"NONE);
        printf("\n");
    }
    if(n==7)   //群聊
    {
        printf(L_CYAN"%s"NONE,recv_buf.time);
        printf(L_CYAN"[群聊]"NONE);
        printf(L_CYAN"%s@%s:"NONE,recv_buf.from,recv_buf.groupname);
        printf(L_CYAN"%s"NONE,recv_buf.chat);
        printf("\n");
    }
    if(n==77)   //创建讨论组成功
    {
        printf(L_YELLOW"创建群:%s 成功"NONE,recv_buf.groupname);
        printf("\n");
        groupi=recv_buf.groupi;
        strcpy(groupname,recv_buf.groupname);
    }
    if(n==-77)  //创建讨论组失败
    {
        printf(L_YELLOW"创建群:%s 失败"NONE,recv_buf.groupname);
        printf("\n");
    }
    if(n==777)  //好友邀请加入讨论组
    {
        printf(L_YELLOW"好友:%s 邀请您加入群:%s"NONE,recv_buf.from,recv_buf.groupname);
        printf("\n");
        groupi=recv_buf.groupi;
        strcpy(groupname,recv_buf.groupname);
    }
    if(n==-777)  //邀请好友加入讨论组失败
    {
        printf(L_YELLOW"邀请好友:%s 进群:%s 失败,对方不是您好友或者不在线"NONE,recv_buf.to,recv_buf.groupname);
        printf("\n");
    }
    if(n==7777)  //查看讨论组成员
    {
        int i=0;
        printf(L_GREEN"群%s中的成员:"NONE,groupname);
        printf("\n");
        while(strlen(recv_buf.friendname[i])!=0)
        {
            printf(L_PURPLE"%s"NONE,recv_buf.friendname[i]);
            printf("\n");
            i++;
        }
    }
    if(n==-7777)  //群主退出。群解散
    {
        printf(L_RED"由于群主退出，群%s已解散"NONE,groupname);
        printf("\n");
        groupi=0;
        groupname[0]='\0';
    }
    if(n==8)   //查看聊天记录
    {
        printf(L_PURPLE"%s"NONE,recv_buf.chathistory); 

    }
    if(n==-8) //查看聊天记录失败
    {
        printf(L_RED"查看聊天记录失败，对方并不是您好友"NONE);
        printf("\n");
        return;
    }


}
void recv_pthread()   //接收服务器数据线程
{
    struct message send_buf,recv_buf;
    int ret=0; 
    int len=sizeof(struct message);
    int sum;
    struct message* p=NULL;
    memset(&send_buf,0,sizeof(struct message));
    memset(&recv_buf,0,sizeof(struct message));

    while(1)
    {
        ret=0;
        sum=0;
        p=&recv_buf;
        strcpy(recv_buf.chathistory,"");
       while(sum!=len)
      {
        p+=ret;
        if((ret=recv(conn_fd,p,len-sum,0))==0)
        {
            printf("从服务器接收数据失败\n");
            exit(1);
        }
        else if(ret<0)
        {
            printf("从服务器接受数据失败\n");
            return;
        }
        sum+=ret;

        
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
            printf(L_RED"服务器退出，程序退出\n"NONE);
        
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
           printf(L_RED"输入的字符过长,请重新输入\n"NONE);
           continue;   
        }
        else
        {
            buf[--j]='\0';
            if(explain_buf(buf,buflist)==0)
            {
                printf(L_RED"输入的命令格式不正确，请重新输入\n"NONE);
                continue;                    
            }
            do_buf(buflist,conn_fd);                    
        }
    }

   

   

}



