#include "func.h"
//最新的时间
char*  printdate(char *result,char *old){
		int e=snprintf(result,17*sizeof(char),"%s",old);
		//printf("get: %s\n",result);
		return result+4;
}
//最新的文件权限
void printmode(mode_t st_mode){
		if(S_ISDIR(st_mode)) printf("d");
		else if (S_ISREG(st_mode)) printf("-");
		else if (S_ISCHR(st_mode)) printf("c");
		else if (S_ISBLK(st_mode)) printf("b"); 
		else if (S_ISFIFO(st_mode)) printf("p"); 
		else if (S_ISLNK(st_mode)) printf("l"); 
		if(st_mode&S_IRUSR) printf("r");else printf("-");
		if(st_mode&S_IWUSR) printf("w");else printf("-"); 
		if(st_mode&S_IRUSR) printf("x");else printf("-"); 
		if(st_mode&S_IXUSR) printf("r");else printf("-"); 
		if(st_mode&S_IRGRP) printf("w");else printf("-"); 
		if(st_mode&S_IWGRP) printf("x");else printf("-"); 
		if(st_mode&S_IROTH) printf("r");else printf("-"); 
		if(st_mode&S_IWOTH) printf("w");else printf("-"); 
		if(st_mode&S_IXOTH) printf("x");else printf("-"); 
}
void ls_l()
{
		DIR  *dir;
		dir=opendir(getcwd(NULL,0));
		struct dirent *p;
		struct stat buf;
		int ret;
		char path[512];
		char result[100];  //保存更改后的时间
		while((p=readdir(dir))!=NULL)	{
				if(strcmp(p->d_name,".")&&strcmp(p->d_name,".."))	{
						memset(&buf,0,sizeof(buf));
						memset(path,0,sizeof(path));
						memset(result,0,sizeof(result));
						//p是dirent结构指针，&buf是stat结构指针
						//拼接目录下文件的路径
						sprintf(path,"%s%s%s",getcwd(NULL,0),"/",p->d_name);
						ret=stat(path,&buf);
						char *time=printdate(result,ctime(&buf.st_mtime));
						printmode(buf.st_mode);
						printf("  %d %s %s %5ld %s %s\n",buf.st_nlink,getpwuid(buf.st_uid)->pw_name,getgrgid(buf.st_gid)->gr_name,buf.st_size,time,p->d_name);
				}	
		}
		closedir(dir);
		return;
}
int recivefile_formclient(int sfd){
    int len;
	char buf[1000]={0};
	recv_n(sfd,(char *)&len,sizeof(len));
	recv_n(sfd,buf,len);     //第一次传递文件名称,保存到buf中
	
	insert_commandlog(3,buf);     //记录日志
	
	int fd=open(buf,O_CREAT|O_WRONLY|O_TRUNC,0666);   //将server 文件保存到client端
	if(-1==fd)
	{
		perror("open");
		return -1;
	}
    //第二次接受文件长度的数据包
    recv_n(sfd,(char*)&len,sizeof(len));
    long big;
    recv_n(sfd,(char*)&big,sizeof(len));
    printf("upload file name=%s big=%ld\n",buf,big);

    long check=big/10;
    long l=0;   //保存文件的长度 bytes
	while(1)
	{
		recv_n(sfd,(char *)&len,sizeof(int));//接火车头
        l=l+len;
		if(len>0)   //只有最后一次结束时发送len=0 的数据包
		{
			memset(buf,0,sizeof(buf));
			recv_n(sfd,buf,len);    
			write(fd,buf,len);

            if(l>=check){
              printf("Now Data %8.4f%s\r",(double)l*100/big,"%");
              fflush(stdout);
              check +=big/10;
            }
		}else{
            printf("Now Data %8.4f%s\n",(double)l*100/big,"%");
            printf("load down ok\n");
			break;
		}
	}
	close(fd);
	return 0;
}

long judge_filesize(char *filename){
		struct stat statbuf;
		memset(&statbuf,0,sizeof(statbuf));
	    stat(filename,&statbuf);
		return statbuf.st_size>1e8?statbuf.st_size:-1;    //大于100M返回文件大小   否则返回-1
}

void insert_commandlog(int command,char* buf){     //记录命令信息
    int logfd=open(LogFile,O_CREAT|O_WRONLY|O_APPEND,0666);    //O_APPEND 以追加形式打开文件
	char buffer[100]={0};
	char cmd[10]={0};
	if(command==1) strcpy(cmd,"cd");
	else if(command==2) strcpy(cmd,"ls");
	else if(command==3) strcpy(cmd,"gets");
	else if(command==4) strcpy(cmd,"puts");
	else if(command==5) strcpy(cmd,"remove");
	else if(command==6) strcpy(cmd,"pwd");
	sprintf(buffer,"[command] %s %s\n",cmd,buf);
	//printf("insert_commandlog: %s",buffer);
	write(logfd,buffer,strlen(buffer)*sizeof(char));
	close(logfd);
}
	
