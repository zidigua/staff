
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <sqlite3.h>
#include <time.h>
#include <pthread.h>

#define DATABASE "staff.db"
#define N 1024

typedef struct {
	int id; 			//用户id
	char name[20]; 		//姓名
	char passwd[20]; 	//密码
	char tips[50];  	//消息
	int  type; 			//用户类型
	char sex[10]; 		//性别
	int age; 			//年龄
	char phone[11]; 	//电话
	char addr[50]; 		//住址
}MSG;

struct sockaddr_in serveraddr, clientaddr;
sqlite3 *db;
socklen_t client_len;
int listenfd;

/**
 *Name 		  : 	socket_init
 *Description : 	初始化网络，为客户端连接做准备
 *Input 	  : 	
 *Output 	  :
 */
void socket_init(const char* argv[]) 
{
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Fail to socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	client_len = sizeof(struct sockaddr);
	
	//绑定服务器端口
	if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
		fprintf(stderr, "Fail to bind:%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	//服务器开启监听，等待客服段连接
	if(listen(listenfd, 20) < 0) {
		fprintf(stderr, "Fail to listen :%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return;
}

/**
 *Name 		  : 	create_database
 *Description : 	创建数据库
 *Input 	  :
 *Output 	  :
 */
int create_database() 
{
	char **dbResult;
	int nRow, nColumn;
	char *errmsg;
	char sql[1024];
	printf("create_database!\n");

	//打开数据库，如果数据库不存在则创建
	if(sqlite3_open(DATABASE, &db) != SQLITE_OK) {
		printf("%s\n", sqlite3_errmsg(db));
		return 0;
	}

	sprintf(sql, "create table user_login (id integer primary key autoincrement,name vchar(20),password vchar(20),type integer,tips vchar(50));");
	printf("create_database!\n");
	if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
		printf("%s\n", errmsg);
		return 0;
	}

	printf("create_database!\n");
	sprintf(sql, "create table user_info (id integer primary key,name vchar(20),sex vchar(10),age integer,phone vchar(11),addr vchar(50));");
	if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
		printf("%s\n", errmsg);
		return 0;
	}


	sprintf(sql, "select * from user_login where name = 'root';");
	sqlite3_get_table(db, sql, &dbResult, &nRow, &nColumn, &errmsg);
	if (nRow == 0) {
		sprintf(sql, "insert into user_login values(null,'root','root',0, 'root');");
		sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	} 

	return 1;
}

void do_quit(MSG *userMsg, int clientfd) 
{
	free(userMsg);
	close(clientfd);
	printf("user quit\n");

	return;
}


/**
 *Name 		  : 	do_register
 *Description : 	注册用户信息，将用户登录信息添加到user_login表中，将
 					用户信息添加到user_info表中
 *Input 	  :     用户信息（MSG）
 *Output 	  :
 */
void do_register(MSG *userMsg, int clientfd) 
{
	char sql[N] = {};
	char *errmsg;
	char **dbResult;
	int nRow, nColumn, index;
	char buf[N] = {};
	MSG recvMsg;
	int length;

	printf("1111111111\n");
	if ((length = recv(clientfd, userMsg, sizeof(MSG), 0)) < 0) {
	}
	if (strcmp(userMsg->name, "root") == 0) {
		strcpy(recvMsg.tips, "#permission denied!"); 
	} else {
		sprintf(sql, "insert into user_login values(null,'%s','%s',%d,'%s');", userMsg->name, userMsg->passwd, userMsg->type, userMsg->tips);

		recvMsg.tips[0] = '\0';
		if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
			fprintf(stderr, "%s\n", errmsg);
			recvMsg.tips[0] = '#'; 
		}

		//从user_login表中查询id
		sprintf(sql, "select id from user_login where  name = '%s' and tips = '%s';",userMsg->name, userMsg->tips);
		if (sqlite3_get_table(db, sql, &dbResult, &nRow, &nColumn, &errmsg) != SQLITE_OK) {
			fprintf(stderr, "%s\n", errmsg);
		} else {
			if (nRow == 0) {
				recvMsg.tips[0] = '#';
			} else {
				bzero(buf, sizeof(buf)); 	//清空buf
				index = nColumn;
				strcpy(buf, dbResult[index]);
				printf("%s\n", buf);
				sprintf(sql, "insert into user_info values(%s,'%s','%s',%d,'%s','%s');",buf, userMsg->name, userMsg->sex, userMsg->age, userMsg->phone, userMsg->addr);
				printf("%s\n", sql);

				//根据查询到的id将用户信息添加到user_info表中
				if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
					fprintf(stderr, "%s\n", errmsg);
					recvMsg.tips[0] = '#';
				}
				strcpy(recvMsg.tips, "Register Success!\n");
			}
		}
	}
	if (send(clientfd, &recvMsg, sizeof(recvMsg), 0) < 0) {
		fprintf(stderr, "Fail to send :%s\n", strerror(errno));
	}

	return;
}


