#include "func.h"

void send_data(int new_fd,char* FILENAME)
{
	train t;
	memset(&t,0,sizeof(t));
	strcpy(t.buf,FILENAME);
	t.len=strlen(t.buf);
	//发送文件名给服务器端
	int ret;
	ret=send_n(new_fd,(char*)&t,sizeof(int)+t.len);
	if(-1==ret)
	{
		perror("send");
		return;
	}
	int fd;
	fd=open(FILENAME,O_RDONLY);
	
	//第二次发送文件大小
    struct stat st;
    memset(&t,0,sizeof(t));
    fstat(fd,&st);
    t.len=sizeof(long);
    printf("client file big=%ld\n",st.st_size);
    strcpy(t.buf,(char*)&st.st_size);
    send_n(new_fd,(char*)&t,sizeof(int)+t.len);

	//开多趟火车，发文件内容
	while(memset(&t,0,sizeof(t)),(t.len=read(fd,t.buf,sizeof(t.buf)))>0)
	{
		send_n(new_fd,(char*)&t,sizeof(int)+t.len);
	}
	t.len=0;
	//发送空火车(只包含int len=0)，标示文件已经发送结束
	send_n(new_fd,(char*)&t,sizeof(int)+t.len);	
	//close(new_fd);
}

