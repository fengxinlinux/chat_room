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

GtkWidget *entry_username; //两个文本框，用于输入用户名和密码
GtkWidget *entry_pwd1;
GtkWidget *entry_pwd2;   //记录注册时第二次输入的密码

void destroy(GtkWidget *widget,gpointer* data)  //退出图形界面
{
    gtk_main_quit();
}

void no(GtkWidget*widget,gpointer* dialog)     //否。对调函数
{
    gtk_widget_destroy(GTK_WIDGET(dialog));
}
void make_dialog()                    //确认注册提示文本框
{
    GtkWidget* dialog;
    GtkWidget* label;
    GtkWidget* button;
    GtkWidget* vbox;
    GtkWidget* hbox;
    dialog=gtk_dialog_new();

    //向对话框中加入一个文本标签
    vbox=GTK_DIALOG(dialog)->vbox;
    label=gtk_label_new("是否确定注册账号");
    gtk_box_pack_start(GTK_BOX(vbox),label,TRUE,TRUE,30);

    //向对话框中加入两个按钮
    hbox=GTK_DIALOG(dialog)->action_area;
    button=gtk_button_new_with_label("是");
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    //g_signal_connect();            //////////////连接是回调函数
    button=gtk_button_new_with_label("否");
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    g_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(no),dialog);


    gtk_widget_show_all(dialog);

}
void zhuce_if()       //注册确定按钮回调函数
{
    make_dialog();
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

     /*插入图片*/
    image=gtk_image_new_from_file("22.png");
    gtk_box_pack_start(GTK_BOX(vbox),image,FALSE,FALSE,0);
    gtk_widget_show(image);

    /*生成标签控件*/
    label=gtk_label_new("账号：");
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
    gtk_widget_show(label);

    /*生成编辑框*/
    text=gtk_entry_new_with_max_length(20);
    gtk_box_pack_start(GTK_BOX(vbox),text,FALSE,FALSE,0);
    gtk_widget_show(text);



    /*生成标签控件*/
    label=gtk_label_new("密码：");
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
    gtk_widget_show(label);

    /*生成编辑框*/
    text=gtk_entry_new_with_max_length(20);          
    gtk_box_pack_start(GTK_BOX(vbox),text,FALSE,FALSE,0);
    //设置密码框不可见，用户输入时显示“*”
    gtk_entry_set_visibility(GTK_ENTRY(text),FALSE);
    gtk_entry_set_invisible_char(GTK_ENTRY(text),'*');
    gtk_widget_show(text);




    /*生成标签控件*/
    label=gtk_label_new("确认密码：");
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
    gtk_widget_show(label);

    /*生成编辑框*/
    text=gtk_entry_new_with_max_length(20);          
    gtk_box_pack_start(GTK_BOX(vbox),text,FALSE,FALSE,0);
    //设置密码框不可见，用户输入时显示“*”
    gtk_entry_set_visibility(GTK_ENTRY(text),FALSE);
    gtk_entry_set_invisible_char(GTK_ENTRY(text),'*');
    gtk_widget_show(text);


    gtk_widget_show(vbox);




    //向对话框中加入两个按钮
    hbox=GTK_DIALOG(dialog)->action_area;
    button=gtk_button_new_with_label("注册");
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    g_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(zhuce_if),dialog);            //////////////连接是回调函数
    button=gtk_button_new_with_label("取消");
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
    g_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(no),dialog);

    gtk_widget_show_all(dialog);

}

void denglu()  //登陆回调函数
{
    gchar* username;
    gchar* pwd;

    //获取输入信息
    username=gtk_entry_get_text(GTK_ENTRY(entry_username));
    pwd=gtk_entry_get_text(GTK_ENTRY(entry_pwd1));

    printf("账号%s\n",username);
    printf("密码%s\n",pwd);
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
    image=gtk_image_new_from_file("11.png");
    gtk_box_pack_start(GTK_BOX(vbox),image,FALSE,FALSE,0);
    gtk_widget_show(image);

    /*生成标签控件*/
    label=gtk_label_new("账号：");
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
    gtk_widget_show(label);

    /*生成编辑框*/
    text=gtk_entry_new_with_max_length(20);
    gtk_box_pack_start(GTK_BOX(vbox),text,FALSE,FALSE,0);
    /*获取输入信息*/
    entry_username=text;

    gtk_widget_show(text);



    /*生成标签控件*/
    label=gtk_label_new("密码：");
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
    gtk_widget_show(label);

    /*生成编辑框*/
    text=gtk_entry_new_with_max_length(20);          
    gtk_box_pack_start(GTK_BOX(vbox),text,FALSE,FALSE,0);
    //设置密码框不可见，用户输入时显示“*”
    gtk_entry_set_visibility(GTK_ENTRY(text),FALSE);
    gtk_entry_set_invisible_char(GTK_ENTRY(text),'*');

    //获取输入信息
    entry_pwd1=text;

    gtk_widget_show(text);



    return vbox;



}


int main(int argc,char** argv)
{
    GtkWidget* window;
    GtkWidget* table;
    GtkWidget* textentry;
    GtkWidget* buttonbox;


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



