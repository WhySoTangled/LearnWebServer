#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

// 避免拷贝构造和拷贝赋值
class CNoneCopy {
public:
    ~CNoneCopy(){}
protected:
    CNoneCopy(){}
private:
    CNoneCopy(const CNoneCopy&) = delete;
    CNoneCopy& operator=(const CNoneCopy&) = delete;
};

class CthreadPool : public CNoneCopy {
public:
    CthreadPool(int thread_num = 8, int max_tasks = 10000);
    ~CthreadPool();
    using Task = std::packaged_task<void()>;

    // TODO: 不确定这么声明可不可行
    template <class F, class... Args>
    auto commit(F&& f, Args&&... args) 
        -> std::future<decltype(std::forward<F>(f)(std::forward<Args>(args)...))>;

private:
    void start();
    void stop(); 

private:
    std::atomic_int             m_thread_num; // num of free threads
    std::atomic_bool            m_stop;
    std::vector<std::thread>    m_threads; // queue of threads
    std::queue<Task>            m_tasks;
    int                         m_max_tasks; // max num of tasks

    std::mutex m_mt; 
    std::condition_variable  m_cv;
    
};


// TODO: 这里如果thread_num过大会怎么样？
CthreadPool::CthreadPool(int thread_num, int max_tasks)
    : m_thread_num(thread_num), m_max_tasks(max_tasks), m_stop(false) {
    if (thread_num <= 0 || m_max_tasks<= 0) {
        throw std::exception();
    } 

    start();
}

CthreadPool::~CthreadPool() {
    stop();
}

/*********************
 * @brief start the thread pool
 */
void CthreadPool::start() {
    for (int i = 0; i < m_thread_num; ++i) {
        m_threads.emplace_back([this]() -> void {
            while (!this->m_stop.load()) {
                Task task;
                {
                    std::unique_lock<std::mutex> mt(m_mt);
                    // wait
                    this->m_cv.wait(mt, [this] {
                        return this->m_stop.load() || !this->m_tasks.empty();
                    });
                    // this->m_stop.load() 和 !this->m_tasks.empty() 至少有一个为true
                    if (this->m_tasks.empty()) { // this->m_stop.load() == true
                        return; // exit
                    }

                    // get a task
                    task = std::move(this->m_tasks.front());
                    this->m_tasks.pop();
                }
                task();// excute the task
            }
        });
    }
}

void CthreadPool::stop() {
    m_stop.store(true);
    m_cv.notify_all();// notify all threads to wake up
    for (auto& thread : m_threads) {
        if (thread.joinable()) {
            std::cout << "join thread: " << thread.get_id() << std::endl;
            thread.join();
        }
    }
}

// TODO: C++11 尾置推导?
template <class F, class... Args>
auto CthreadPool::commit(F&& f, Args&&... args) -> 
    std::future<decltype(std::forward<F>(f)(std::forward<Args>(args)...))> {
    using RetType = decltype(std::forward<F>(f)(std::forward<Args>(args)...));
    if (m_stop.load())
        return std::future<RetType>{};

    auto task = std::make_shared<std::packaged_task<RetType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<RetType> ret = task->get_future();
    {
        std::lock_guard<std::mutex> cv_mt(m_mt);
        m_tasks.emplace([task] { (*task)(); });
    }
    m_cv.notify_one();
    return ret;
}
