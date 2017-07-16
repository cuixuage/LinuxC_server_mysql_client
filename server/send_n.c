#include "func.h"

int send_n(int sfd,char* p,int len)
{
	int total=0;
	int ret;
	//防止接收方速度慢导致数据丢失,循环发送len长度内的数据直到本次len数据发送完毕  
	while(total<len)
	{
		ret=send(sfd,p+total,len-total,0);    //send返回值即为实际发送到的数据bytes(recv、read等等类似)
		total=total+ret;
		if(ret<0) {
			printf("send_n break ok\n");
			return (-1);     //断点续传时发送失败返回-1
		}
	}
	return 0;
}

int recv_n(int sfd,char* p,int len)
{
	int total=0;
	int ret;
	//防止发送数据大于1500是产生数据分片,接收方recv快使得数据不完整
	while(total<len)
	{
		ret=recv(sfd,p+total,len-total,0);
		total=total+ret;
	    if(ret<0) {
			printf("recv_n break ok\n");
			return (-1);     //断点续传时发送失败返回-1
		}
	}
	return 0;
}	

