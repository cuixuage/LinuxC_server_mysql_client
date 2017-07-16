#include "func.h"

void query_encrypted_password(char *login_username,char *encrypted_password)
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char* server="localhost";
	char* user="root";
	char* password="123";
	char* database="login_password";
	char query[100]="select password from login where username=";
	strcat(query,"'");
	strcat(query,login_username);
	strcat(query,"'");
	//printf("query %s\n",query);
	
	int t;
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
	}else{
		//printf("Connected...\n");
	}
	t=mysql_query(conn,query);
	if(t)
	{
		printf("Error making query:%s\n",mysql_error(conn));
	}else{
		//printf("Query made...\n");
		res=mysql_use_result(conn);
		if(res)
		{
			while((row=mysql_fetch_row(res))!=NULL)
			{	
				//printf("num=%d\n",mysql_num_fields(res));//列数
				for(t=0;t<mysql_num_fields(res);t++){
					//printf("%8s ",row[t]);
					strcpy(encrypted_password,row[t]);
				}
				//printf("\n");
				break;    //只打印一行？？？
			}
		}
		mysql_free_result(res);
	}
	mysql_close(conn);
	return ;
}
void query_salt(char *login_username,char *salt)
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char* server="localhost";
	char* user="root";
	char* password="123";
	char* database="login_password";
	char query[100]="select salt from login where username=";
	strcat(query,"'");
	strcat(query,login_username);
	strcat(query,"'");
	//printf("query %s\n",query);
	
	int t;
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
	}else{
		//printf("Connected...\n");
	}
	t=mysql_query(conn,query);
	if(t)
	{
		printf("Error making query:%s\n",mysql_error(conn));
	}else{
		//printf("Query made...\n");
		res=mysql_use_result(conn);
		if(res)
		{
			while((row=mysql_fetch_row(res))!=NULL)
			{	
				//printf("num=%d\n",mysql_num_fields(res));//列数
				for(t=0;t<mysql_num_fields(res);t++){
					//printf("%8s ",row[t]);
					strcpy(salt,row[t]);
				}
				//printf("\n");
				break;    //只打印一行？？？
			}
		}
		mysql_free_result(res);
	}
	mysql_close(conn);
	return ;
}

void insert_data(char* login_username,char* login_salt,char* login_password){
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char* server="localhost";
	char* user="root";
	char* password="123";
	char* database="login_password";
	char query[200];
	int t;

	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
	}else{
		//printf("Connected...\n");
	}
	//int personID=atoi(argv[2]);
	//while(memset(name,0,sizeof(name)),fgets(name,sizeof(name),fp)!=NULL)
	//{
		//char* login_username="cui2";
		//char* login_salt="$6$nk3MPmgA";
		//char* login_password="$6$nk3MPmgA$5MFpoawRRKWPnSeF2Q7SWGVVA7nDOZ8UfJmcXXmemaeiPWRsq22CDJ8Eba/.VtB4OKZ/D73DtgDOeAP737dHp1";
		//memset(name,0,sizeof(name))
		//name[strlen(name)-1]='\0';
		memset(query,0,sizeof(query));
		sprintf(query,"%s%s%s%s%s%s%s","insert into login(username,salt,password) values('" ,login_username, "','" ,login_salt, "','" ,login_password,"')");
		//puts(query);
		t=mysql_query(conn,query);
		if(t)
		{
			printf("Error making query:%s\n",mysql_error(conn));
			//break;
		}else{
			printf("insert success\n");
		}
	//}			
	//fclose(fp);
	mysql_close(conn);
	return ;
}
