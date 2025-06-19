#ifndef LOG_H
#define LOG_H

#include <iostream>
#include "block_queue.h"
#include <string>
class Log
{
public:
    Log();
    virtual ~Log();
    bool init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);
    static Log *get_instance();
    static void *flush_log_thread(void *args);
    void write_log(int level, const char *format, ...);
    void flush(void);

private:
    void *async_write_log();

    char dir_name[128]; //路径名
    char log_name[128]; //log文件名
    int m_split_lines;  //日志最大行数
    int m_log_buf_size; //日志缓冲区大小
    long long m_count;  //日志行数记录
    int m_today;        //因为按天分类,记录当前时间是那一天
    FILE *m_fp;         //打开log的文件指针
    char *m_buf;
    block_queue<std::string> *m_log_queue; //阻塞队列
    bool m_is_async;                  //是否同步标志位 true : 同步; false : 异步
    locker m_mutex;
    int m_close_log; //关闭日志
};

// TODO: 这段宏定义我本身能看懂，但是不明白实际运行时在哪被调用了
#define LOG_DEBUG(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}

#endif