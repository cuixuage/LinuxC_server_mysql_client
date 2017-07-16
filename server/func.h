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
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>  
#include <sys/mman.h> 
#include <crypt.h>
#include <mysql/mysql.h>
#define LogFile "/home/cuixuange/20170714/server/server.log"

typedef struct{
	pid_t pid;//子进程的pid
	int tfds;//通过该管道传递内核控制信息
	short busy;//标示进程是否忙碌
}child,*pchild;
typedef struct{
	int len;
	char buf[1000];
}train,*ptrain;

int send_n(int sfd,char* p,int len);
int recv_n(int sfd,char* p,int len);
void send_data(int new_fd,char* FILENAME);
void make_child(pchild p,int num);
void child_handle(int fdr);
void send_fd(int fdw,int fd);
void recv_fd(int fdr,int* fd);

void judge_handle(int command,int new_fd);
char*  printdate(char *result,char *old);
void printmode(mode_t st_mode);
void ls_l();
int recivefile_formclient(int sfd);

void insert_loginlog();
void insert_commandlog(int command,char* buf);
void send_data_again(int new_fd,char* FILENAME,int need_seek);
long judge_filesize(char *filename);
void send_mmpdata(int new_fd,char* FILENAME,void* src,long filesize,long download_file_size);

int judge_clientlogin(int new_fd);
int judge_clientSignup(int new_fd);
void query_encrypted_password(char *login_username,char *encrypted_password);
void query_salt(char *login_username,char *salt);
void insert_data(char* login_username,char* login_salt,char* login_password);