#include "sql_connection_pool.h"
#include "log.h"

connection_pool::connection_pool() {
    m_CurConn = 0;
	m_FreeConn = 0;
}

connection_pool::~connection_pool() {
    DestroyPool();
}

/**************
 * @brief 获取数据库连接池单例
 */
connection_pool *connection_pool::GetInstance()
{
	static connection_pool connPool;
	return &connPool;
}

/******************
 * @brief 初始化数据库连接池
 * @param url 数据库地址
 * @param User 数据库用户名
 * @param PassWord 数据库密码
 * @param DBName 数据库名
 * @param Port 数据库端口号
 * @param MaxConn 最大连接数
 * @param close_log 日志开关
 ************/
void connection_pool::init(string url, string User, string PassWord, string DBName, int Port, int MaxConn, int close_log)
{
	m_url = url;
	m_Port = Port;
	m_User = User;
	m_PassWord = PassWord;
	m_DatabaseName = DBName;
	m_close_log = close_log;

	for (int i = 0; i < MaxConn; i++)
	{
		MYSQL *con = NULL;
		con = mysql_init(con);

		if (con == NULL)
		{
			LOG_ERROR("MySQL Error");
			exit(1);
		}
		con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);

		if (con == NULL)
		{
			LOG_ERROR("MySQL Error");
			exit(1);
		}
		connList.push_back(con);
		++m_FreeConn;
	}

	reserve = sem(m_FreeConn);

	m_MaxConn = m_FreeConn;
}

//销毁数据库连接池
void connection_pool::DestroyPool()
{

	lock.lock();
	if (connList.size() > 0)
	{
		list<MYSQL *>::iterator it;
		for (it = connList.begin(); it != connList.end(); ++it)
		{
			MYSQL *con = *it;
			mysql_close(con);
		}
		m_CurConn = 0;
		m_FreeConn = 0;
		connList.clear();
	}

	lock.unlock();
}