void do_add(MSG *userMsg, int clientfd)
{
	do_register(userMsg, clientfd);
}

/**
 *Name 		  : 	do_delete
 *Description :     根据用户的id删除用户记录 
 *Input 	  : 	用户id(msg->id)
 *Output 	  :
 */
void do_delete(MSG *userMsg, int clientfd) 
{
	int length;
	char sql[N] = {};
	char *errmsg;
	char **dbResult;
	int nRow, nColumn;
	MSG recvMsg;

	if ((length = recv(clientfd, userMsg, sizeof(MSG), 0)) < 0) {
	}

	sprintf(sql, "select * from user_info where id = %d;", userMsg->id);
	printf("%s\n", sql);

	//判断该用户是否存在
	if (sqlite3_get_table(db, sql, &dbResult, &nRow, &nColumn, &errmsg) != SQLITE_OK) {
		fprintf(stderr, "%s\n", errmsg);
	} else {
		printf("nRow = %d\n", nRow);
		if (nRow < 1) {
			strcpy(recvMsg.tips, "#The user not exist");
			if (send(clientfd, &recvMsg, sizeof(recvMsg), 0) < 0) {
				fprintf(stderr, "Fail to send :%s\n", strerror(errno));
			}
			return;
		}

		//从user_login表中删除该用户
		sprintf(sql, "delete from user_login where id = %d;", userMsg->id);
		printf("%s\n", sql);
		if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
			printf("%s\n", errmsg);
			return;
		}

		//从user_info表中删除用户
		sprintf(sql, "delete from user_info where id = %d;", userMsg->id);
		if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
			printf("%s\n", errmsg);
			return;
		}

		strcpy(recvMsg.tips, "delete success");
		if (send(clientfd, &recvMsg, sizeof(recvMsg), 0) < 0) {
			fprintf(stderr, "Fail to send :%s\n", strerror(errno));
		}
	}

	return;
}

/**
 *Name 		  : 	do_select 
 *Description : 	根据用户id查询某个用户的信息
 *Input 	  : 	用户id
 *Output 	  :
 */
void do_select(MSG *userMsg, int clientfd) 
{
	int length;
	char sql[N] = {};
	char *errmsg;
	char **dbResult;
	int nRow,nColumn;
	MSG recvMsg;

	if ((length = recv(clientfd, userMsg, sizeof(MSG), 0)) < 0) {
	}

	sprintf(sql, "select * from user_info where id = %d;", userMsg->id);
	if (sqlite3_get_table(db, sql, &dbResult, &nRow, &nColumn, &errmsg) != SQLITE_OK) {
		fprintf(stderr, "%s\n", errmsg);
	} else {
		if (nRow == 0) {
			strcpy(recvMsg.tips, "#The user not exist");
		} else {
			//将查询所得结果进行封装到MSG
			recvMsg.id = userMsg->id;
			strcpy(recvMsg.name, dbResult[7]);
			strcpy(recvMsg.sex, dbResult[8]);
			recvMsg.age = atoi(dbResult[9]);
			strcpy(recvMsg.phone, dbResult[10]);
			strcpy(recvMsg.addr, dbResult[11]);
			strcpy(recvMsg.tips, "query success");
		}
		if (send(clientfd, &recvMsg, sizeof(recvMsg), 0) < 0) {
			fprintf(stderr, "Fail to send :%s\n", strerror(errno));
		}
	}

	return;
}

