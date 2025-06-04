/************
 * @brief MySQL C++ Example
 * @details 使用库​​libmysqlclient-dev来简单实现MySQL的连接
********** */
#include <iostream>
#include <string>
#include <mysql/mysql.h>

using namespace std;

string m_user       = "user";
string m_password   = "user";
string m_dbname     = "my_webserver_db";

#include <stdio.h>

int main() {
    MYSQL *conn = mysql_init(NULL);
    // 连接数据库（需替换为实际参数）
    if (!mysql_real_connect(conn, "localhost", m_user.c_str(), m_password.data(), m_dbname.c_str(), 0, NULL, 0)) {
        fprintf(stderr, "连接失败: %s\n", mysql_error(conn));
        return 1;
    }

    // 插入数据
    if (mysql_query(conn, "INSERT INTO users(name, age) VALUES('Alice', 25)")) {
        fprintf(stderr, "插入失败: %s\n", mysql_error(conn));
    }

    // 查询数据
    if (mysql_query(conn, "SELECT * FROM users")) {
        fprintf(stderr, "查询失败: %s\n", mysql_error(conn));
    } else {
        MYSQL_RES *res = mysql_store_result(conn);
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res))) {
            printf("ID: %s, Name: %s, Age: %s\n", row[0], row[1], row[2]);
        }
        mysql_free_result(res);
    }

    // 关闭连接
    mysql_close(conn);
    return 0;
}