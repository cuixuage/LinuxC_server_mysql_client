# 使用概述   
**启动**  
**server-> make;./server  ../conf/serverconf.ini      client-> make; ./client  127.0.0.1  2000**   
**默认 server ip为主机ip,port=2000,子进程个数=5**  

**server端**   
进程池响应client 登录、注册、下载、上传、断点下载、日志记录功能   
**client端**  
将对应命令的数据包进行发送
  
#**登录、注册**   
**注册: client salt盐值随机生成$6$.......格式,crypt SHA-512加密,即可得到密文密码,tcp发送给server保存到mysql中**   
**登录: server 先发送已保存的salt给client,client将加密后的密文密码发送给server,server与mysql数据进行比对**   
**server、client建立连接后,处于while 1,只有login成功后break;**   

#**下载、上传**  
**mmap映射文件,得到映射区指针,memcpy到数据包中不断发送**   

#**断点下载**  
**本地存在同名文件,将已有大小发送给server,server mmap映射时偏移对应大小位置即可**   

#**断点下载**  
**登录用户、登录时间、命令操作的信息记录保存到本地文件**  