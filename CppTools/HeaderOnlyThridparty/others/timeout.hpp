#ifndef __TIMEOUT_HPP__
#define __TIMEOUT_HPP__

#include <sys/time.h>
#include <unistd.h>

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

#endif  //__TIMEOUT_HPP__