/**
 *Name 		  : 	do_updata 
 *Description : 	管理员根据用户id修改用户信息
 *Input 	  : 	用户id
 *Output 	  :
 */
void do_updata(MSG *userMsg, int clientfd) 
{
	char sql[1024] = {};
	char **pazResult;
	int pnRow;
	int pnColumn;
	char *pzErrmsg;

	if (recv(clientfd, userMsg, sizeof(*userMsg), 0) < 0) {
	}

	//根据id查询该用户是否存在
	sprintf(sql, "select * from user_info where id = %d;", userMsg->id);
	sqlite3_get_table(db, sql, &pazResult, &pnRow, &pnColumn, &pzErrmsg);
	if (pnRow >= 1) {
		//修改用户的电话和地址
		sqlite3_free_table(pazResult);
		sprintf(sql, "update user_info set phone = '%s',addr = '%s' where id = %d;", userMsg->phone, userMsg->addr, userMsg->id);
		sqlite3_get_table(db, sql, &pazResult, &pnRow, &pnColumn, &pzErrmsg);
		strcpy(userMsg->tips, "update success!");
		send(clientfd, userMsg, sizeof(*userMsg), 0);
	} else {
		sqlite3_free_table(pazResult);
		strcpy(userMsg->tips, "#id not exist!");
		send(clientfd, userMsg, sizeof(*userMsg), 0);
	}

	return;
}

void do_passwd(MSG *userMsg, int clientfd) 
{
	char sql[1024] = {};
	char **pazResult;
	int pnRow;
	int pnColumn;
	char *pzErrmsg;
	MSG recvMsg;

	if (recv(clientfd, userMsg, sizeof(*userMsg), 0) < 0) {
	}

	//根据用户id修改用户密码
	sprintf(sql, "update user_login set password = '%s' where id = %d;", userMsg->passwd, userMsg->id);
	sqlite3_get_table(db, sql, &pazResult, &pnRow, &pnColumn, &pzErrmsg);
	if (pnRow >= 1) {
		sqlite3_free_table(pazResult);
		strcpy(recvMsg.tips, "success!");
		send(clientfd, userMsg, sizeof(*userMsg), 0);
	} else {
		sqlite3_free_table(pazResult);
		strcpy(recvMsg.tips, "success!");
		send(clientfd, &recvMsg, sizeof(recvMsg), 0);
	}

	return;
}



void do_login_success(MSG *userMsg, int clientfd)
{
	while(1) {
		if (recv(clientfd, userMsg, sizeof(*userMsg), 0) < 0) {
		}

		switch(userMsg->tips[0]) {
			case 'A':
				do_add(userMsg, clientfd);
				break;
			case 'D':
				do_delete(userMsg, clientfd);
				break;
			case 'U':
				do_updata(userMsg, clientfd);
				break;
			case 'S':
				do_select(userMsg, clientfd);
				break;
			case 'P':
				do_passwd(userMsg, clientfd);
				return;
				break;
			case 'Q':
				return;
				break;
			default:
				break;
		}
	}

	return;
}



/**
 *Name 		  : 	do_login
 *Description : 	判断用户输入的帐号与密码是否正确
 *Input 	  : 	用户id,密码
 *Output 	  :
 */
