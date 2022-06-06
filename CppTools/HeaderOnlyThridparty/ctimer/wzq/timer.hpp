
/*
https://github.com/chengxumiaodaren/wzq_utils/blob/master/timer/include/timer/timer.h

*/

#ifndef __CTIMER_HPP__
#define __CTIMER_HPP__

#ifndef __THREAD_SAFE_MAP__
#define __THREAD_SAFE_MAP__

#include <map>
#include <mutex>

namespace wzq {
// thread safe map
template <typename K, typename V>
class ThreadSafeMap {
   public:
    void Emplace(const K& key, const V& v) {
        std::unique_lock<std::mutex> lock(mutex_);
        map_[key] = v;
    }

    void Emplace(const K& key, V&& v) {
        std::unique_lock<std::mutex> lock(mutex_);
        map_[key] = std::move(v);
    }

    void EraseKey(const K& key) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (map_.find(key) != map_.end()) {
            map_.erase(key);
        }
    }

    bool GetValueFromKey(const K& key, V& value) {
        std::unique_lock<std::mutex> l(mutex_);
        if (map_.find(key) != map_.end()) {
            value = map_[key];
            return true;
        }
        return false;
    }

    bool IsKeyExist(const K& key) {
        std::unique_lock<std::mutex> l(mutex_);
        return map_.find(key) != map_.end();
    }

    std::size_t Size() {
        std::unique_lock<std::mutex> l(mutex_);
        return map_.size();
    }

   private:
    std::map<K, V> map_;
    std::mutex mutex_;
};

}  // namespace wzq

#endif    //__THREAD_SAFE_MAP__

#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

using std::cout;
using std::endl;

namespace wzq {

class ThreadPool {
   public:
    using PoolSeconds = std::chrono::seconds;

    /** 线程池的配置
     * core_threads: 核心线程个数，线程池中最少拥有的线程个数，初始化就会创建好的线程，常驻于线程池
     *
     * max_threads: >=core_threads，当任务的个数太多线程池执行不过来时，
     * 内部就会创建更多的线程用于执行更多的任务，内部线程数不会超过max_threads
     *
     * max_task_size: 内部允许存储的最大任务个数，暂时没有使用
     *
     * time_out: Cache线程的超时时间，Cache线程指的是max_threads-core_threads的线程,
     * 当time_out时间内没有执行任务，此线程就会被自动回收
     */
    struct ThreadPoolConfig {
        int core_threads;
        int max_threads;
        int max_task_size;
        PoolSeconds time_out;
    };

    /**
     * 线程的状态：有等待、运行、停止
     */
    enum class ThreadState { kInit = 0, kWaiting = 1, kRunning = 2, kStop = 3 };

    /**
     * 线程的种类标识：标志该线程是核心线程还是Cache线程，Cache是内部为了执行更多任务临时创建出来的
     */
    enum class ThreadFlag { kInit = 0, kCore = 1, kCache = 2 };

    using ThreadPtr = std::shared_ptr<std::thread>;
    using ThreadId = std::atomic<int>;
    using ThreadStateAtomic = std::atomic<ThreadState>;
    using ThreadFlagAtomic = std::atomic<ThreadFlag>;

    /**
     * 线程池中线程存在的基本单位，每个线程都有个自定义的ID，有线程种类标识和状态
     */
    struct ThreadWrapper {
        ThreadPtr ptr;
        ThreadId id;
        ThreadFlagAtomic flag;
        ThreadStateAtomic state;

        ThreadWrapper() {
            ptr = nullptr;
            id = 0;
            state.store(ThreadState::kInit);
        }
    };
    using ThreadWrapperPtr = std::shared_ptr<ThreadWrapper>;
    using ThreadPoolLock = std::unique_lock<std::mutex>;

