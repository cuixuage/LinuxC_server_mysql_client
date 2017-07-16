#include "func.h"
int sfd;
void sigfunc(int signalnum){          //捕获ctrl+c 信号作为退出机制
	train t;
	memset(&t,0,sizeof(t));
	t.len=0;                        //只传递0号命令
	send_n(sfd,(char*)&t,sizeof(int)); 
	close(sfd);
	kill(getpid(),SIGKILL);
}

void judge(ptrain pt,char* buf1,char* buf2){        //输入命令的字符串切割
	strcpy(buf1,pt->buf);
	strcpy(buf2,pt->buf);
	for(int i=0;i<strlen(buf1);i++){
	    if(buf1[i]==' ') {buf1[i]='\0';break;}
	}
	//printf("buf1 %s\n",buf1);
	for(int i=0;i<strlen(buf2);i++){
        if(buf2[i]==' '){
            sprintf(buf2,"%s",&buf2[i+1]);   //conf[i]是i后面所有字符的字符串
            break;
		}
	}
	//printf("buf2 %s",buf2);    //buf2末尾没有换行符
	return;
 }
int recivefile_formserver(int sfd){
    int len;
	char buf[1000]={0};
	recv_n(sfd,(char *)&len,sizeof(len));
	recv_n(sfd,buf,len);     //第一次传递文件名称,保存到buf中
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
    printf("file big=%ld\n",big);

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
int judgefile_exit(char *filename){       //判断文件名称是否存在
		DIR  *dir;
		dir=opendir(getcwd(NULL,0));
		struct dirent *entry;
		struct stat statbuf;
		int flag=-1;     //默认文件名称不存在
		while((entry=readdir(dir))!=NULL)	{
			    memset(&statbuf,0,sizeof(statbuf));
	           	stat(entry->d_name,&statbuf);
				if(S_ISREG(statbuf.st_mode)){
					if(!strcmp(filename,entry->d_name))  //文件名称已存在
					    flag=statbuf.st_size;   //返回文件大小
				}         		
     	}	
		closedir(dir);
		return flag;
}
int recivefile_formserver_again(int sfd,char *filename,int need_seek){

	int len;
	char buf[1000]={0};
	recv_n(sfd,(char *)&len,sizeof(len));
	recv_n(sfd,buf,len);     //第一次传递文件名称,保存到buf中
	int fd=open(buf,O_CREAT|O_WRONLY|O_APPEND,0666);   //将server 文件保存到client端,以追加形式写入数据
	if(-1==fd)
	{
		perror("open");
		return -1;
	}
    //第二次接受文件长度的数据包
    recv_n(sfd,(char*)&len,sizeof(len));
    long big;
    recv_n(sfd,(char*)&big,sizeof(len));
    printf("file big=%ld\n",big);

	//lseek(fd,need_seek,SEEK_SET);   //移动读写指针
	
    long check=need_seek;
    long l=need_seek;   //文件长度初始值为 need_seek
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
            printf("load down ok--最终文件大小为 %ld\n",l);
			break;
		}
	}
	close(fd);
	return 0;
}	
int main(int argc,char* argv[])
{
	if(argc!=3)
	{
		printf("error args\n");
		return -1;
	}
	//int sfd;
	sfd=socket(AF_INET,SOCK_STREAM,0);
	if(-1==sfd)
	{
		perror("socket");
		return -1;
	}
	struct sockaddr_in ser;
	memset(&ser,0,sizeof(ser));
	ser.sin_family=AF_INET;
	ser.sin_port=htons(atoi(argv[2]));
	ser.sin_addr.s_addr=inet_addr(argv[1]);
	int ret;
	ret=connect(sfd,(struct sockaddr*)&ser,sizeof(ser));
	if(-1==ret)
	{
		perror("connect");
		return -1;
	}
	
    signal(SIGINT,sigfunc);   //异步捕获SIG_INT
	
	while(1){
		// return 1 登陆成功  return 0注册完成  return -1 登录失败
		if(judge_login_singup()>0) {printf("login success! welcome!\n");break;}     //注册 or 登陆
	}
	
	train t;
	int ret_length;
	char buf1[100]={0};         //保存命令
	char buf2[100]={0};         //保存命令后面的参数
	while(1){	
	    memset(&t,0,sizeof(t));
	    ret_length=read(STDIN_FILENO,t.buf,sizeof(t.buf));
	    if(ret_length>0) {
			t.buf[strlen(t.buf)-1]='\0';    //删除字符串末尾换行符 
	        memset(buf1,0,sizeof(buf1));
	        memset(buf2,0,sizeof(buf2));
			judge(&t,buf1,buf2);
			//printf("buf1 %s buf2 %s\n",buf1,buf2);
			if(!strcmp(buf1,"cd")){
				t.len=1;
				send_n(sfd,(char*)&t,sizeof(int));
				strcpy(t.buf,buf2);            //传递cd命令内容
				t.len=strlen(t.buf);
				send_n(sfd,(char*)&t,sizeof(int)+t.len);
			}
			else if(!strcmp(buf1,"ls")){
				t.len=2;                        //只传递ls命令
				send_n(sfd,(char*)&t,sizeof(int));   
			}	
			else if(!strcmp(buf1,"puts")){
                printf("uploadfile=%s\n",buf2);
				t.len=3;
				send_n(sfd,(char*)&t,sizeof(int));
				send_data(sfd,buf2);      //上传文件到服务器
				printf("puts file ok\n");
			}
			else if(!strcmp(buf1,"gets")){
				int exit=judgefile_exit(buf2);    //判断文件是否已存在进行断点续传
				
				if(exit>0){     //文件名称已存在
				    printf("文件名称已经存在,大小是 %d\n",exit);
					t.len=44;
				    send_n(sfd,(char*)&t,sizeof(int));    //发送44号命令,相当于断点续传命令
				    strcpy(t.buf,buf2);                   //发送文件名称
				    t.len=strlen(t.buf);
				    send_n(sfd,(char*)&t,sizeof(int)+t.len);
					
					//发送已存在的文件大小
					t.len=exit;
				    send_n(sfd,(char*)&t,sizeof(int)); 
					printf("已存在大小 %d\n",exit);
					
				    recivefile_formserver_again(sfd,buf2,exit);     //文件再次接收
					
				}	
				else if(exit<0){ //不存在此文件名称
				    t.len=4;
				    send_n(sfd,(char*)&t,sizeof(int));
				    strcpy(t.buf,buf2);            //传递gets命令中文件名称
				    t.len=strlen(t.buf);
				    send_n(sfd,(char*)&t,sizeof(int)+t.len);
				    recivefile_formserver(sfd);     //文件接收
				}
			}
			else if(!strcmp(buf1,"remove")){
				t.len=5;
				send_n(sfd,(char*)&t,sizeof(int));
				strcpy(t.buf,buf2);            //传递命令中文件名称
				t.len=strlen(t.buf);
				send_n(sfd,(char*)&t,sizeof(int)+t.len);
			}
			else if(!strcmp(buf1,"pwd")){
				t.len=6;
				send_n(sfd,(char*)&t,sizeof(int));
			}
			else {
				printf("please input right command\n");
			}
		}
	}

	close(sfd);
	return 0;
}
int judge_login_singup(){
	// 登陆成功返回1   登录失败返回-1
	char judge_check[5]={0};
	int check=0;
	int login_num=111;
	int signup_num=222;
	printf("login or signup? (1/2)\n");
	int z=read(STDIN_FILENO,&judge_check,sizeof(judge_check));     //把换行符去掉！！！！！  
    judge_check[strlen(judge_check)-1]='\0';	
	check=atoi(judge_check);
	if(z<=0) {printf("error\n"); return -1;}
	if(check==1) {
		send_n(sfd,(char*)&login_num,sizeof(int));
		return(client_login());
	}
	else if(check==2) {
		send_n(sfd,(char*)&signup_num,sizeof(int));
		client_signup();
		return 0;
	}
	else return -1;
}
int client_login(){
	int len;
	char usernamebuf[100];
	char passwordbuf[100];
	char saltbuf[100];
	memset(usernamebuf,0,sizeof(usernamebuf));
	memset(passwordbuf,0,sizeof(passwordbuf));
	memset(saltbuf,0,sizeof(saltbuf));

    train t;
	printf("login username:\n");
	read(STDIN_FILENO,usernamebuf,sizeof(usernamebuf));
	usernamebuf[strlen(usernamebuf)-1]='\0';     //去掉换行符
	t.len=strlen(usernamebuf);                 
	strcpy(t.buf,usernamebuf);
	send_n(sfd,(char*)&t,sizeof(int)+t.len);    //向server发送username
	
	recv_n(sfd,(char *)&len,sizeof(len));
	if(len>0) recv_n(sfd,saltbuf,len);                   //保存salt值
    if(len==0) {
		printf("loginuser not exit!\n");                      //server 不存在username
		return -1;
	}
	printf("login password:\n");
	read(STDIN_FILENO,passwordbuf,sizeof(passwordbuf));
	passwordbuf[strlen(passwordbuf)-1]='\0';
    char *encrypted_password=crypt(passwordbuf,saltbuf);
	
	t.len=strlen(encrypted_password);
	strcpy(t.buf,encrypted_password);
	send_n(sfd,(char*)&t,sizeof(int)+t.len);    //向server发送密文密码
	
	recv_n(sfd,(char *)&len,sizeof(len));       //获取匹配结果               
	if(len<0) { printf("Permission deny\n");return -1;}         //递归的话会发生 段错误
    else if(len>=0) {printf("Permission ok\n"); return 1;}
}

