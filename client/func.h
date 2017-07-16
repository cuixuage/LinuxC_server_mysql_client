#include <strings.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <time.h>
#include <dirent.h>
#include <crypt.h>
#include <mysql/mysql.h>

typedef struct{
	int len;
	char buf[1000];
}train,*ptrain;

int send_n(int sfd,char* p,int len);
int recv_n(int sfd,char* p,int len);
void send_data(int new_fd,char* FILENAME);

void judge(ptrain pt,char* buf1,char* buf2);
int recivefile_formserver(int sfd);
void sigfunc(int signalnum);

int judgefile_exit(char *filename);
int recivefile_formserver_again(int sfd,char *filename,int need_seek);

int client_login();
int client_signup();
int judge_login_singup();