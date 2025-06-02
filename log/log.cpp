#include "log.h"

Log::Log()
{
    m_count     = 0;
    m_is_async  = false;
}

Log::~Log()
{
    if (m_fp != NULL) {
        fclose(m_fp);
    }
}

bool Log::init(const char *file_name, int close_log, int log_buf_size, int split_lines, int max_queue_size)
{
    // 默认 0，若不是0则需要设置异步
    if (max_queue_size >= 1)
    {
        m_is_async = true;
        m_log_queue = new block_queue<string>(max_queue_size);
        // TODO
    }
        
}