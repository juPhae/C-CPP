/*******************************************************************************
 *FileName: Timer.hpp
 *version:  1.0
 *Author: phae
 *Date: 2022-06-10 11:38:14

 *Description:    Modify it on other people's blogs              --->
  https://www.cnblogs.com/gtarcoder/p/4924097.html

 *LastEditors: phae
 *LastEditTime: 2022-06-10 12:12:34
 ********************************************************************************/

                                    
#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string.h>
#include <thread>


class Timer {
public:
  Timer() : expired_(true), try_to_expire_(false) {}

  Timer(const Timer &t) {

    expired_ = t.expired_.load();

    try_to_expire_ = t.try_to_expire_.load();
  }

  ~Timer() { Expire(); }

  void StartTimer(int interval, std::function<void()> task) {

    if (expired_ == false) {

      return;
    }

    expired_ = false;

    std::thread([this, interval, task]() {
      while (!try_to_expire_) {

        std::this_thread::sleep_for(std::chrono::milliseconds(interval));

        task();
      }

      {

        std::lock_guard<std::mutex> locker(mutex_);

        expired_ = true;

        expired_cond_.notify_one();
      }
    })
        .detach();
  }

  void Expire() {

    if (expired_) {

      return;
    }

    if (try_to_expire_) {

      return;
    }

    try_to_expire_ = true;

    {

      std::unique_lock<std::mutex> locker(mutex_);

      expired_cond_.wait(locker, [this] { return expired_ == true; });

      if (expired_ == true) {

        try_to_expire_ = false;
      }
    }
  }

  template <typename callable, class... arguments>
  void SyncWait(int after, callable &&f, arguments &&... args) {

    std::function<typename std::result_of<callable(arguments...)>::type()> task

        (std::bind(std::forward<callable>(f),
                   std::forward<arguments>(args)...));

    std::this_thread::sleep_for(std::chrono::milliseconds(after));

    task();
  }

  template <typename callable, class... arguments>
  void SyncWait(std::string target_time, callable &&f, arguments &&... args) {

    int after = CalculateTimeInterval(target_time);
    std::function<typename std::result_of<callable(arguments...)>::type()> task

        (std::bind(std::forward<callable>(f),
                   std::forward<arguments>(args)...));

    std::this_thread::sleep_for(std::chrono::milliseconds(after));

    task();
  }


  template <typename callable, class... arguments>
  void AsyncWait(int after, callable &&f, arguments &&... args) {

    std::function<typename std::result_of<callable(arguments...)>::type()> task

        (std::bind(std::forward<callable>(f),
                   std::forward<arguments>(args)...));

    std::thread([after, task]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(after));

      task();
    })
        .detach();
  }


template <typename callable, class... arguments>
  void AsyncWait(std::string target_time, callable &&f, arguments &&... args) {
    int after = CalculateTimeInterval(target_time);
    std::function<typename std::result_of<callable(arguments...)>::type()> task

        (std::bind(std::forward<callable>(f),
                   std::forward<arguments>(args)...));

    std::thread([after, task]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(after));

      task();
    })
        .detach();
  }


private:
  std::atomic<bool> expired_;

  std::atomic<bool> try_to_expire_;

  std::mutex mutex_;

  std::condition_variable expired_cond_;

#if __SIZEOF_POINTER__ == 4
  // 32位系统

#define ALT_E 0x01
#define ALT_O 0x02
#define LEGAL_ALT(x)                                                           \
  {                                                                            \
    if (alt_format & ~(x))                                                     \
      return (0);                                                              \
  }
