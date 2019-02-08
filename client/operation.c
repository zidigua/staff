/*************************************************************************
	> File Name:   client.c   客户端代码
	> Description: 客户端代码，客户端功能代码函数的实现
	> Author:      陈阿磊、马建业、黄文康
	> Mail:        @@
	> Version:      v3
	> Modification: ##
	> Created Time: Mon 10 Oct 2016 08:39:30 PM PDT
 ************************************************************************/

#include "client.h"

//网络套接字初始化
void socket_init(const char *argv[])
{
	if((socketfd = socket(AF_INET, SOCK_STREAM, 0 )) < 0){
		perror("fail to socket");	
		exit(-1);
	}
	bzero(&serveraddr, sizeof(serveraddr));

	//网络信息结构体填充
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	serveraddr.sin_port = htons(atoi(argv[2]));

	if(connect(socketfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr) ) < 0){
		perror("fail to connect");	
		close(socketfd);
		exit(-1);
	}
}

/**
 * Name         :  do_register 
 * Description  :  用户进行注册，填充信息后发送给服务系接收，阻塞等待
 									 接收一些提示信息。
 * Input ：   
 *			           socketfd： 通信套接字
 *			           msg： 接受到的信息结构体指针
 * output       :
 * Author       :  陈阿磊
 * Other        :  返回的提示信息中 msg->tips[0]为'#'，表示失败
 */
 
void do_register(int socketfd, MSG *msg){
	
	msg->type = 1;//由于注册的只能是普通用户，所以这里直接把类型
								//定义为 '1'（普通用户）		
	//输入用户信息			
	printf("input your name:");
	scanf("%s",msg->name);
	getchar();
	printf("input your password:");
	scanf("%s",msg->passwd);
	getchar();
	printf("input remark message:");
	scanf("%s",msg->tips);
	getchar();
	printf("input your sex:");
	scanf("%s",msg->sex);
	getchar();
	printf("input your age:");
	scanf("%d",&msg->age);
	getchar();
	printf("input your phone:");
	scanf("%s",msg->phone);
	getchar();
	printf("input your addr:");
	scanf("%s",msg->addr);
	getchar();              //吃掉垃圾字符

	send(socketfd, msg, sizeof(MSG),0);//发送消息结构体
	recv(socketfd, msg, sizeof(MSG),0);//阻塞等待接收一些提示信息，
	
	//无论成功与否，打印查询结果。
	if(msg->tips[0] == '#'){
		printf("%s\n", msg->tips+1);
	}
	else
		printf("%s\n", msg->tips);
		
	return;
}


/**
 * Name         :  do_login 
 * Description  :  登录功能，通过用户名和密码进行登录，
 *								 登录成功的同时会根据不同的用户类型接入不同的入口。
 * Input    
 *			           socketfd： 通信套接字
 *			           msg： 接受到的信息结构体指针
 * output       :
 * Author       :  马建业
 * Other        :  返回的提示信息中 msg->tips[0]为'#'，表示失败

 */
int do_login(int socketfd, MSG *msg){

	printf("input your name:");
	scanf("%s",msg->name);
	getchar();              //吃掉垃圾字符
	printf("input your password:");
	scanf("%s",msg->passwd);
	getchar();              //吃掉垃圾字符

	send(socketfd,msg,sizeof(MSG),0);
	//如果登录成功，此时应该会传回用户的 id 号，以及 用户类型 type 
	recv(socketfd,msg,sizeof(MSG),0);

  //无论成功与否，打印查询结果。
	if(msg->tips[0] == '#'){
		printf("register : %s\n", msg->tips + 1);
		return 0;
	}
	else
	{
		printf("%s\n",msg->tips);
		if (msg->type == 1) {
			do_general_user(socketfd, msg);  //登录为普通用户
		}
		else {
			do_root_user(socketfd, msg);  	 //登录为root用户
		}
	}	
	
	return 1;
}


/**
 * Name         :  do_forget_password 
 * Description  :  找回密码功能，忘记密码时通过姓名和注册时填写的备注信息
 *								 来找回密码
 * Input    
 *			           socketfd： 通信套接字
 *			           msg： 接受到的信息结构体指针
 * output       :
 * Author       :  黄文康
 * Other        :  返回的提示信息中 msg->tips[0]为'#'，表示失败，否则正确
 *								 打印出密码。

 */

void do_forget_password(int socketfd, MSG *msg){
	printf("input your name:");
	scanf("%s",msg->name);
	getchar();              //吃掉垃圾字符
	printf("input remark message:");
	scanf("%s",msg->tips);
	getchar();              //吃掉垃圾字符
	
	send(socketfd,msg,sizeof(MSG),0);
	recv(socketfd,msg,sizeof(MSG),0);
	//无论成功与否都会打印提示信息，成功找到后同时会打印密码，实现密码找回功能
	if(msg->tips[0] == '#'){
		printf("%s\n",msg->tips+1);
	}
	else{
		printf("%s\n", msg->tips);
		printf("password : %s\n",msg->passwd);
	}
	return;

}


/**以下实现 增删改查 等功能 */


//添加用户：超级用户执行的添加用户功能实际上与注册永和的功能相同，所以这里直接调用
void do_add_user(int socketfd, MSG * msg){
	do_register(socketfd,msg);
	return;
}

