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
#include"message.h"
#define  PORT  6666

GtkWidget* entry_username;  //记录登陆时文本框中的账号信息
GtkWidget* entry_pwd;

GtkWidget* dialog1;

int conn_fd;


GtkWidget *entry_username1; //记录注册文本框中账号信息
GtkWidget *entry_pwd1;
GtkWidget *entry_pwd2;   //记录注册时第二次输入的密码

void my_err(char* string)
{
    perror(string);
    exit(1);
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
    gchar* username;
    gchar* pwd1;
    gchar* pwd2;
    struct message* send_buf,*recv_buf;
    int recv_len=sizeof(struct message);

    memset(&send_buf,0,sizeof(send_buf));
    memeset(&recv_buf,0,sizeof(recv_buf));

    //获取输入信息
    username=gtk_entry_get_text(GTK_ENTRY(entry_username1));
    pwd1=gtk_entry_get_text(GTK_ENTRY(entry_pwd1));
    pwd2=gtk_entry_get_text(GTK_ENTRY(entry_pwd2));

    strcpy(send_buf.username,username);
    strcpy(send_buf.pwd1,pwd1);
    strcpy(send_buf.pwd2,pwd2);
    send_buf.n=0;

    if(send(conn_fd,(struct message*)&send_buf,sizeof(struct message))<0)
    {
        printf("服务器未响应\n");
        return;
    }
    if(recv(conn_fd,(struct message*)&recv_buf,&recv_len)<0)
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
    destroy();
}
void denglu()  //登陆回调函数
{
    gchar* username;
    gchar* pwd;
    struct message send_buf;
    struct message recv_buf;

    memset(&send_buf,0,sizeof(struct message));
    memset(&recv_buf,0,sizeof(struct message));


    //获取输入信息
    username=gtk_entry_get_text(GTK_ENTRY(entry_username));
    pwd=gtk_entry_get_text(GTK_ENTRY(entry_pwd));

    strcpy(send_buf.username,username);
    strcpy(recv_buf.pwd1,pwd);
    send_buf.n=1;

    if(send(conn_fd,(struct message*)&send_buf,sizeof(struct message))<0)
    {
        printf("服务器未响应\n");
        return;
    }
    int recv_len=sizeof(struct message);
    if(recv(conn_fd,(struct message*)&recv_buf,&recv_len)<0)
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
       sys("clear");  //清屏
    
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


int main(int argc,char** argv)
{
    GtkWidget* window;
    GtkWidget* table;
    GtkWidget* textentry;
    GtkWidget* buttonbox;
    
    struct sockaddr_in serv_adrr;
    int serv_port;
    int i;

    if(argc!=3)
    {
        printf("Usage: [-a] [serv_address]\n");
        exit(1);
    }


    //初始化服务器地址结构
    memset(&serv_adrr,0,sizeof(struct sockaddr_in));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(PORT);

    //从命令行服务器地址
    for(i=0;i<argc;i++)
    {
        if(strcmp("-a",argv[i]==0))
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
    if(connect(conn_fd,(struct sockaddr*)&serv_adrr,sizeof(struct sockaddr))<0)
    {
        my_err("connect");
    }



    /*创建主窗口*/
    gtk_init(&argc,&argv);
    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(GTK_OBJECT(window),"destroy",GTK_SIGNAL_FUNC(destroy),NULL);
    gtk_container_border_width(GTK_CONTAINER(window),30); //主窗口大小
    gtk_widget_show(window);

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

}



