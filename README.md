# 使用概述 
   
#**启动**  
server-> make;./server  ../conf/serverconf.ini      client-> make; ./client  127.0.0.1  2000     
默认 server ip为主机ip,port=2000,子进程个数=5    

#**server端**   
进程池响应client 登录、注册、下载、上传、断点下载、日志记录功能   
#**client端**  
将对应命令的数据包进行发送,包括退出处理机制
  
#**登录、注册**   
注册: client salt盐值随机生成$6$.......格式,crypt SHA-512加密,即可得到密文密码,tcp发送给server保存到mysql中    
登录: server 先发送已保存的salt给client,client将加密后的密文密码发送给server,server与mysql数据进行比对    
server、client建立连接后,处于while 1,只有login成功后break;   
login_password.sql 文件创建login_password数据库以及对应的表结构   

#**命令响应**  
1.cd 进入对应目录    
2.ls 列出相应目录文件    
3.puts 将本地文件上传至服务器    
4.gets 文件名下载服务器文件到本地    
5.remove 删除服务器上文件   
6.pwd 显示目前所在路径    
7.其他命令不响应    
8.客户端断开,server 子进程结束等待下一次连接    

#**下载、上传**  
mmap映射文件,得到映射区指针,memcpy到数据包中不断发送      

#**断点下载**  
本地存在同名文件,将已有大小发送给server,server mmap映射时偏移对应大小位置即可       

#**日志记录**   
登录用户、登录时间、命令操作的信息记录保存到本地文件       
server func.h 记录日志的绝度路径#define LogFile "/home/cuixuange/20170714/server/server.log" 防止cd 命令后改变了工作路径所以采用了绝对路径  

#**代码行数**   
find ./ -name "*.c"|xargs wc -l   1700行左右
find /home/cuixuange/Linux_C_vacation -name *.c -or -name *.h -or -name *.cpp | xargs cat | grep -v ^$ | wc -l 