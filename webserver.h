#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <string>
#include "./http/http_conn.h"
#include "./timer/lst_timer.h"

using std::string;

const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;             //最小超时单位

class WebServer {
public:
    WebServer();
    ~WebServer();

    void init(int port , string user, string passWord, string databaseName,
              int log_write , int opt_linger, int trigmode, int sql_num,
              int thread_num, int close_log, int actor_model);

    void log_write();
private:
    
    int m_port;         //端口号
    char* m_root;       //root文件夹路径
    int m_log_write;    //日志写入方式
    int m_close_log;    //是否关闭日志
    int m_actormodel;   //并发模型选择

    string m_user;         //登陆数据库用户名
    string m_passWord;     //登陆数据库密码
    string m_databaseName; //使用数据库名
    int m_sql_num;

    //about 线程池
    // threadpool<http_conn> *m_pool;
    int m_thread_num;

    int m_OPT_LINGER;   //优雅关闭链接
    int m_TRIGMode;     //触发组合模式

    http_conn *users;
    
    client_data *users_timer; //定时器
    
};

#endif