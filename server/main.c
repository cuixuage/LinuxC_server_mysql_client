#include "func.h"
	
int main(int argc,char* argv[])
{
	if(argc!=2)
	{
		printf("Please INPUT IP PORT PROCESS_NUM\n");
		return -1;
	}
	
	int child_num=0;
	//pchild p=(pchild)calloc(child_num,sizeof(child));
	//make_child(p,child_num);//创建子进程并初始化所有的数据结构
	
	int sfd;
	sfd=socket(AF_INET,SOCK_STREAM,0);
	if(-1==sfd)
	{
		perror("socket");
		return -1;
	}
	struct sockaddr_in ser;
	memset(&ser,0,sizeof(ser));
	ser.sin_family=AF_INET;
	//ser.sin_port=htons(atoi(argv[2]));
	//ser.sin_addr.s_addr=inet_addr(argv[1]);
	char conf[100]={0};
	char conf2[100]={0};    //保存conf切割后的数据
	int conf_fd=open(argv[1],O_RDONLY);
	read(conf_fd,conf,sizeof(conf));
	strcpy(conf2,conf);
	//printf("%s\n",conf);
	int i=0;
	for(i=0;i<strlen(conf);i++){
		if(conf[i]=='\n'){
			strcpy(conf,&conf[i+1]);    //得到第一个换行符以后的数据
			conf2[i]='\0';
		    for(i=0;i<strlen(conf2);i++){
				//第一个换行符前的数据再次截取
			    if(conf2[i]=='=') {ser.sin_addr.s_addr=inet_addr(&conf2[i+1]);break;}   //conf[i]是i后面所有字符的字符串
		    }
			printf("IP  %s\n",&conf2[i+1]);
			break;
		}
	}
	//printf("剩余 %s\n",conf);
	for(i=0;i<strlen(conf);i++){
		if(conf[i]=='\n'){
			strcpy(conf2,conf);
			strcpy(conf,&conf[i+1]);    //得到第二个换行符以后数据
			conf2[i]='\0';
		    for(i=0;i<strlen(conf2);i++){
			    if(conf2[i]=='=') {ser.sin_port=htons(atoi(&conf2[i+1]));break;}
		    }
			printf("PORT  %s\n",&conf2[i+1]);
			break;
		}
	}
	//printf("剩余 %s\n",conf);
	for(i=0;i<strlen(conf);i++){
	    if(conf[i]=='=') {
			child_num=atoi(&conf[i+1]);
		    printf("NUM  %s\n",&conf[i+1]);
			break;
		}
	}

	
	pchild p=(pchild)calloc(child_num,sizeof(child));
	make_child(p,child_num);//创建子进程并初始化所有的数据结构
	
	int ret;
	ret=bind(sfd,(struct sockaddr*)&ser,sizeof(ser));
	if(-1==ret)
	{
		perror("bind");
		return -1;
	}
	int epfd=epoll_create(1);
	struct epoll_event event,*evs;
	evs=(struct epoll_event*)calloc(child_num+1,sizeof(struct epoll_event));   // sfd+server_fds  两个额外需要监听的
	memset(&event,0,sizeof(event));
	event.events=EPOLLIN;
	event.data.fd=sfd;
	epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&event);   //监控sfd
	
	//int i;
	for(i=0;i<child_num;i++)//父进程监控每一个子进程管道的对端的可读事件
	{
		event.events=EPOLLIN;
		event.data.fd=p[i].tfds;
		epoll_ctl(epfd,EPOLL_CTL_ADD,p[i].tfds,&event);
	}
	listen(sfd,child_num+1);//端口就打开了
	int ret1;
	int new_fd;
	int j;
	char flag;
	while(1)
	{
		ret1=epoll_wait(epfd,evs,child_num+1,-1);   //返回处理事件的数目,evs亦为传出参数
		for(i=0;i<ret1;i++)
		{
			if(evs[i].data.fd==sfd)//网络请求到达;找到evs集合中监听的sfd的
			{
				new_fd=accept(sfd,NULL,NULL);
				
				while(1){
					int command=0;
					recv_n(new_fd,(char*)&command,sizeof(int));                 //接受login or signup 命令
					//printf("recive command is %d\n",command);
					if(command==111) {if(judge_clientlogin(new_fd)>0) break ; }      //用户登录验证
					else if(command==222) judge_clientSignup(new_fd);                //用户注册
				}
				
				insert_loginlog();      //记录日志信息
				for(j=0;j<child_num;j++)  //找到非忙碌的子进程，然后把new_fd发送给它
				{	
					if(p[j].busy==0)
					{	
						send_fd(p[j].tfds,new_fd);//把描述符发送给子进程,发送到子进程socketpair读端
						p[j].busy=1;
						printf("find a not busy process,send success\n");
						break;
						
					}
				}
				close(new_fd);//父进程close对应的new_fd,将new_fd引用计数减一
			}
			for(j=0;j<child_num;j++)
			{
				if(evs[i].data.fd==p[j].tfds)          
				{
					int k=read(p[j].tfds,&flag,sizeof(flag));
					//printf("child%d flag is\n",flag);
					p[j].busy=0;
					if(k==0) break;
					if(k>0) printf("child%d is not busy\n",p[j].pid);	
				}
			}   				
		}
	}
}