#define TM_YEAR_BASE 1900

  typedef struct bbtTM {
    // int tm_sec;  /* 秒 – 取值区间为[0,59] */
    // int tm_min;  /* 分 - 取值区间为[0,59] */
    // int tm_hour; /* 时 - 取值区间为[0,23] */
    // int tm_mday; /* 一个月中的日期 - 取值区间为[1,31] */
    // int tm_mon; /* 月份（从一月开始，0代表一月） - 取值区间为[0,11] */
    // int tm_year; /* 年份，其值等于实际年份 */
    int tm_sec;   /* 秒，范围从 0 到 59                */
    int tm_min;   /* 分，范围从 0 到 59                */
    int tm_hour;  /* 小时，范围从 0 到 23                */
    int tm_mday;  /* 一月中的第几天，范围从 1 到 31     */
    int tm_mon;   /* 月份，范围从 0 到 11                */
    int tm_year;  /* 自 1900 起的年数                */
    int tm_wday;  /* 一周中的第几天，范围从 0 到 6   */
    int tm_yday;  /* 一年中的第几天，范围从 0 到 365  */
    int tm_isdst; /* 夏令时                        */
    // } bbtTM_S;
  } bbtTM_S;

  const char Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  const uint32_t mon_yday[2][12] = {
      {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
      {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
  };

  // 时间戳转年月日时分秒
  void bbt_localtime(uint32_t time, bbtTM_S *t) {
    uint32_t Pass4year;
    int hours_per_year;

    //取秒时间
    t->tm_sec = (int)(time % 60);
    time /= 60;
    //取分钟时间
    t->tm_min = (int)(time % 60);
    time /= 60;
    //取过去多少个四年，每四年有 1461*24 小时
    Pass4year = time / (1461L * 24L);
    //计算年份
    t->tm_year = (Pass4year << 2) + 1970;
    //四年中剩下的小时数
    time %= 1461L * 24L;
    //校正闰年影响的年份，计算一年中剩下的小时数
    while (1) {
      //一年的小时数
      hours_per_year = 365 * 24;
      //判断闰年，是闰年，一年则多24小时，即一天
      if ((t->tm_year & 3) == 0)
        hours_per_year += 24;

      if (time < hours_per_year)
        break;

      t->tm_year++;
      time -= hours_per_year;
    }
    //小时数
    t->tm_hour = (int)(time % 24);
    //一年中剩下的天数
    time /= 24;
    //假定为闰年
    time++;
    //校正闰年的误差，计算月份，日期
    if ((t->tm_year & 3) == 0) {
      if (time > 60) {
        time--;
      } else {
        if (time == 60) {
          t->tm_mon = 1;
          t->tm_mday = 29;
          return;
        }
      }
    }
    //计算月日
    for (t->tm_mon = 0; Days[t->tm_mon] < time; t->tm_mon++) {
      time -= Days[t->tm_mon];
    }

    t->tm_mday = (int)(time);

    return;
  }

  int bbtIsLeap(int year) {
    return (year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0);
  }

  // 年月日时分秒转时间戳
  uint32_t bbt_mktime(bbtTM_S *dt) {
    uint32_t ret;
    int i = 0;
    // 以平年时间计算的秒数
    ret = (dt->tm_year - 1970) * 365 * 24 * 3600;
    ret += (mon_yday[bbtIsLeap(dt->tm_year)][dt->tm_mon] + dt->tm_mday - 1) *
           24 * 3600;
    ret += dt->tm_hour * 3600 + dt->tm_min * 60 + dt->tm_sec;
    // 加上闰年的秒数
    for (i = 1970; i < dt->tm_year; i++) {
      if (bbtIsLeap(i)) {
        ret += 24 * 3600;
      }
    }
    if (ret > 4107715199) { // 2100-02-29 23:59:59
      ret += 24 * 3600;
    }
    return (ret);
  }

  const char *day[7] = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                        "Thursday", "Friday", "Saturday"};
  const char *abday[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  const char *mon[12] = {"January",   "February", "March",    "April",
                         "May",       "June",     "July",     "August",
                         "September", "October",  "November", "December"};
  const char *abmon[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  const char *am_pm[2] = {"AM", "PM"};

  // char const *day[7] = {"Sunday",   "Monday", "Tuesday", "Wednesday",
  //                 "Thursday", "Friday", "Saturday"};
  // char const *abday[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  // char const *mon[12] = {"January",   "February", "March",    "April",
  //                  "May",       "June",     "July",     "August",
  //                  "September", "October",  "November", "December"};
  // char const *abmon[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
  //                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  // char const *am_pm[2] = {"AM", "PM"};
  int conv_num(const char **buf, int *dest, int llim, int ulim) {
    int result = 0;

    /* The limit also determines the number of valid digits. */
    int rulim = ulim;

    if (**buf < '0' || **buf > '9')
      return (0);

    do {
      result *= 10;
      result += *(*buf)++ - '0';
      rulim /= 10;
    } while ((result * 10 <= ulim) && rulim && **buf >= '0' && **buf <= '9');

    if (result < llim || result > ulim)
      return (0);

    *dest = result;
    return (1);
  }

  char *mystrptime(const char *buf, const char *fmt, struct bbtTM *tm) {
    char c;
    const char *bp;
    size_t len = 0;
    int alt_format, i, split_year = 0;

    bp = buf;

    while ((c = *fmt) != '\0') {
      /* Clear `alternate' modifier prior to new conversion. */
      alt_format = 0;

      /* Eat up white-space. */
      if (isspace(c)) {
        while (isspace(*bp))
          bp++;

        fmt++;
        continue;
      }

      if ((c = *fmt++) != '%')
        goto literal;

    again:
      switch (c = *fmt++) {
      case '%': /* "%%" is converted to "%". */
      literal:
        if (c != *bp++)
          return (0);
        break;

      /*
       * "Alternative" modifiers. Just set the appropriate flag
       * and start over again.
       */
      case 'E': /* "%E?" alternative conversion modifier. */
        LEGAL_ALT(0);
        alt_format |= ALT_E;
        goto again;

      case 'O': /* "%O?" alternative conversion modifier. */
        LEGAL_ALT(0);
        alt_format |= ALT_O;
        goto again;

      /*
       * "Complex" conversion rules, implemented through recursion.
       */
      case 'c': /* Date and time, using the locale's format. */
        LEGAL_ALT(ALT_E);
        if (!(bp = mystrptime(bp, "%x %X", tm)))
          return (0);
        break;

      case 'D': /* The date as "%m/%d/%y". */
        LEGAL_ALT(0);
        if (!(bp = mystrptime(bp, "%m/%d/%y", tm)))
          return (0);
        break;

      case 'R': /* The time as "%H:%M". */
        LEGAL_ALT(0);
        if (!(bp = mystrptime(bp, "%H:%M", tm)))
          return (0);
        break;

      case 'r': /* The time in 12-hour clock representation. */
        LEGAL_ALT(0);
        if (!(bp = mystrptime(bp, "%I:%M:%S %p", tm)))
          return (0);
        break;

      case 'T': /* The time as "%H:%M:%S". */
        LEGAL_ALT(0);
        if (!(bp = mystrptime(bp, "%H:%M:%S", tm)))
          return (0);
        break;

      case 'X': /* The time, using the locale's format. */
        LEGAL_ALT(ALT_E);
        if (!(bp = mystrptime(bp, "%H:%M:%S", tm)))
          return (0);
        break;

      case 'x': /* The date, using the locale's format. */
        LEGAL_ALT(ALT_E);
        if (!(bp = mystrptime(bp, "%m/%d/%y", tm)))
          return (0);
        break;

      /*
       * "Elementary" conversion rules.
       */
      case 'A': /* The day of week, using the locale's form. */
      case 'a':
        LEGAL_ALT(0);
        for (i = 0; i < 7; i++) {
          /* Full name. */
          len = strlen(day[i]);
          if (strncasecmp(day[i], bp, len) == 0)
            break;

          /* Abbreviated name. */
          len = strlen(abday[i]);
          if (strncasecmp(abday[i], bp, len) == 0)
            break;
        }

        /* Nothing matched. */
        if (i == 7)
          return (0);

        tm->tm_wday = i;
        bp += len;
        break;

      case 'B': /* The month, using the locale's form. */
      case 'b':
      case 'h':
        LEGAL_ALT(0);
        for (i = 0; i < 12; i++) {
          /* Full name. */
          len = strlen(mon[i]);
          if (strncasecmp(mon[i], bp, len) == 0)
            break;

          /* Abbreviated name. */
          len = strlen(abmon[i]);
          if (strncasecmp(abmon[i], bp, len) == 0)
            break;
        }

        /* Nothing matched. */
        if (i == 12)
          return (0);

        tm->tm_mon = i;
        bp += len;
        break;

      case 'C': /* The century number. */
        LEGAL_ALT(ALT_E);
        if (!(conv_num(&bp, &i, 0, 99)))
          return (0);

        if (split_year) {
          tm->tm_year = (tm->tm_year % 100) + (i * 100);
        } else {
          tm->tm_year = i * 100;
          split_year = 1;
        }
        break;

      case 'd': /* The day of month. */
      case 'e':
        LEGAL_ALT(ALT_O);
        if (!(conv_num(&bp, &tm->tm_mday, 1, 31)))
          return (0);
        break;

      case 'k': /* The hour (24-hour clock representation). */
        LEGAL_ALT(0);
        /* FALLTHROUGH */
      case 'H':
        LEGAL_ALT(ALT_O);
        if (!(conv_num(&bp, &tm->tm_hour, 0, 23)))
          return (0);
        break;

      case 'l': /* The hour (12-hour clock representation). */
        LEGAL_ALT(0);
        /* FALLTHROUGH */
      case 'I':
        LEGAL_ALT(ALT_O);
        if (!(conv_num(&bp, &tm->tm_hour, 1, 12)))
          return (0);
        if (tm->tm_hour == 12)
          tm->tm_hour = 0;
        break;

      case 'j': /* The day of year. */
        LEGAL_ALT(0);
        if (!(conv_num(&bp, &i, 1, 366)))
          return (0);
        tm->tm_yday = i - 1;
        break;

      case 'M': /* The minute. */
        LEGAL_ALT(ALT_O);
        if (!(conv_num(&bp, &tm->tm_min, 0, 59)))
          return (0);
        break;

      case 'm': /* The month. */
        LEGAL_ALT(ALT_O);
        if (!(conv_num(&bp, &i, 1, 12)))
          return (0);
        tm->tm_mon = i - 1;
        break;

      case 'p': /* The locale's equivalent of AM/PM. */
        LEGAL_ALT(0);
        /* AM? */
        if (strcasecmp(am_pm[0], bp) == 0) {
          if (tm->tm_hour > 11)
            return (0);

          bp += strlen(am_pm[0]);
          break;
        }
        /* PM? */
        else if (strcasecmp(am_pm[1], bp) == 0) {
          if (tm->tm_hour > 11)
            return (0);

          tm->tm_hour += 12;
          bp += strlen(am_pm[1]);
          break;
        }

        /* Nothing matched. */
        return (0);

      case 'S': /* The seconds. */
        LEGAL_ALT(ALT_O);
        if (!(conv_num(&bp, &tm->tm_sec, 0, 61)))
          return (0);
        break;

      case 'U': /* The week of year, beginning on sunday. */
      case 'W': /* The week of year, beginning on monday. */
        LEGAL_ALT(ALT_O);
        /*
         * XXX This is bogus, as we can not assume any valid
         * information present in the tm structure at this
         * point to calculate a real value, so just check the
         * range for now.
         */
        if (!(conv_num(&bp, &i, 0, 53)))
          return (0);
        break;

      case 'w': /* The day of week, beginning on sunday. */
        LEGAL_ALT(ALT_O);
        if (!(conv_num(&bp, &tm->tm_wday, 0, 6)))
          return (0);
        break;

      case 'Y': /* The year. */
        LEGAL_ALT(ALT_E);
        if (!(conv_num(&bp, &i, 0, 9999)))
          return (0);

        tm->tm_year = i - TM_YEAR_BASE;
        break;

      case 'y': /* The year within 100 years of the epoch. */
        LEGAL_ALT(ALT_E | ALT_O);
        if (!(conv_num(&bp, &i, 0, 99)))
          return (0);

        if (split_year) {
          tm->tm_year = ((tm->tm_year / 100) * 100) + i;
          break;
        }
        split_year = 1;
        if (i <= 68)
          tm->tm_year = i + 2000 - TM_YEAR_BASE;
        else
          tm->tm_year = i + 1900 - TM_YEAR_BASE;
        break;

      /*
       * Miscellaneous conversions.
       */
      case 'n': /* Any kind of white-space. */
      case 't':
        LEGAL_ALT(0);
        while (isspace(*bp))
          bp++;
        break;

      default: /* Unknown/unsupported conversion. */
        return (0);
      }
    }

    /* LINTED functional specification */
    return ((char *)bp);
  }

  long long StringToTime(std::string str) {

    int length = str.length();

    struct bbtTM tm;
    // if (length == 5) {
    //   mystrptime(str.c_str(), "%H:%M", &tm); //
    // } else if (length == 8) {
    //   mystrptime(str.c_str(), "%H:%M:%S", &tm); //
    // } else if (length == 10) {
    //   mystrptime(str.c_str(), "%Y-%m-%d", &tm); //
    // } else if (length == 16) {
    //   mystrptime(str.c_str(), "%Y-%m-%d %H:%M", &tm); //
    // } else if (length == 19) {
    //   mystrptime(str.c_str(), "%Y-%m-%d %H:%M:%S", &tm); //
    // }
    //返回值是计算1970年1月1日到现在的时间，没有年月日则计算出现偏差
    if (length == 10) {
      mystrptime(str.c_str(), "%Y-%m-%d", &tm); //
    } else if (length == 16) {
      mystrptime(str.c_str(), "%Y-%m-%d %H:%M", &tm); //
    } else if (length == 19) {
      mystrptime(str.c_str(), "%Y-%m-%d %H:%M:%S", &tm); //
    }
    return bbt_mktime(&tm);
  }

  std::string GetNowTime(int length) {

    time_t timep;
    time(&timep);
    char tmp[128];
    if (length == 5) {
      strftime(tmp, sizeof(tmp), "%H:%M", localtime(&timep));
    } else if (length == 8) {
      strftime(tmp, sizeof(tmp), "%H:%M:%S", localtime(&timep)); //
    } else if (length == 10) {
      strftime(tmp, sizeof(tmp), "%Y-%m-%d", localtime(&timep));
    } else if (length == 16) {
      strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M", localtime(&timep)); //
    } else if (length == 19) {
      strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    }

    return tmp;
  }

#elif __SIZEOF_POINTER__ == 8
  // 64位系统

  std::string GetNowTime(int length) {
    time_t timep;
    time(&timep);
    char tmp[64];
    if (length == 5) {
      strftime(tmp, sizeof(tmp), "%H:%M", localtime(&timep));
    } else if (length == 8) {
      strftime(tmp, sizeof(tmp), "%H:%M:%S", localtime(&timep)); //
    } else if (length == 10) {
      strftime(tmp, sizeof(tmp), "%Y-%m-%d", localtime(&timep));
    } else if (length == 16) {
      strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M", localtime(&timep)); //
    } else if (length == 19) {
      strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    }

    return tmp;
  }
  //返回值单位是秒
  long StringToTime(std::string str) {
    int length = str.length();
    struct tm tm;
    //返回值是计算1970年1月1日到现在的时间，没有年月日则计算出现偏差
    if (length == 10) {
      strptime(str.c_str(), "%Y-%m-%d", &tm); //
    } else if (length == 16) {
      strptime(str.c_str(), "%Y-%m-%d %H:%M", &tm); //
    } else if (length == 19) {
      strptime(str.c_str(), "%Y-%m-%d %H:%M:%S", &tm); //
    }

    return mktime(&tm);
  }

#endif   //__SIZEOF_POINTER__

  long CalculateTimeInterval(std::string targetTime) {
    std::string nTime = GetNowTime(targetTime.length()); //获取现在的时间
    // int msTime = 0;

    if ((targetTime.length() == nTime.length()) && (targetTime.length() < 10)) {
      int compare_time = strcmp(targetTime.c_str(), nTime.c_str());
      if (compare_time > 0) {
        targetTime = "1970-01-01 " + targetTime;
        nTime = "1970-01-01 " + nTime;
        //当天，还没到
        //填充日期年月日

      } else if (compare_time < 0) {
        //次日
        //填充日期年月日
        targetTime = "1970-01-02 " + targetTime;
        nTime = "1970-01-01 " + nTime;

      } else if (compare_time == 0) {
        //此刻
        targetTime = "1970-01-01 " + targetTime;
        nTime = "1970-01-01 " + nTime;
      }
    }

    long obj_time = StringToTime(targetTime);
    long now_time = StringToTime(nTime);
    long interval_sec = abs(obj_time - now_time); //计算时间间隔差，精度为秒
    long msTime = interval_sec * 1000;            //转成毫秒
    return msTime;
  }
};

#endif  //__TIMER_HPP__


#if 0
/****************************************************************************
 * 
 * 
 * 
 *
 
////////////////////test.cpp
#include<iostream>
#include<string>
#include<memory>
#include"Timer.hpp"
using namespace std;
void EchoFunc(std::string&& s){
    std::cout << "test : " << s << endl;
}
 
int main(){
    Timer t;
    //周期性执行定时任务
    t.StartTimer(1000, std::bind(EchoFunc,"hello world!"));
    std::this_thread::sleep_for(std::chrono::seconds(4));
    std::cout << "try to expire timer!" << std::endl;
    t.Expire();
 
    //周期性执行定时任务
    t.StartTimer(1000, std::bind(EchoFunc,  "hello c++11!"));
    std::this_thread::sleep_for(std::chrono::seconds(4));
    std::cout << "try to expire timer!" << std::endl;
    t.Expire();
 
    std::this_thread::sleep_for(std::chrono::seconds(2));
 
    //只执行一次定时任务
    //同步
    t.SyncWait(1000, EchoFunc, "hello world!");
    //异步
    t.AsyncWait(1000, EchoFunc, "hello c++11!");
 
    std::this_thread::sleep_for(std::chrono::seconds(2));
 
    return 0;
}

 * 
 * 
 * 
 * **************************************************************************/

#endif 