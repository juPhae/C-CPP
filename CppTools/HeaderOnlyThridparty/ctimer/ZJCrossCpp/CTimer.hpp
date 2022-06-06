//
//  CTimer.hpp
//  ZJCrossCpp
//
//  Created by eafy on 2020/10/5.
//  Copyright © 2020 ZJ. All rights reserved.
//

/*
https://github.com/Eafy/ZJCrossCpp/
https://blog.csdn.net/lizhijian21/article/details/83417747
*/

#ifndef __CTIMER_HPP__
#define __CTIMER_HPP__

#include <stdio.h>
#include <functional>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include <condition_variable>
#include <future>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>


#if defined(__ANDROID__) || defined(ANDROID)    //Android
#define IS_ANDROID_PLATFORM 1
#define IS_IOS_PLATFORM 0
#elif defined(__APPLE__)  //iOS
#define IS_ANDROID_PLATFORM 0
#define IS_IOS_PLATFORM 1
#else
#define IS_ANDROID_PLATFORM 0
#define IS_IOS_PLATFORM 0
#endif



class CTimer
{
public:
    CTimer(const std::string sTimerName = "");   //构造定时器，附带名称
    ~CTimer();
    
    /**
     开始运行定时器

     @param msTime 延迟运行(单位ms)
     @param task 任务函数接口
     @param bLoop 是否循环(默认执行1次)
     @param async 是否异步(默认异步)
     @return true:已准备执行，否则失败
     */
    bool Start(unsigned int msTime, std::function<void()> task, bool bLoop = false, bool async = true);
    
    /**
     取消定时器，同步定时器无法取消(若任务代码已执行则取消无效)
     */
    void Cancel();
    
    /**
     同步执行一次
     #这个接口感觉作用不大，暂时现实在这里

     @param msTime 延迟时间(ms)
     @param fun 函数接口或lambda代码块
     @param args 参数
     @return true:已准备执行，否则失败
     */
    template<typename callable, typename... arguments>
    bool SyncOnce(int msTime, callable&& fun, arguments&&... args) {
        std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(fun), std::forward<arguments>(args)...)); //绑定任务函数或lambda成function
        return Start(msTime, task, false, false);
    }
    
    /**
     异步执行一次任务
     
     @param msTime 延迟及间隔时间（毫秒）
     @param fun 函数接口或lambda代码块
     @param args 参数
     @return true:已准备执行，否则失败
     */
    template<typename callable, typename... arguments>
    bool AsyncOnce(int msTime, callable&& fun, arguments&&... args) {
        std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(fun), std::forward<arguments>(args)...));
        
        return Start(msTime, task, false);
    }
    
    /**
     异步执行一次任务(默认延迟10毫秒后执行)
     
     @param fun 函数接口或lambda代码块
     @param args 参数
     @return true:已准备执行，否则失败
     */
    template<typename callable, typename... arguments>
    bool AsyncOnce(callable&& fun, arguments&&... args) {
        std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(fun), std::forward<arguments>(args)...));
        
        return Start(1, task, false);
    }
    
    
    /**
     异步循环执行任务

     @param msTime 延迟及间隔时间（毫秒）
     @param fun 函数接口或lambda代码块
     @param args 参数
     @return true:已准备执行，否则失败
     */
    template<typename callable, typename... arguments>
    bool AsyncLoop(int msTime, callable&& fun, arguments&&... args) {
        std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(fun), std::forward<arguments>(args)...));
        
        return Start(msTime, task, true);
    }
    
public:
    /// 获取时间戳(毫秒)
    static uint64_t Timestamp();
    
    /// 获取格式化时间
    static std::string FormatTime(const std::string sFormat = "%Y-%m-%d %H:%M:%S");

    /// 获取UTC时间
    static struct tm *UTCTime(long long secTime = 0);

    /// 获取UTC时间(秒)
    static int64_t UTCTime();

    /// 获取与0时区的时差（以秒为单位）
    static int TimeDifFrimGMT();
    
private:
    void DeleteThread();    //删除任务线程

public:
    int m_nCount = 0;   //循环次数
    int m_nTag = 0;     //定时器标签
    
private:
    std::string m_sName;   //定时器名称
    
    std::atomic_bool m_bExpired;       //装载的任务是否已经过期
    std::atomic_bool m_bTryExpired;    //装备让已装载的任务过期(标记)
    std::atomic_bool m_bLoop;          //是否循环
    
    std::thread *m_Thread = nullptr;
    std::mutex m_ThreadLock;
    std::condition_variable_any m_ThreadCon;
};