void judge_handle(int command,int new_fd){
	int contentlength;
	char buf[100]={0};
	if(command==1){
		memset(buf,0,sizeof(buf));
		recv_n(new_fd,(char*)&contentlength,sizeof(int));
		recv_n(new_fd,(char*)buf,contentlength);
		chdir(buf);   // cd 目录位置
		//printf("working directory: %s\n",getcwd(NULL,0));
		insert_commandlog(command,buf);  //记录日志
	}
	if(command==2){
		memset(buf,0,sizeof(buf));
		ls_l();
		insert_commandlog(command,buf);
	}
	if(command==3){
		memset(buf,0,sizeof(buf));
		recivefile_formclient(new_fd);    //获取客户端上传文件
	}
	if(command==4){
		memset(buf,0,sizeof(buf));
		recv_n(new_fd,(char*)&contentlength,sizeof(int));   
		recv_n(new_fd,(char*)buf,contentlength);  //获取下载文件名称
		printf("download filename = %s\n",buf);
		//send_data(new_fd,buf);
		long filesize=judge_filesize(buf);
		if(filesize<0)  send_data(new_fd,buf);
		else if(filesize>0){                      //文件大于100M
			int src_fd=open(buf,O_RDONLY);
			void* src_ptr = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, src_fd, 0); 
			send_mmpdata(new_fd,buf,src_ptr,filesize,0);
		}	
		insert_commandlog(command,buf);
	}
	if(command==44){
		memset(buf,0,sizeof(buf));
		recv_n(new_fd,(char*)&contentlength,sizeof(int));   
		recv_n(new_fd,(char*)buf,contentlength);  //获取下载文件名称	
		recv_n(new_fd,(char*)&contentlength,sizeof(int));   //获取客户端已下载的文件长度
		printf("断点下载%s文件已有大小 %d\n",buf,contentlength);
		//send_data_again(new_fd,buf,contentlength);
		long filesize=judge_filesize(buf);
		if(filesize-contentlength<1e8)  send_data_again(new_fd,buf,contentlength);
		else if(filesize-contentlength>1e8){                      
			int src_fd=open(buf,O_RDONLY);
			//lseek(src_fd,contentlength,SEEK_SET);   //移动读写指针     对于mmap无效
			//printf("lseek ok %d\n",contentlength);
			//void* src_ptr = mmap(NULL, filesize-contentlength, PROT_READ, MAP_PRIVATE, src_fd, 0);  //mmap映射已经偏移后的src_fd(无效)  或者最后一个参数即为偏移量被映射对象内容的起点
			void* src_ptr = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, src_fd, 0); 
			send_mmpdata(new_fd,buf,src_ptr,filesize,contentlength);   //mmap继续下载剩余内容
		}	
		insert_commandlog(command,buf);	
	}
	if(command==5){
		memset(buf,0,sizeof(buf));
		recv_n(new_fd,(char*)&contentlength,sizeof(int));   
		recv_n(new_fd,(char*)buf,contentlength);  //获取删除文件名称
		printf("delete filename = %s\n",buf);
		int z=remove(buf);
		if(z==0) printf("delete %s ok\n",buf);
		insert_commandlog(command,buf);
	}
	if(command==6){
		memset(buf,0,sizeof(buf));
        printf("working directory: %s\n",getcwd(NULL,0));
		insert_commandlog(command,buf);
	}
}


void child_handle(int fdr)
{
	char flag=1;
	int new_fd;
	train t;
	int command;
	while(1)
	{
		recv_fd(fdr,&new_fd);//从父进程接收任务     //连接只需要接受一次,多次时fdr值已为空
		while(1){
		    recv_n(new_fd,(char*)&command,sizeof(int));    //不断接受客户端命令
		    
		    if(command==0){                               //客户端按下ctrl+c 传递给server 0号命令
				write(fdr,&flag,sizeof(flag));
				close(new_fd);
				break;
			}    
	
		    judge_handle(command,new_fd);
			write(fdr,&flag,sizeof(flag));//通知父进程，完成任务
		}
	}
}

void make_child(pchild p,int num)
{
	int i;
	int fds[2];
	pid_t pid;
	for(i=0;i<num;i++)//创建多个子进程
	{
		socketpair(AF_LOCAL,SOCK_STREAM,0,fds);
		pid=fork();
		if(0==pid)
		{
			close(fds[1]);
			child_handle(fds[0]);
		}//让子进程永远不能从这个括号走出来
		close(fds[0]);
		p[i].pid=pid;//子进程的pid
		p[i].tfds=fds[1];//拿到管道的一端
		p[i].busy=0;//子进程处于空闲状态
	}
}

