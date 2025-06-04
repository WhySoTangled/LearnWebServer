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




## 参考

+ 