void insert_loginlog(){
	int logfd=open(LogFile,O_CREAT|O_WRONLY|O_APPEND,0666);    //O_APPEND 以追加形式打开文件
	char nowtime[100]={0};
	time_t timep;
    struct tm *p; 
    time(&timep);
    p=localtime(&timep);     //取得当地时间
    sprintf (nowtime,"[login time]  %d-%d-%d %d:%d:%d\n", (1900+p->tm_year),(1+p->tm_mon), p->tm_mday,p->tm_hour, p->tm_min, p->tm_sec);
    //printf("nowtime %s",nowtime);
	write(logfd,nowtime,strlen(nowtime)*sizeof(char));
	close(logfd);
}

int judge_clientlogin(int new_fd){
	int len;
	char usernamebuf[100];
	char passwordbuf[100];
	train t;
	memset(usernamebuf,0,sizeof(usernamebuf));
	memset(passwordbuf,0,sizeof(passwordbuf));
	memset(&t,0,sizeof(t));
	recv_n(new_fd,(char *)&len,sizeof(int));
	recv_n(new_fd,usernamebuf,len);                   //接受client username
	//printf("login username %s\n",usernamebuf);   
	
	char salt[15]={0};
	memset(&t,0,sizeof(t));
	query_salt(usernamebuf,salt);
	t.len=strlen(salt);
	//printf("salt %d\n",t.len);
	if(t.len>0) {
		strcpy(t.buf,salt);
		send_n(new_fd,(char*)&t,sizeof(int)+t.len);     //发送盐值
	}
	else if(t.len==0) {
		send_n(new_fd,(char*)&t,sizeof(int)+t.len);     //如果未注册过username，发送空数据包 t.len=0
		return -1;
	}
	
	memset(&t,0,sizeof(t));
	recv_n(new_fd,(char *)&len,sizeof(int));
	recv_n(new_fd,passwordbuf,len);                           //接受密文密码
	char encrypted_password[100]={0};
	query_encrypted_password(usernamebuf,encrypted_password);
	if(!strcmp(passwordbuf,encrypted_password)) t.len=0;
	else t.len=-1;
	printf("compare result %d\n",t.len);
	send_n(new_fd,(char*)&t,sizeof(int));     //-1 匹配失败  0 匹配成功
	
	if(t.len<0) return -1;   
	else return 1;
}

int judge_clientSignup(int new_fd){
	int len;
	char usernamebuf[100];
	char encrypted_passwordbuf[100];
	char saltbuf[100];
	memset(usernamebuf,0,sizeof(usernamebuf));
	memset(encrypted_passwordbuf,0,sizeof(encrypted_passwordbuf));
	memset(saltbuf,0,sizeof(saltbuf));
	recv_n(new_fd,(char *)&len,sizeof(int));
	recv_n(new_fd,usernamebuf,len);                      //接受用户名
	recv_n(new_fd,(char *)&len,sizeof(int));
	recv_n(new_fd,saltbuf,len);                          //接受盐值
	recv_n(new_fd,(char *)&len,sizeof(int));
	recv_n(new_fd,encrypted_passwordbuf,len);            //接受密文密码
	//printf("username %s\n",usernamebuf);
	//printf("salt %s\n",saltbuf);
	//printf("password %s\n",encrypted_passwordbuf);
	
	//插入数据库
    insert_data(usernamebuf,saltbuf,encrypted_passwordbuf);
	return 0;
}
	