    ThreadPool(ThreadPoolConfig config) : config_(config) {
        this->total_function_num_.store(0);
        this->waiting_thread_num_.store(0);

        this->thread_id_.store(0);
        this->is_shutdown_.store(false);
        this->is_shutdown_now_.store(false);

        if (IsValidConfig(config_)) {
            is_available_.store(true);
        } else {
            is_available_.store(false);
        }
    }

    ~ThreadPool() { ShutDown(); }

    bool Reset(ThreadPoolConfig config) {
        if (!IsValidConfig(config)) {
            return false;
        }
        if (config_.core_threads != config.core_threads) {
            return false;
        }
        config_ = config;
        return true;
    }

    // 开启线程池功能
    bool Start() {
        if (!IsAvailable()) {
            return false;
        }
        int core_thread_num = config_.core_threads;
        cout << "Init thread num " << core_thread_num << endl;
        while (core_thread_num-- > 0) {
            AddThread(GetNextThreadId());
        }
        cout << "Init thread end" << endl;
        return true;
    }

    // 获取正在处于等待状态的线程的个数
    int GetWaitingThreadSize() { return this->waiting_thread_num_.load(); }

    // 获取线程池中当前线程的总个数
    int GetTotalThreadSize() { return this->worker_threads_.size(); }

    // 放在线程池中执行函数
    template <typename F, typename... Args>
    auto Run(F &&f, Args &&... args) -> std::shared_ptr<std::future<std::result_of_t<F(Args...)>>> {
        if (this->is_shutdown_.load() || this->is_shutdown_now_.load() || !IsAvailable()) {
            return nullptr;
        }
        if (GetWaitingThreadSize() == 0 && GetTotalThreadSize() < config_.max_threads) {
            AddThread(GetNextThreadId(), ThreadFlag::kCache);
        }

        using return_type = std::result_of_t<F(Args...)>;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        total_function_num_++;

        std::future<return_type> res = task->get_future();
        {
            ThreadPoolLock lock(this->task_mutex_);
            this->tasks_.emplace([task]() { (*task)(); });
        }
        this->task_cv_.notify_one();
        return std::make_shared<std::future<std::result_of_t<F(Args...)>>>(std::move(res));
    }

    // 获取当前线程池已经执行过的函数个数
    int GetRunnedFuncNum() { return total_function_num_.load(); }

    // 关掉线程池，内部还没有执行的任务会继续执行
    void ShutDown() {
        ShutDown(false);
        cout << "shutdown" << endl;
    }

    // 执行关掉线程池，内部还没有执行的任务直接取消，不会再执行
    void ShutDownNow() {
        ShutDown(true);
        cout << "shutdown now" << endl;
    }

    // 当前线程池是否可用
    bool IsAvailable() { return is_available_.load(); }

   private:
    void ShutDown(bool is_now) {
        if (is_available_.load()) {
            if (is_now) {
                this->is_shutdown_now_.store(true);
            } else {
                this->is_shutdown_.store(true);
            }
            this->task_cv_.notify_all();
            is_available_.store(false);
        }
    }

    void AddThread(int id) { AddThread(id, ThreadFlag::kCore); }