void do_login(MSG *userMsg, int clientfd) 
{
	char sql[1024] = {};
	char **pazResult;
	int pnRow;
	int pnColumn;
	char *pzErrmsg;

	if (recv(clientfd, userMsg, sizeof(*userMsg), 0) < 0) {
	}

	sprintf(sql, "select * from user_login where name = '%s' and password = '%s'", userMsg->name, userMsg->passwd);
	printf("%s\n", sql);
	sqlite3_get_table(db, sql, &pazResult, &pnRow, &pnColumn, &pzErrmsg);

	if ((pnRow >= 1) && (strcmp(pazResult[6], userMsg->name) == 0) && (strcmp(pazResult[7], userMsg->passwd) == 0)) {
		sqlite3_free_table(pazResult);
		strcpy(userMsg->tips, "login success!");
		printf("%s\n", pazResult[8]);

		if (strcmp(pazResult[8], "1") == 0)
			userMsg->type = 1;
		else 
			userMsg->type = 0;

		userMsg->id = atoi(pazResult[5]);
		send(clientfd, userMsg, sizeof(*userMsg), 0);
		do_login_success(userMsg, clientfd);
	} else {
		sqlite3_free_table(pazResult);
		strcpy(userMsg->tips, "#login Fail!");
		send(clientfd, userMsg, sizeof(*userMsg), 0);
	}

	return;
}

/**
 *Name 		  : 	do_findPassword 
 *Description : 	在用户忘记密码时提供密保找回密码功能
 *Input 	  : 	用户id,密保问题
 *Output 	  :
 */
void do_findPassword(MSG *userMsg, int clientfd) 
{
	char sql[1024] = {};
	char **pazResult;
	int pnRow;
	int pnColumn;
	char *pzErrmsg;

	if (recv(clientfd, userMsg, sizeof(*userMsg), 0) < 0) {
	}

	sprintf(sql, "select * from user_login where name = '%s' and tips = '%s'", userMsg->name, userMsg->tips);
	printf("%s\n", sql);
	sqlite3_get_table(db, sql, &pazResult, &pnRow, &pnColumn, &pzErrmsg);
	if ((pnRow >= 1) && (strcmp(pazResult[6], userMsg->name) == 0) && (strcmp(pazResult[9], userMsg->tips) == 0)) {
		sqlite3_free_table(pazResult);
		strcpy(userMsg->tips, "Success");
		strcpy(userMsg->passwd, pazResult[7]);
		send(clientfd, userMsg, sizeof(*userMsg), 0);
	} else {
		sqlite3_free_table(pazResult);
		strcpy(userMsg->tips, "#name or tips wrong!");
		send(clientfd, userMsg, sizeof(*userMsg), 0);
	}

	return;
}

void * do_client(void *arg) 
{
	int length;
	int clientfd = *(int *)arg;
	MSG *userMsg = (MSG *)malloc(sizeof(MSG));

	while(1) {
		if ((length = recv(clientfd, userMsg, sizeof(*userMsg), 0)) < 0) {
			fprintf(stderr, "%s\n", strerror(errno));
		}

		printf("%c\n", userMsg->tips[0]);
		switch(userMsg->tips[0]) {
			case 'R':
				do_register(userMsg, clientfd);
				break;
			case 'F':
				do_findPassword(userMsg, clientfd);
				break;
			case 'L':
				do_login(userMsg, clientfd);
				break;
			case 'Q':
				do_quit(userMsg, clientfd);
				return NULL;
				break;
			default:
				printf("Server Error \n");
				break;
		}
		if(length == 0) {
			break;
		}
	}
	
	return NULL;
}

int main(int argc, const char *argv[])
{
	pthread_t tid; 		//线程id

	if(argc < 3) {
		fprintf(stderr,"Usage :%s <server_ip> <server_port>\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	create_database(); //创建数据库
	socket_init(argv);
	while(1) {
		int clientfd;

		//连接客服端
		if ((clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &client_len)) < 0) {
			fprintf(stderr, "Fail to accept:%s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (pthread_create(&tid, NULL, do_client, (void *)&clientfd)) {
			fprintf(stderr, "Fail to pthread_create :%s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	close(listenfd);

	return 0;
}

