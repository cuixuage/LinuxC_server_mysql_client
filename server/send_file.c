#include "func.h"

void send_mmpdata(int new_fd,char* FILENAME,void* src,long filesize,long download_file_size){     //filesize代表文件大小; rest_of_file_size 断点续传时已下载文件大小
	train t;
	memset(&t,0,sizeof(t));
	strcpy(t.buf,FILENAME);
	t.len=strlen(t.buf);
	//发送文件名给对客户端
	int ret;
	ret=send_n(new_fd,(char*)&t,sizeof(int)+t.len);
	if(-1==ret)
	{
		perror("send");
		return;
	}

    t.len=sizeof(long);  //发送文件大小
    printf("server file big=%ld\n",filesize);
    strcpy(t.buf,(char*)&filesize);
    send_n(new_fd,(char*)&t,sizeof(int)+t.len);  
	
	src +=download_file_size;          //偏移mmap指针
	filesize -=download_file_size;    //计算需要传递的文件大小
	
	//开多趟火车，发文件内容
	while(memset(&t,0,sizeof(t)),filesize>0){
		if(filesize>sizeof(t.buf)){
		    memcpy(t.buf,(char*)src,sizeof(t.buf));
		    t.len=sizeof(t.buf);   
		}
		else {
			memcpy(t.buf,(char*)src,filesize);
			t.len=filesize;     // strlen只能获取字符串长度  '\0'
		}
		//printf("final t.buf %d\n",t.len);
		//if(t.len>0) printf("t.len %10d\n",t.len);
		src +=sizeof(t.buf);
		filesize -=sizeof(t.buf);
		//printf("filesize %ld\n",filesize);
		
	    int check=send_n(new_fd,(char*)&t,sizeof(int)+t.len);
		if(check<0) {printf("send file break ok\n");return;}

	}
	t.len=0;
	//发送空火车(只包含int len=0)，标示文件已经发送结束
	send_n(new_fd,(char*)&t,sizeof(int)+t.len);	
	//close(new_fd);	
}


void send_data(int new_fd,char* FILENAME)
{
	train t;
	memset(&t,0,sizeof(t));
	strcpy(t.buf,FILENAME);
	t.len=strlen(t.buf);
	//发送文件名给对客户端
	int ret;
	ret=send_n(new_fd,(char*)&t,sizeof(int)+t.len);
	if(-1==ret)
	{
		perror("send");
		return;
	}
	int fd;
	fd=open(FILENAME,O_RDONLY);
    struct stat st;
    memset(&t,0,sizeof(t));
    fstat(fd,&st);
    t.len=sizeof(long);
    printf("server file big=%ld\n",st.st_size);
    strcpy(t.buf,(char*)&st.st_size);
    send_n(new_fd,(char*)&t,sizeof(int)+t.len);

	//开多趟火车，发文件内容
	while(memset(&t,0,sizeof(t)),(t.len=read(fd,t.buf,sizeof(t.buf)))>0)
	{
		int check=send_n(new_fd,(char*)&t,sizeof(int)+t.len);
		if(check<0) {printf("send file break ok\n");return;}    //当客户端断开连接时此处break
	}
	t.len=0;
	//发送空火车(只包含int len=0)，标示文件已经发送结束
	send_n(new_fd,(char*)&t,sizeof(int)+t.len);	
	//close(new_fd);
}

void send_data_again(int new_fd,char* FILENAME,int need_seek){
	train t;
	memset(&t,0,sizeof(t));
	strcpy(t.buf,FILENAME);
	t.len=strlen(t.buf);
	//发送文件名给对客户端
	int ret;
	ret=send_n(new_fd,(char*)&t,sizeof(int)+t.len);
	if(-1==ret)
	{
		perror("send");
		return;
	}
	int fd;
	fd=open(FILENAME,O_RDONLY);
    struct stat st;
    memset(&t,0,sizeof(t));
    fstat(fd,&st);
    t.len=sizeof(long);           //额外获取文件大小，便于客户端百分比显示
    printf("server file big=%ld\n",st.st_size);
    strcpy(t.buf,(char*)&st.st_size);
    send_n(new_fd,(char*)&t,sizeof(int)+t.len);
	
	printf("server file seek=%d\n",need_seek);
	lseek(fd,need_seek,SEEK_SET);   //移动读写指针

	//开多趟火车，发文件内容
	while(memset(&t,0,sizeof(t)),(t.len=read(fd,t.buf,sizeof(t.buf)))>0)
	{
		int check=send_n(new_fd,(char*)&t,sizeof(int)+t.len);
		if(check<0) {printf("send file break ok\n");return;}
	}
	t.len=0;
	//发送空火车(只包含int len=0)，标示文件已经发送结束
	send_n(new_fd,(char*)&t,sizeof(int)+t.len);	
	//close(new_fd);
}

