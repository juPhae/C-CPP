#ifndef __TIMEOUT_HPP__
#define __TIMEOUT_HPP__

#include <ctime> // std::time_t, std::tm, std::localtime, std::mktime
#include <functional>
#include <sys/time.h>
#include <thread>
#include <unistd.h>
#include <vector>

class CTimeOut {
public:
  int set_timeout_msec(int msec) {
    if (msec <= 0) {
      return 1;
    }

    gettimeofday(&timeout_set, NULL);

    // 分解设置的毫秒数
    long usec2add = (msec & 1000) * 1000;
    int sec2add = (msec / 1000);

    // 算出加上超时时间后的微秒数和秒数
    long real_usec = (timeout_set.tv_usec + usec2add) % 1000000;
    int real_sec = ((timeout_set.tv_usec + usec2add) / 1000000) + sec2add +
                   timeout_set.tv_sec;

    // 设置超时的时间
    timeout_set.tv_sec = real_sec;
    timeout_set.tv_usec = real_usec;

    return 0;
  };
  int set_timeout_sec(int sec) {
    if (sec <= 0) {
      return 1;
    }

    gettimeofday(&timeout_set, NULL);

    timeout_set.tv_sec += sec;

    return 0;
  };
  bool is_timeout() {
    gettimeofday(&timeout_now, NULL);
    return timeout_now.tv_sec >= timeout_set.tv_sec &&
           timeout_now.tv_usec >= timeout_set.tv_usec;
  };

private:
  timeval timeout_set;
  timeval timeout_now;
};

class Timeout : public CTimeOut {
  bool run_thread = true;
  std::thread timerThread;
  std::vector<std::pair<std::time_t, std::function<void()>>> lambdas;

  void threadCore() {
    while (run_thread) {
      std::time_t now = std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now());

      for (int i = 0; i < lambdas.size(); i++) {
        int x = lambdas.at(i).first - now;
        if (x <= 0) {
          lambdas.at(i).second();             // 执行 lambda
          lambdas.erase(lambdas.begin() + i); // 删除 lambda
          i--;
        }
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }

public:
  //开启一个线程，等待lambdas超时并执行它们
  Timeout() {
    timerThread = std::thread([this] { this->threadCore(); });
  };

  ~Timeout() {
    run_thread = false;
    if (timerThread.joinable())
      timerThread.join();
  };

  //等待lambdas执行
  void wait() {
    while (lambdas.size() > 0) {
    }
  }

  //重载
  void operator()(std::function<void()> lambda, int seconds) {
    std::time_t now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) +
        seconds;
    lambdas.push_back(std::make_pair(now, lambda));
  }
};

/**********************************************************************
*
*

class CTimeOut:
-------------------------------------------------------------------------------------

CTimeOut timeout;
  timeout.set_timeout_msec(50);

  int i = 0;
  while (1) {
    i++;
    std::cout << i << std::endl;
    if (timeout.is_timeout()) {
     return;
    }
  }

--------------------------------------------------------------------------------------


class Timeout:
-----------------------------------------------------------------------------------
 Timeout Timeout;
   Timeout ([]()
    {
    std::cout << "Function is executed with 3 seconds delay"<<std::endl;
    }, 3);
------------------------------------------------------------------------------------
*
**********************************************************************/

#endif //__TIMEOUT_HPP__