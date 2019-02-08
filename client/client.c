
#include "client.h"
	
	
//主函数入口
int main(int argc, const char *argv[])
{

	//输入参数个数判断 格式：client <serv_ip> <serv_port>
	if(argc < 3){
		printf("Usage : %s <serv_ip> <serv_port>\n", argv[0]);	
		exit(-1);
	}

	//网络套接字初始化
	socket_init(argv);
	
		while(1){
		//初始化界面
		printf("*****************************************************\n");
		printf("* 1: register  2: login  3: forget password  4: exit*\n");
		printf("*****************************************************\n");
		printf("please choose : ");

		//输入错误向服务器发送退出信息，并关闭套接字退出程序
		scanf("%d", &n);
		getchar();

		
		//根据n值进入不同的函数
		switch(n){
			
		case 1:
			//send R  注册
			msg.tips[0] = 'R';
			send(socketfd, &msg, sizeof(MSG),0);
			do_register(socketfd, &msg);
			break;
			//send L  登录
		case 2:
			msg.tips[0] = 'L';
			send(socketfd, &msg, sizeof(MSG),0);	
			do_login(socketfd, &msg);
			break;
			//send F  找回密码
		case 3:
			msg.tips[0] = 'F';
			send(socketfd, &msg,sizeof(MSG), 0);
			do_forget_password(socketfd, &msg);
			break;
			//send Q  退出
		case 4:		
			msg.tips[0] = 'Q';
			send(socketfd, &msg,sizeof(MSG), 0);
			close(socketfd);
			exit(0);
		default:
			printf("input error!\n");
			break;
		}
	}

	return 0;
}