    void AddThread(int id, ThreadFlag thread_flag) {
        cout << "AddThread " << id << " flag " << static_cast<int>(thread_flag) << endl;
        ThreadWrapperPtr thread_ptr = std::make_shared<ThreadWrapper>();
        thread_ptr->id.store(id);
        thread_ptr->flag.store(thread_flag);
        auto func = [this, thread_ptr]() {
            for (;;) {
                std::function<void()> task;
                {
                    ThreadPoolLock lock(this->task_mutex_);
                    if (thread_ptr->state.load() == ThreadState::kStop) {
                        break;
                    }
                    cout << "thread id " << thread_ptr->id.load() << " running start" << endl;
                    thread_ptr->state.store(ThreadState::kWaiting);
                    ++this->waiting_thread_num_;
                    bool is_timeout = false;
                    if (thread_ptr->flag.load() == ThreadFlag::kCore) {
                        this->task_cv_.wait(lock, [this, thread_ptr] {
                            return (this->is_shutdown_ || this->is_shutdown_now_ || !this->tasks_.empty() ||
                                    thread_ptr->state.load() == ThreadState::kStop);
                        });
                    } else {
                        this->task_cv_.wait_for(lock, this->config_.time_out, [this, thread_ptr] {
                            return (this->is_shutdown_ || this->is_shutdown_now_ || !this->tasks_.empty() ||
                                    thread_ptr->state.load() == ThreadState::kStop);
                        });
                        is_timeout = !(this->is_shutdown_ || this->is_shutdown_now_ || !this->tasks_.empty() ||
                                       thread_ptr->state.load() == ThreadState::kStop);
                    }
                    --this->waiting_thread_num_;
                    cout << "thread id " << thread_ptr->id.load() << " running wait end" << endl;

                    if (is_timeout) {
                        thread_ptr->state.store(ThreadState::kStop);
                    }

                    if (thread_ptr->state.load() == ThreadState::kStop) {
                        cout << "thread id " << thread_ptr->id.load() << " state stop" << endl;
                        break;
                    }
                    if (this->is_shutdown_ && this->tasks_.empty()) {
                        cout << "thread id " << thread_ptr->id.load() << " shutdown" << endl;
                        break;
                    }
                    if (this->is_shutdown_now_) {
                        cout << "thread id " << thread_ptr->id.load() << " shutdown now" << endl;
                        break;
                    }
                    thread_ptr->state.store(ThreadState::kRunning);
                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                }
                task();
            }
            cout << "thread id " << thread_ptr->id.load() << " running end" << endl;
        };
        thread_ptr->ptr = std::make_shared<std::thread>(std::move(func));
        if (thread_ptr->ptr->joinable()) {
            thread_ptr->ptr->detach();
        }
        this->worker_threads_.emplace_back(std::move(thread_ptr));
    }

    void Resize(int thread_num) {
        if (thread_num < config_.core_threads) return;
        int old_thread_num = worker_threads_.size();
        cout << "old num " << old_thread_num << " resize " << thread_num << endl;
        if (thread_num > old_thread_num) {
            while (thread_num-- > old_thread_num) {
                AddThread(GetNextThreadId());
            }
        } else {
            int diff = old_thread_num - thread_num;
            auto iter = worker_threads_.begin();
            while (iter != worker_threads_.end()) {
                if (diff == 0) {
                    break;
                }
                auto thread_ptr = *iter;
                if (thread_ptr->flag.load() == ThreadFlag::kCache &&
                    thread_ptr->state.load() == ThreadState::kWaiting) {  // wait
                    thread_ptr->state.store(ThreadState::kStop);          // stop;
                    --diff;
                    iter = worker_threads_.erase(iter);
                } else {
                    ++iter;
                }
            }
            this->task_cv_.notify_all();
        }
    }

    int GetNextThreadId() { return this->thread_id_++; }

    bool IsValidConfig(ThreadPoolConfig config) {
        if (config.core_threads < 1 || config.max_threads < config.core_threads || config.time_out.count() < 1) {
            return false;
        }
        return true;
    }

   private:
    ThreadPoolConfig config_;

    std::list<ThreadWrapperPtr> worker_threads_;

    std::queue<std::function<void()>> tasks_;
    std::mutex task_mutex_;
    std::condition_variable task_cv_;

    std::atomic<int> total_function_num_;
    std::atomic<int> waiting_thread_num_;
    std::atomic<int> thread_id_;

    std::atomic<bool> is_shutdown_now_;
    std::atomic<bool> is_shutdown_;
    std::atomic<bool> is_available_;
};

}  // namespace wzq

#endif   //__THREAD_POOL__


#ifndef __TIMER__
#define __TIMER__

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>




namespace wzq {
class TimerQueue {
   public:
    struct InternalS {
        std::chrono::time_point<std::chrono::high_resolution_clock> time_point_;
        std::function<void()> func_;
        int repeated_id;
        bool operator<(const InternalS& b) const { return time_point_ > b.time_point_; }
    };