CTimer::CTimer(const std::string sTimerName):m_bExpired(true), m_bTryExpired(false), m_bLoop(false)
{
    m_sName = sTimerName;
}

CTimer::~CTimer()
{
    m_bTryExpired = true;   //尝试使任务过期
    DeleteThread();
}

bool CTimer::Start(unsigned int msTime, std::function<void()> task, bool bLoop, bool async)
{
    if (!m_bExpired || m_bTryExpired) return false;  //任务未过期(即内部仍在存在或正在运行任务)
    m_bExpired = false;
    m_bLoop = bLoop;
    m_nCount = 0;

    if (async) {
        DeleteThread();
        m_Thread = new std::thread([this, msTime, task]() {
            if (!m_sName.empty()) {
#if (defined(__ANDROID__) || defined(ANDROID))      //兼容Android
                pthread_setname_np(pthread_self(), m_sName.c_str());
#elif defined(__APPLE__)                            //兼容苹果系统
                pthread_setname_np(m_sName.c_str());    //设置线程(定时器)名称
#endif
            }
            
            while (!m_bTryExpired) {
                m_ThreadCon.wait_for(m_ThreadLock, std::chrono::milliseconds(msTime));  //休眠
                if (!m_bTryExpired) {
                    task();     //执行任务

                    m_nCount ++;
                    if (!m_bLoop) {
                        break;
                    }
                }
            }
            
            m_bExpired = true;      //任务执行完成(表示已有任务已过期)
            m_bTryExpired = false;  //为了下次再次装载任务
        });
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(msTime));
        if (!m_bTryExpired) {
            task();
        }
        m_bExpired = true;
        m_bTryExpired = false;
    }
    
    return true;
}

void CTimer::Cancel()
{
    if (m_bExpired || m_bTryExpired || !m_Thread) {
        return;
    }
    
    m_bTryExpired = true;
}

void CTimer::DeleteThread()
{
    if (m_Thread) {
        m_ThreadCon.notify_all();   //休眠唤醒
        m_Thread->join();           //等待线程退出
        delete m_Thread;
        m_Thread = nullptr;
    }
}

#pragma mark -

uint64_t CTimer::Timestamp()
{
    uint64_t msTime = 0;

#if defined(__APPLE__)  //iOS
    if (__builtin_available(iOS 10.0, *)) {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);
        msTime = ((uint64_t)abstime.tv_sec) * 1000 + ((uint64_t)abstime.tv_nsec) / 1000000;
    } else {
        struct timeval abstime;
        gettimeofday(&abstime, NULL);
        msTime = ((uint64_t)abstime.tv_sec) * 1000 + ((uint64_t)abstime.tv_usec) / 1000;
    }
#else
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);

    msTime = ((uint64_t)abstime.tv_sec) * 1000 + ((uint64_t)abstime.tv_nsec) / 1000000;   //需要强制转long long
#endif

    return msTime;
}

std::string CTimer::FormatTime(const std::string sFormat)
{
    time_t timep;
    time (&timep);

    char tmp[64];
    strftime(tmp, sizeof(tmp), sFormat.c_str(), localtime(&timep));

    return std::string(tmp);
}

struct tm *CTimer::UTCTime(long long secTime)
{
    time_t timep;
    if (secTime) {
        timep = secTime;
    } else {
        time (&timep);
    }

    struct tm *data = gmtime(&timep);
    data->tm_year += 1900;
    data->tm_mon += 1;

    return data;
}

int64_t CTimer::UTCTime()
{
    int64_t msTime = 0;

#if defined(__APPLE__)  //iOS
    if (__builtin_available(iOS 10.0, *)) {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);
        msTime = ((int64_t)abstime.tv_sec) * 1000 + ((int64_t)abstime.tv_nsec) / 1000000;
    } else {
        struct timeval abstime;
        gettimeofday(&abstime, NULL);
        msTime = ((int64_t)abstime.tv_sec) * 1000 + ((int64_t)abstime.tv_usec) / 1000;
    }
#else
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);

    msTime = (int64_t)abstime.tv_sec;
#endif

    return msTime;
}

int CTimer::TimeDifFrimGMT()
{
    time_t now = time(NULL);
    struct tm *gmTime = gmtime(&now);
    if (gmTime) {
        return (int)difftime(now, mktime(gmTime));
    }

    return 0;
}


#endif //__CTIMER_HPP__