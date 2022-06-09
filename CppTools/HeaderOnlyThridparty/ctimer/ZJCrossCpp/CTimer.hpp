//
//  CTimer.hpp
//  ZJCrossCpp
//
//  Created by eafy on 2020/10/5.
//  Copyright © 2020 ZJ. All rights reserved.
//

/*
https://github.com/Eafy/ZJCrossCpp/
*/

#ifndef __CTIMER_HPP__
#define __CTIMER_HPP__

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>
#include <thread>
#include <time.h>
#include <vector>

#if defined(__ANDROID__) || defined(ANDROID) // Android
#define IS_ANDROID_PLATFORM 1
#define IS_IOS_PLATFORM 0
#elif defined(__APPLE__) // iOS
#define IS_ANDROID_PLATFORM 0
#define IS_IOS_PLATFORM 1
#else
#define IS_ANDROID_PLATFORM 0
#define IS_IOS_PLATFORM 0
#endif

// ZJ_NAMESPACE_BEGIN

class CTimer {
public:
  CTimer(const std::string sTimerName = ""); //构造定时器，附带名称
  ~CTimer();

  /**
   开始运行定时器

   @param msTime 延迟运行(单位ms)
   @param task 任务函数接口
   @param bLoop 是否循环(默认执行1次)
   @param async 是否异步(默认异步)
   @return true:已准备执行，否则失败
   */
  bool Start(unsigned int msTime, std::function<void()> task,
             bool bLoop = false, bool async = true);

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
  template <typename callable, typename... arguments>
  bool SyncOnce(int msTime, callable &&fun, arguments &&... args) {
    std::function<typename std::result_of<callable(arguments...)>::type()> task(
        std::bind(
            std::forward<callable>(fun),
            std::forward<arguments>(args)...)); //绑定任务函数或lambda成function
    return Start(msTime, task, false, false);
  }

  /**
   异步执行一次任务

   @param msTime 延迟及间隔时间（毫秒）
   @param fun 函数接口或lambda代码块
   @param args 参数
   @return true:已准备执行，否则失败
   */
  template <typename callable, typename... arguments>
  bool AsyncOnce(int msTime, callable &&fun, arguments &&... args) {
    std::function<typename std::result_of<callable(arguments...)>::type()> task(
        std::bind(std::forward<callable>(fun),
                  std::forward<arguments>(args)...));

    return Start(msTime, task, false);
  }

  /**
   异步执行一次任务(默认延迟10毫秒后执行)

   @param fun 函数接口或lambda代码块
   @param args 参数
   @return true:已准备执行，否则失败
   */
  template <typename callable, typename... arguments>
  bool AsyncOnce(callable &&fun, arguments &&... args) {
    std::function<typename std::result_of<callable(arguments...)>::type()> task(
        std::bind(std::forward<callable>(fun),
                  std::forward<arguments>(args)...));

    return Start(1, task, false);
  }

  /**
 异步执行一次任务，到达目标时间点执行。
注意，更改系统时间需要重新执行该函数
 @param targetTime 目标时间
 @param fun 函数接口或lambda代码块
 @param args 参数
 @return true:已准备执行，否则失败
 */
  template <typename callable, typename... arguments>
  bool AsyncOnce(std::string targetTime, callable &&fun, arguments &&... args) {
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
      }
    }
    long obj_time = StringToTime(targetTime);
    long now_time = StringToTime(nTime);
    long interval_sec = abs(obj_time - now_time); //计算时间间隔差，精度为秒

    std::function<typename std::result_of<callable(arguments...)>::type()> task(
        std::bind(std::forward<callable>(fun),
                  std::forward<arguments>(args)...));

    int msTime = interval_sec * 1000; //转成毫秒

    return Start(msTime, task, false);
  }

  /**
   异步循环执行任务

   @param msTime 延迟及间隔时间（毫秒）
   @param fun 函数接口或lambda代码块
   @param args 参数
   @return true:已准备执行，否则失败
   */
  template <typename callable, typename... arguments>
  bool AsyncLoop(int msTime, callable &&fun, arguments &&... args) {
    std::function<typename std::result_of<callable(arguments...)>::type()> task(
        std::bind(std::forward<callable>(fun),
                  std::forward<arguments>(args)...));

    return Start(msTime, task, true);
  }

#if 0
  /**
 异步循环执行任务，每天到达目标时间点执行。
注意，更改系统时间需要重新执行该函数
 @param targetTime 目标时间
 @param fun 函数接口或lambda代码块
 @param args 参数
 @return true:已准备执行，否则失败
 */
  template <typename callable, typename... arguments>
  bool AsyncLoop(std::string targetTime, callable &&fun, arguments &&... args) {

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

      }
    }

    long obj_time = StringToTime(targetTime);
    long now_time = StringToTime(nTime);
    long interval_sec = abs(obj_time - now_time); //计算时间间隔差，精度为秒

    std::function<typename std::result_of<callable(arguments...)>::type()> task(
        std::bind(std::forward<callable>(fun),
                  std::forward<arguments>(args)...));


    int msTime = interval_sec * 1000; //转成毫秒
 
    return Start(msTime, task, true);
  }
#endif
public:
  /// 获取时间戳(毫秒)
  static uint64_t Timestamp();

  /// 获取格式化时间
  static std::string
  FormatTime(const std::string sFormat = "%Y-%m-%d %H:%M:%S");

  /// 获取UTC时间
  static struct tm *UTCTime(long long secTime = 0);

  /// 获取UTC时间(秒)
  static int64_t UTCTime();

  /// 获取与0时区的时差（以秒为单位）
  static int TimeDifFrimGMT();

