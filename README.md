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
  - [x] 实现了互斥锁、信号量、条件变量
  - [x] 实现了block_queue，一个简单的队列



```c++
/// 线程控制
Linux POSIX -lpthread
#include <pthread.h>
	pthread_create()//创建线程 success : 0 errno
    gitpid()// 获取进程id
    pthread_self()//线程id
//终止线程
    pthread_cancel();
	pthread_exit();

	//调用该函数的线程将挂起等待，直到id为thread的线程终止
	pthread_join(pthread_t thread, void **value_ptr);
```



```c++
/// 线程间同步
#include <pthread.h>
/// mutex

	// 互斥锁（Mutex，Mutual Exclusive Lock）
	int pthread_mutex_destroy(pthread_mutex_t *mutex);
	int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr);
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;// 静态分配

	// 加锁 解锁
	int pthread_mutex_lock(pthread_mutex_t *mutex);
	int pthread_mutex_trylock(pthread_mutex_t *mutex);
	int pthread_mutex_unlock(pthread_mutex_t *mutex);

/// Condition Variable
	int pthread_cond_destroy(pthread_cond_t *cond);
	int pthread_cond_init(pthread_cond_t *restrict cond, const pthread_condattr_t *restrict attr);
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
```



epoll对文件描述符的操作模式

```c++
// 对新连接的处理( fd == sockfd )的处理是LT还是ET
CwebServer::m_listen_trig_mode
// 对socket的读缓冲区的读取是LT还是ET
CwebServer::m_conn_trig_mode
// 上面两个模式的合并，即 00，01，10，11
CwebServer::m_trig_mode
// http连接的epoll模式
ChttpConn::m_triggerMode
   
// 根据CwebServer::m_conn_trig_mode决定怎么读缓冲区
bool ChttpConn::read();
```

实例运行时选择的模式：LT/ET，reactor/proactor都只是在构造或者调用init赋了值，具体到业务逻辑时才会对几个变量进行判断选择分支

```c++
CwebServer::    int m_actor_model; // 0: proactor, 1: reactor
```

整个程序不同类的所有实例共享同一个epoll文件描述符epollfd，虽然在每个类中可能名字不一样

```c++
int 		CwebServer::m_epollfd;
static int 	Cutils::m_epollfd;
static int 	ChttpConn::m_epollfd;
```

服务器的事件处理模式相关

```c++
// 0 : Practor 1 : Reactor
int CwebServer::m_actor_model;
```



跳过了信号相关的部分，sigaction，alarm之类的

数据库的接驳相关逻辑仍是空白，未曾实现



append_p和append有什么区别？reactor模式和proactor的不同，这里选择另一种方式实现

通用线程池的task packged封装的实现

选择llfc的泛用线程池，将任务封装后通过commit加入队伍执行，暂时将任务设为实例中的私有方法，后续运行时看看可不可行吧，如果不行的话需要改成共有

准备跳过http的处理，读通复制粘贴吧










## 参考

+ 