//删除用户：删除用户是通过输入 id 号进行删除
void do_delete_user(int socketfd, MSG * msg){
	printf("input id:");
	scanf("%d",&msg->id);
	getchar();              //吃掉垃圾字符

	send(socketfd,msg,sizeof(MSG),0);
	recv(socketfd,msg,sizeof(MSG),0);

	if(msg->tips[0] == '#'){
		printf("%s\n",msg->tips+1);
	}
	else
		printf("%s\n", msg->tips);
	return;
}

//root用户修改信息：通过 id号查找用户，找到之后可以更改电话号码，地址信息								
void do_update_root_user(int socketfd, MSG * msg){

		printf("input user id:");
		scanf("%d",&msg->id);
		getchar();              //吃掉垃圾字符
		
		printf("input user new phone number:");
		scanf("%s",msg->phone);
		getchar();              //吃掉垃圾字符
		printf("input user new address:");
		scanf("%s",msg->addr);
		getchar();              //吃掉垃圾字符
		
		send(socketfd,msg,sizeof(MSG),0);
		recv(socketfd,msg,sizeof(MSG),0);
		if(msg->tips[0] == '#'){
			printf("%s\n",msg->tips+1);
		}
		else
			printf("%s\n", msg->tips);
		return;
}


//普通用户修改信息：如果是普通用户则可以更改自己的密码
void do_update_general_user(int socketfd, MSG * msg){
		
		printf("input your new password:");
		scanf("%s",msg->passwd);
		getchar();              //吃掉垃圾字符

		send(socketfd,msg,sizeof(MSG),0);
		recv(socketfd,msg,sizeof(MSG),0);
	
	if(msg->tips[0] == '#'){
		printf("%s\n",msg->tips+1);
	}
	else
		printf("%s\n", msg->tips);
	return;
}

//root用户查询信息：通过 id号查看不同的用户用户
void do_search_root_user(int socketfd, MSG * msg){
	printf("input id:");
	scanf("%d",&msg->id);
	getchar();              //吃掉垃圾字符

	send(socketfd,msg,sizeof(MSG),0);
	recv(socketfd,msg,sizeof(MSG),0);

	if(msg->tips[0] == '#'){
		printf("%s\n",msg->tips+1);
	}
	//
	//打印查询结果：
	else{
		printf("id : %d\n",msg->id);	
		printf("name : %s\n", msg->name);
		printf("sex : %s\n", msg->sex);
		printf("age : %d\n", msg->age);
		printf("phone : %s\n", msg->phone);
		printf("address : %s\n", msg->addr);
	}
	return;
}

//普通用户查询信息：普通用户只能查看自己的信息
void do_search_general_user(int socketfd, MSG * msg){
	
	send(socketfd,msg,sizeof(MSG),0);
	recv(socketfd,msg,sizeof(MSG),0);

	if(msg->tips[0] == '#'){
		printf("%s\n",msg->tips+1);
	}
	//
	//打印查询结果：
	else{
		printf("id : %d\n",msg->id);	
		printf("name : %s\n", msg->name);
		printf("sex : %s\n", msg->sex);
		printf("age : %d\n", msg->age);
		printf("phone : %s\n", msg->phone);
		printf("address : %s\n", msg->addr);
	}
	return;
}

//root用户：在登录时type为0，进入root用户界面
void do_root_user(int socketfd, MSG *msg)
{
		int n;
				
		while(1){
			printf("*****************************************************\n");
			printf("* 1: add  2: delete  3: update   4: search  5: exit *\n");
			printf("*****************************************************\n");
			printf("please choose : ");
			
			scanf("%d", &n);
			getchar();
			switch(n){
			case 1:
				msg->tips[0] = 'A';
				send(socketfd,msg,sizeof(MSG),0);
				do_add_user(socketfd,msg);   	 //添加用户
				break;
				
			case 2:				
				msg->tips[0] = 'D';
				send(socketfd,msg,sizeof(MSG),0);
				do_delete_user(socketfd,msg);  //删除用户
				break;
				
			case 3:			
				msg->tips[0] = 'U';
				send(socketfd,msg,sizeof(MSG),0);
				do_update_root_user(socketfd,msg); //root用户修改信息
				break;
				
			case 4:				
				msg->tips[0] = 'S';
				send(socketfd,msg,sizeof(MSG),0);
				do_search_root_user(socketfd,msg); //root用户查询信息
				break;
				
			case 5:			
				msg->tips[0] = 'Q';                //退出
				send(socketfd,msg,sizeof(MSG),0);
				return;
			default:
				printf("input error!\n");
				break;
			}
		}
}


//普通用户：在登录时type为1，进入普通用户界面
void do_general_user(int socketfd, MSG *msg){
		int n;
		
		while(1){
		
			printf("********************************\n");
			printf("* 1: update  2: search  3: exit*\n");
			printf("********************************\n");
			printf("please choose : ");

			scanf("%d", &n);
			getchar();
			
			switch(n){
			case 1:
				msg->tips[0] = 'P';
				send(socketfd,msg,sizeof(MSG),0);
				do_update_general_user(socketfd,msg);  //普通用户修改信息
				return;
				break;
				
			case 2:
				msg->tips[0] = 'S';
				send(socketfd,msg,sizeof(MSG),0);
				do_search_general_user(socketfd,msg);  //普通用户查询信息
				break;
				
			case 3:
				msg->tips[0] = 'Q';
				send(socketfd,msg,sizeof(MSG),0);      //退出
				return;
			default:
				printf("input error!\n");
				break;
			}
		}
}