private:
  void DeleteThread(); //删除任务线程
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

  // static const char *day[7] = {
  //     "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
  //     "Friday", "Saturday"
  // };
  // static const char *abday[7] = {
  //     "Sun","Mon","Tue","Wed","Thu","Fri","Sat"
  // };
  // static const char *mon[12] = {
  //     "January", "February", "March", "April", "May", "June", "July",
  //     "August", "September", "October", "November", "December"
  // };
  // static const char *abmon[12] = {
  //     "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  //     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  // };
  // static const char *am_pm[2] = {
  //     "AM", "PM"
  // };

  char *day[7] = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                  "Thursday", "Friday", "Saturday"};
  char *abday[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  char *mon[12] = {"January",   "February", "March",    "April",
                   "May",       "June",     "July",     "August",
                   "September", "October",  "November", "December"};
  char *abmon[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  char *am_pm[2] = {"AM", "PM"};
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

#endif

public:
  int m_nCount = 0; //循环次数
  int m_nTag = 0;   //定时器标签

private:
  std::string m_sName; //定时器名称

  std::atomic_bool m_bExpired;    //装载的任务是否已经过期
  std::atomic_bool m_bTryExpired; //装备让已装载的任务过期(标记)
  std::atomic_bool m_bLoop;       //是否循环

  std::thread *m_Thread = nullptr;
  std::mutex m_ThreadLock;
  std::condition_variable_any m_ThreadCon;

}; // class CTimer

CTimer::CTimer(const std::string sTimerName)
    : m_bExpired(true), m_bTryExpired(false), m_bLoop(false) {
  m_sName = sTimerName;
}

CTimer::~CTimer() {
  m_bTryExpired = true; //尝试使任务过期
  DeleteThread();
}

bool CTimer::Start(unsigned int msTime, std::function<void()> task, bool bLoop,
                   bool async) {
  if (!m_bExpired || m_bTryExpired)
    return false; //任务未过期(即内部仍在存在或正在运行任务)
  m_bExpired = false;
  m_bLoop = bLoop;
  m_nCount = 0;

  if (async) {
    DeleteThread();
    m_Thread = new std::thread([this, msTime, task]() {
      if (!m_sName.empty()) {
#if (defined(__ANDROID__) || defined(ANDROID)) //兼容Android
        pthread_setname_np(pthread_self(), m_sName.c_str());
#elif defined(__APPLE__) //兼容苹果系统
        pthread_setname_np(m_sName.c_str()); //设置线程(定时器)名称
#endif
      }

      while (!m_bTryExpired) {
        m_ThreadCon.wait_for(m_ThreadLock,
                             std::chrono::milliseconds(msTime)); //休眠
        if (!m_bTryExpired) {
          task(); //执行任务

          m_nCount++;
          if (!m_bLoop) {
            break;
          }
        }
      }

      m_bExpired = true;     //任务执行完成(表示已有任务已过期)
      m_bTryExpired = false; //为了下次再次装载任务
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

void CTimer::Cancel() {
  if (m_bExpired || m_bTryExpired || !m_Thread) {
    return;
  }

  m_bTryExpired = true;
}

void CTimer::DeleteThread() {
  if (m_Thread) {
    m_ThreadCon.notify_all(); //休眠唤醒
    m_Thread->join();         //等待线程退出
    delete m_Thread;
    m_Thread = nullptr;
  }
}

#pragma mark -

uint64_t CTimer::Timestamp() {
  uint64_t msTime = 0;

#if defined(__APPLE__) // iOS
  if (__builtin_available(iOS 10.0, *)) {
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);
    msTime = ((uint64_t)abstime.tv_sec) * 1000 +
             ((uint64_t)abstime.tv_nsec) / 1000000;
  } else {
    struct timeval abstime;
    gettimeofday(&abstime, NULL);
    msTime =
        ((uint64_t)abstime.tv_sec) * 1000 + ((uint64_t)abstime.tv_usec) / 1000;
  }
#else
  struct timespec abstime;
  clock_gettime(CLOCK_REALTIME, &abstime);

  msTime = ((uint64_t)abstime.tv_sec) * 1000 +
           ((uint64_t)abstime.tv_nsec) / 1000000; //需要强制转long long
#endif

  return msTime;
}

std::string CTimer::FormatTime(const std::string sFormat) {
  time_t timep;
  time(&timep);

  char tmp[64];
  strftime(tmp, sizeof(tmp), sFormat.c_str(), localtime(&timep));

  return std::string(tmp);
}

struct tm *CTimer::UTCTime(long long secTime) {
  time_t timep;
  if (secTime) {
    timep = secTime;
  } else {
    time(&timep);
  }

  struct tm *data = gmtime(&timep);
  data->tm_year += 1900;
  data->tm_mon += 1;

  return data;
}

int64_t CTimer::UTCTime() {
  int64_t msTime = 0;

#if defined(__APPLE__) // iOS
  if (__builtin_available(iOS 10.0, *)) {
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);
    msTime =
        ((int64_t)abstime.tv_sec) * 1000 + ((int64_t)abstime.tv_nsec) / 1000000;
  } else {
    struct timeval abstime;
    gettimeofday(&abstime, NULL);
    msTime =
        ((int64_t)abstime.tv_sec) * 1000 + ((int64_t)abstime.tv_usec) / 1000;
  }
#else
  struct timespec abstime;
  clock_gettime(CLOCK_REALTIME, &abstime);

  msTime = (int64_t)abstime.tv_sec;
#endif

  return msTime;
}

int CTimer::TimeDifFrimGMT() {
  time_t now = time(NULL);
  struct tm *gmTime = gmtime(&now);
  if (gmTime) {
    return (int)difftime(now, mktime(gmTime));
  }

  return 0;
}

// ZJ_NAMESPACE_END
#endif //__CTIMER_HPP__