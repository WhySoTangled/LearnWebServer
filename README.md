# LearnWebServer
a record about learn how to build a webserver by C++ of mine   

​    计划通过逐行阅读、逐行写入，以及阅读资料等学习、开发运行一个Web服务器，参考项目为《[qinguoyi/TinyWebServer: :fire: Linux下C++轻量级WebServer服务器](https://github.com/qinguoyi/TinyWebServer/tree/master)》。

## 项目流程

+ [wsl安装MySQL](https://blog.csdn.net/weixin_43796621/article/details/148100973)



【Linux系统编程】I/O多路复用

传统IO：Input把文件读入内存，output从内存写入文件

BIO：Blocking IO 阻塞等待；

​    同一时刻只能处理一个操作，效率低；可以用多进程、多线程解决，但是效率低

NIO Nonblocking IO非阻塞、忙碌轮询；

​    非阻塞，轮询提高了程序的执行效率，需要占用更多的CPU和系统资源



## 进度和Todo-list：

20250519 : 启动了吃灰许久的wsl，apt安装了mysql-server和libmysqlclient-dev，但是还没来得及配置MySQL；在GitHub新建了仓库，并在wsl配置好了sshkey，绑定成功。

- [x] 配置MySQL
  - [x] 安装mysql
  - [x] 配置远程访问功能

- [ ] 开始项目
  - [x] 线性阅读代码，拷贝，到LOG了
  - [ ] 阅读locker.h，了解了封装信号类和互斥锁类




## 参考

+ 
