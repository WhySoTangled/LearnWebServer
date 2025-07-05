#include <string>
#include "config.h"
#include "webserver.h"

int main(int argc, char* argv[]) {
    std::string user = "user";
    std::string password = "user";
    string databasename = ""; //not defined

    Config config;
    config.parse_arg(argc, argv);

    WebServer server;
    //初始化
    server.init(config.PORT, user, password, databasename, config.LOGWrite, 
                config.OPT_LINGER, config.TRIGMode,  config.sql_num,  config.thread_num, 
                config.close_log, config.actor_model);

    // 日志
    server.log_write();
    // TODO: 归纳合并文件树，完成webserver

    //数据库
    server.sql_pool();
}