   public:
    bool Run() {
        bool ret = thread_pool_.Start();
        if (!ret) {
            return false;
        }
        std::thread([this]() { RunLocal(); }).detach();
        return true;
    }

    bool IsAvailable() { return thread_pool_.IsAvailable(); }

    int Size() { return queue_.size(); }

    void Stop() {
        running_.store(false);
        cond_.notify_all();
        thread_pool_.ShutDown();
    }

    template <typename R, typename P, typename F, typename... Args>
    void AddFuncAfterDuration(const std::chrono::duration<R, P>& time, F&& f, Args&&... args) {
        InternalS s;
        s.time_point_ = std::chrono::high_resolution_clock::now() + time;
        s.func_ = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(s);
        cond_.notify_all();
    }

    template <typename F, typename... Args>
    void AddFuncAtTimePoint(const std::chrono::time_point<std::chrono::high_resolution_clock>& time_point, F&& f,
                            Args&&... args) {
        InternalS s;
        s.time_point_ = time_point;
        s.func_ = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(s);
        cond_.notify_all();
    }

    template <typename R, typename P, typename F, typename... Args>
    int AddRepeatedFunc(int repeat_num, const std::chrono::duration<R, P>& time, F&& f, Args&&... args) {
        int id = GetNextRepeatedFuncId();
        repeated_id_state_map_.Emplace(id, RepeatedIdState::kRunning);
        auto tem_func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        AddRepeatedFuncLocal(repeat_num - 1, time, id, std::move(tem_func));
        return id;
    }

    void CancelRepeatedFuncId(int func_id) { repeated_id_state_map_.EraseKey(func_id); }

    int GetNextRepeatedFuncId() { return repeated_func_id_++; }

    TimerQueue() : thread_pool_(wzq::ThreadPool::ThreadPoolConfig{4, 4, 40, std::chrono::seconds(4)}) {
        repeated_func_id_.store(0);
        running_.store(true);
    }

    ~TimerQueue() { Stop(); }

    enum class RepeatedIdState { kInit = 0, kRunning = 1, kStop = 2 };

   private:
    void RunLocal() {
        while (running_.load()) {
            std::unique_lock<std::mutex> lock(mutex_);
            if (queue_.empty()) {
                cond_.wait(lock);
                continue;
            }
            auto s = queue_.top();
            auto diff = s.time_point_ - std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() > 0) {
                cond_.wait_for(lock, diff);
                continue;
            } else {
                queue_.pop();
                lock.unlock();
                thread_pool_.Run(std::move(s.func_));
            }
        }
    }

    template <typename R, typename P, typename F>
    void AddRepeatedFuncLocal(int repeat_num, const std::chrono::duration<R, P>& time, int id, F&& f) {
        if (!this->repeated_id_state_map_.IsKeyExist(id)) {
            return;
        }
        InternalS s;
        s.time_point_ = std::chrono::high_resolution_clock::now() + time;
        auto tem_func = std::move(f);
        s.repeated_id = id;
        s.func_ = [this, &tem_func, repeat_num, time, id]() {
            tem_func();
            if (!this->repeated_id_state_map_.IsKeyExist(id) || repeat_num == 0) {
                return;
            }
            AddRepeatedFuncLocal(repeat_num - 1, time, id, std::move(tem_func));
        };
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(s);
        lock.unlock();
        cond_.notify_all();
    }

   private:
    std::priority_queue<InternalS> queue_;
    std::atomic<bool> running_;
    std::mutex mutex_;
    std::condition_variable cond_;

    wzq::ThreadPool thread_pool_;

    std::atomic<int> repeated_func_id_;
    wzq::ThreadSafeMap<int, RepeatedIdState> repeated_id_state_map_;
};

}  // namespace wzq

#endif  //  __TIMER__

#endif  //  __CTIMER_HPP__