# LearnWebServer
a record about learn how to build a webserver by C++ of mine   

​    计划通过逐行阅读、逐行写入，以及阅读资料等学习、开发运行一个Web服务器，参考项目为《[qinguoyi/TinyWebServer: :fire: Linux下C++轻量级WebServer服务器](https://github.com/qinguoyi/TinyWebServer/tree/master)》。

## WSL安装MySQL

​    阅读 [Using the MySQL APT Repository](https://dev.mysql.com/doc/refman/8.4/en/linux-installation-apt-repo.html)（MySQL官方文档）后，终端输入：

```bash
~$ sudo apt update
~$ sudo apt install mysql-server
~$ sudo apt install libmysqlclient-dev
```

​    按照官方文档的说法，安装过程中会要求我给root用户设置密码，但是可能是我安装时输入了`-y`？没有遇到这个要求就结束了。安装时我也没按照deepseek说的运行安全配置脚本：

```bash
~$ sudo mysql_secure_installation
```

​    而是直接检查MySQL server的状态了：

```bash
~$ systemctl status mysql
System has not been booted with systemd as init system (PID 1). Can't operate.
Failed to connect to bus: Host is down
```

​    WSL内显示如上，无奈只能换方法：

```bash
~$ service mysql status
 * MySQL is stopped.
~$ service mysql start
 * Starting MySQL database server mysqld                                                                                install: cannot change owner and permissions of ‘/var/run/mysqld’: No such file or directory
~$ sudo service mysql start
[sudo] password for xxx:
 * Starting MySQL database server mysqld                                                                                su: warning: cannot change directory to /nonexistent: No such file or directory
```

​    问ai和百度也没搜出个所以然，只好去看《[使用 WSL 添加或连接数据库 | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows/wsl/tutorials/wsl-database)》官方文档，按照《[使用 systemd 通过 WSL 管理 Linux 服务 | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows/wsl/systemd#how-to-enable-systemd)》的说法，使用 `wsl --install` 命令默认值安装的 Ubuntu 当前版本的默认值现在是 systemd，不知道为什么我的WSL2安装的Ubuntu初始系统默认值不是，遂按教程启用systemd：

1. 确保 WSL 版本为 0.67.6 或更高版本；

2. 打开 Linux 分发版的命令行并输入 cd / 以访问根目录，然后 `ls` 列出文件。 你将看到一个名为“etc”的目录，其中包含分发的 WSL 配置文件。 打开此文件，以便通过输入：`nano /etc/wsl.conf`，使用 Nano 文本编辑器进行更新。

3. 在 `wsl.conf` 文件中添加以下行，你现在已打开此文件来更改用于 systemd 的初始值：

```bash
[boot]
systemd=true
```

4. 退出 Nano 文本编辑器（Ctrl + X，键入 Y 以保存更改，并使用 `enter` 键进行确认）。
4. 然后，需要关闭 Linux 分发版。 可以使用 PowerShell 中的命令 `wsl.exe --shutdown` 重新启动所有 WSL 实例。
4. 重启 Linux 分发版后，systemd 将运行。 可以使用命令 `systemctl status` 来验证它，以显示运行 *状态的* 和命令 `systemctl list-unit-files --type=service`，这将显示与 Linux 分发版关联的任何服务的状态。

​    重新尝试：

```bash
~$ systemctl status mysql
● mysql.service - MySQL Community Server
~$ sudo mysql
[sudo] password for xxx:
Welcome to the MySQL monitor.  Commands end with ; or \g.
Your MySQL connection id is 9
Server version: 8.0.42-0ubuntu0.22.04.1 (Ubuntu)
```

​    可以远程fang'wen

## 进度和Todo-list：

20250519 : 启动了吃灰许久的wsl，apt安装了mysql-server和libmysqlclient-dev，但是还没来得及配置MySQL；在GitHub新建了仓库，并在wsl配置好了sshkey，绑定成功。

- [ ] 配置MySQL
  - [x] 安装mysql
  - [ ] 配置远程访问功能

- [ ] 开始项目



## 参考

+ 
