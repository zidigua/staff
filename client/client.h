#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct {
	int  id;             //员工的编号
	char name[20];       
	char passwd[20];
	char tips[50];       //用于传送信息内容
	int  type;					 //0位root用户，1为普通用户
	char sex[10];
	int  age;
	char phone[11];
	char addr[50];
}MSG;

//一些全局变量
int socketfd;      								//定义网络套接字
struct sockaddr_in serveraddr;    //定义网络信息结构体
MSG msg;                          //定义信息结构体
int n;														//变量定义
//一些函数的定义
void socket_init();
void do_register(int socketfd, MSG *msg);
int do_login(int socketfd, MSG *msg);
void do_forget_password(int socketfd, MSG *msg);
void do_add_user(int socketfd, MSG * msg);
void do_delete_user(int socketfd, MSG * msg);
void do_update_general_user(int socketfd, MSG * msg);
void do_update_root_user(int socketfd, MSG * msg);

void do_search_general_user(int socketfd, MSG * msg);
void do_search_root_user(int socketfd, MSG * msg);

void do_root_user(int socketfd, MSG *msg);
void do_general_user(int socketfd, MSG *msg);

#endif

