#ifndef SQL_CONNECTION_POOL_H
#define SQL_CONNECTION_POOL_H
#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <list>
#include "locker.h"

using namespace std;

class connection_pool {
public:
	static connection_pool *GetInstance();
    void init(string url, string User, string PassWord, string DataBaseName, int Port, int MaxConn, int close_log); 
    MYSQL *GetConnection();				 //获取数据库连接
	bool ReleaseConnection(MYSQL *conn); //释放连接
	int GetFreeConn();					 //获取连接
	void DestroyPool();					 //销毁所有连接
    
private:
    connection_pool();
    ~connection_pool();
    int m_MaxConn;  //最大连接数
    int m_CurConn;  //当前已使用的连接数
	int m_FreeConn; //当前空闲的连接数
    list<MYSQL *> connList; //连接池
    sem reserve;
    locker lock;

    string m_url;			 //主机地址
	string m_Port;		 //数据库端口号
	string m_User;		 //登陆数据库用户名
	string m_PassWord;	 //登陆数据库密码
	string m_DatabaseName; //使用数据库名
	int m_close_log;	//日志开关
};

/*class connectionRAII{
public:
	connectionRAII(MYSQL **con, connection_pool *connPool);
	~connectionRAII();
	
private:
	MYSQL *conRAII;
	connection_pool *poolRAII;
};*/


#endif