void get_rand_str(char* saltbuf){
	char s[10]={0};  //获取随机生成的字符串
	int num=8;
	char *str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"; //定义随机生成字符串表
	int i,lstr;
	char ss[2] = {0};
	lstr = strlen(str);
	srand((unsigned int)time((time_t *)NULL));//使用系统时间来初始化随机数发生器
	for(i = 0; i < num; i++){
		sprintf(ss,"%c",str[(rand()%lstr)]);      //rand()%lstr 
		strcat(s,ss);                             //将随机生成的字符串连接到指定数组后面,strcat在dest末尾先覆盖再添加'\0'
	}
	sprintf(saltbuf,"$6$%s",s); 
}
int client_signup(){
	int len;
	char usernamebuf[100];
	char passwordbuf[100];
	char saltbuf[100];
	memset(usernamebuf,0,sizeof(usernamebuf));
	memset(passwordbuf,0,sizeof(passwordbuf));
	memset(saltbuf,0,sizeof(saltbuf));
	printf("signup username:\n");
	read(STDIN_FILENO,usernamebuf,sizeof(usernamebuf));
	usernamebuf[strlen(usernamebuf)-1]='\0';     //去掉换行符
	printf("signup password:\n");
	read(STDIN_FILENO,passwordbuf,sizeof(passwordbuf));
	passwordbuf[strlen(passwordbuf)-1]='\0';     //去掉换行符
	
	get_rand_str(saltbuf);   //得到盐值
	//printf("salt %s\n",saltbuf);
	char* encrypted_password=crypt(passwordbuf,saltbuf);   //得到密文密码
	
	train t;
	memset(&t,0,sizeof(t));
	t.len=strlen(usernamebuf);
	strcpy(t.buf,usernamebuf);
	send_n(sfd,(char*)&t,sizeof(int)+t.len);   //传递用户名
	memset(&t,0,sizeof(t));
	t.len=strlen(saltbuf);
	strcpy(t.buf,saltbuf);
	send_n(sfd,(char*)&t,sizeof(int)+t.len);   //传递盐值
	memset(&t,0,sizeof(t));
	t.len=strlen(encrypted_password);
	strcpy(t.buf,encrypted_password);
	send_n(sfd,(char*)&t,sizeof(int)+t.len);    //传递密文密码
	return 0;
}
