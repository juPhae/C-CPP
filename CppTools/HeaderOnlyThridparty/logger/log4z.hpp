/*
 * Log4z License
 * -----------
 * 
 * Log4z is licensed under the terms of the MIT license reproduced below.
 * This means that Log4z is free software and can be used for both academic
 * and commercial purposes at absolutely no cost.
 * 
 * 
 * ===============================================================================
 * 
 * Copyright (C) 2010-2017 YaweiZhang <yawei.zhang@foxmail.com>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * ===============================================================================
 * 
 * (end of COPYRIGHT)
 */


/*
 * AUTHORS:  YaweiZhang <yawei.zhang@foxmail.com>
 * VERSION:  3.5.0
 * PURPOSE:  A lightweight library for error reporting and logging to file and screen .
 * CREATION: 2010.10.4
 * LCHANGE:  2017.08.20
 * LICENSE:  Expat/MIT License, See Copyright Notice at the begin of this file.
 */


/*
 * contact me:
 * tencent qq group: 19811947
 * mail: yawei.zhang@foxmail.com
 */


/* 
 * UPDATES LOG
 * 
 * VERSION 0.1.0 <DATE: 2010.10.4>
 *     create the first project.  
 *     It support put log to screen and files, 
 *     support log level, support one day one log file.
 *     support multi-thread, cross-platform.
 * 
 * VERSION .... <DATE: ...>
 *     ...
 * 
 * VERSION 0.9.0 <DATE: 2012.12.24>
 *     support config files.
 *     support color text in screen.
 *     support multiple output to different files.
 * 
 * VERSION 1.0.0 <DATE: 2012.12.29>
 *     support comments in the config file.
 *     add a advanced demo in the ./project
 *     fix some details.
 * 
 * VERSION 1.0.1 <DATE: 2013.01.01>
 *     change and add some Comments in the log4z
 *     simplify the 'fast_test' demo projects.
 * 
 * VERSION 1.1.0 <DATE: 2013.01.24>
 *     the method Start will wait for the logger thread started.
 *     config and add method change. 
 *     namespace change.
 * 
 * VERSION 1.1.1 <DATE: 2013.02.23>
 *     add status info method.
 *     optimize. 
 *
 * VERSION 1.2.0 <DATE: 2013.04.05>
 *     add stress test demo
 *  rewrite Stream module,better performance. 
 * 
 * VERSION 1.2.1 <DATE: 2013.04.13>
 *     fixed type name 'long' stream format on 64/32 operation system.
 *  logger will not loss any log on process normal exit.
 *
 * VERSION 2.0.0 <DATE: 2013.04.25>
 *     new interface: 
 *      merge some Main interface and Dynamic interface
 *      add Change Logger Attribute method by thread-safe
 *     new config design.
 *     log file name append process id.
 *
 * VERSION 2.1 <DATE: 2013.05.22>
 *     support binary text output 
 *  rewrite write file module, support vs2005 open Chinese characters path
 *
 * VERSION 2.2 <DATE: 2013.07.08>
 *    optimized binary stream output view
 *     support wchar * string.
 *  
 * VERSION 2.3 <DATE: 2013.08.29>
 *  adjust output file named.
 *  support different month different directory.
 *  adjust some detail.
 * 
 * VERSION 2.4 <DATE: 2013.10.07>
 *  support rolling log file.
 *  support hot update configure
 *  used precision time in log.
 *  micro set default logger attribute
 *  fix tls bug in windows xp dll
 *
 * VERSION 2.5 <DATE: 2014.03.25>
 *  screen output can choice synchronous or not
 *  fix sometimes color will disorder on windows.
 *  eliminate some compiler warning
 *  fix sem_timewait in linux
 *  add format-style method at input log, cannot support vs2003 and VC6.
 *  fix WCHAR String cannot output
 *  optimize std::string, binary log input, and support std::wstring.
 *  clean code, better readability
 *  
 * VERSION 2.6 <DATE: 2014.08.19>
 *  add PrePushLog 
 *  better performance when log is filter out.
 *  interface replace std::string because it's in shared library is unsafe.
 *  add log level 'trace'
 * 
 * VERSION 2.6.1 <DATE: 2014.08.22>
 *  fix bug from defined _MSC_VER 
 *
 * VERSION 2.7 <DATE: 2014.09.21>
 *  compatible mac machine,  now  log4z can working in linux/windows/mac.
 *
 * VERSION 2.8 <DATE: 2014.09.27>
 *  support synchronous written to file and thread-safe
 *  fix compatibility on MinGW. a constant value suffix.
 *  ignore utf-8 file BOM when load configure file
 *  use macro WIN32_LEAN_AND_MEAN replace head file winsock2.h
 *  new naming notations
 * 
 * VERSION 3.0 <DATE: 2014.12.19>
 *  new naming notations
 *  support for reading config from a string.
 *  remove all TLS code, used dispatch_semaphore in apple OS.
 *  support system: windows, linux, mac, iOS
 *
 * VERSION 3.1 <DATE: 2014.12.19>
 *  add method enable/disable logger by specific logger id
 *  add method enable/disable log suffix line number.
 *  add method enable/disable log output to file.
 *
 * VERSION 3.2 <DATE: 2014.12.19>
 *  add interface setLoggerName,setLoggerPath,setAutoUpdate
 *  support auto update from configure file
 *
 * VERSION 3.3 <DATE: 2014.12.19>
 *  support map vector list
 *  support continuum via travis.
 *  new hot change design, all change realize need via push log flow.
 *  support oem string convert.
 *  new method to read whole content of file.
 *  check configure's checksum when auto update it.
 *  some other optimize.
 * 
 * VERSION 3.5 <DATE: 2017.08.20>
 *  optimization
 *
 */


/*
https://github.com/zsummer/log4z/
*/

#pragma once
#ifndef _ZSUMMER_LOG4Z_H_
#define _ZSUMMER_LOG4Z_H_

#include <string>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <cmath>
#include <stdlib.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <vector>
#include <map>
#include <list>
#include <queue>
#include <deque>


//! logger ID type. DO NOT TOUCH
typedef int LoggerId;

//! the invalid logger id. DO NOT TOUCH
const int LOG4Z_INVALID_LOGGER_ID = -1;

//! the main logger id. DO NOT TOUCH
//! can use this id to set the main logger's attribute.
//! example:
//! ILog4zManager::getPtr()->setLoggerLevel(LOG4Z_MAIN_LOGGER_ID, LOG_LEVEL_WARN);
//! ILog4zManager::getPtr()->setLoggerDisplay(LOG4Z_MAIN_LOGGER_ID, false);
const int LOG4Z_MAIN_LOGGER_ID = 0;

//! the main logger name. DO NOT TOUCH
const char*const LOG4Z_MAIN_LOGGER_KEY = "Main";

//! check VC VERSION. DO NOT TOUCH
//! format micro cannot support VC6 or VS2003, please use stream input log, like LOGI, LOGD, LOG_DEBUG, LOG_STREAM ...
#if _MSC_VER >= 1400 //MSVC >= VS2005
#define LOG4Z_FORMAT_INPUT_ENABLE
#endif

#ifndef WIN32
#define LOG4Z_FORMAT_INPUT_ENABLE
#endif

//! LOG Level
enum ENUM_LOG_LEVEL
{
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_ALARM,
    LOG_LEVEL_FATAL,
};

//////////////////////////////////////////////////////////////////////////
//! -----------------default logger config, can change on this.-----------
//////////////////////////////////////////////////////////////////////////
//! the max logger count.
const int LOG4Z_LOGGER_MAX = 20;
//! the max log content length.
const int LOG4Z_LOG_BUF_SIZE = 1024 * 8;
//! the max stl container depth.
const int LOG4Z_LOG_CONTAINER_DEPTH = 5;
//! the log queue length limit size.
const int LOG4Z_LOG_QUEUE_LIMIT_SIZE = 20000;

//! all logger synchronous output or not
const bool LOG4Z_ALL_SYNCHRONOUS_OUTPUT = false;
//! all logger synchronous display to the windows debug output
const bool LOG4Z_ALL_DEBUGOUTPUT_DISPLAY = false;

//! default logger output file.
const char* const LOG4Z_DEFAULT_PATH = "./log/";
//! default log filter level
const int LOG4Z_DEFAULT_LEVEL = LOG_LEVEL_DEBUG;
//! default logger display
const bool LOG4Z_DEFAULT_DISPLAY = true;
//! default logger output to file
const bool LOG4Z_DEFAULT_OUTFILE = true;
//! default logger month dir used status
const bool LOG4Z_DEFAULT_MONTHDIR = false;
//! default logger output file limit size, unit M byte.
const int LOG4Z_DEFAULT_LIMITSIZE = 100;
//! default logger show suffix (file name and line number) 
const bool LOG4Z_DEFAULT_SHOWSUFFIX = true;
//! support ANSI->OEM console conversion on Windows
#undef LOG4Z_OEM_CONSOLE
//! default logger force reserve log file count.
const size_t LOG4Z_FORCE_RESERVE_FILE_COUNT = 7;

///////////////////////////////////////////////////////////////////////////
//! -----------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////

#ifndef _ZSUMMER_BEGIN
#define _ZSUMMER_BEGIN namespace zsummer {
#endif  
#ifndef _ZSUMMER_LOG4Z_BEGIN
#define _ZSUMMER_LOG4Z_BEGIN namespace log4z {
#endif
_ZSUMMER_BEGIN
_ZSUMMER_LOG4Z_BEGIN


struct LogData
{
    LoggerId _id;        //dest logger id
    int    _type;     //type.
    int    _typeval;
    int    _level;    //log level
    time_t _time;        //create time
    unsigned int _precise; //create time 
    unsigned int _threadID;
    int _contentLen;
    char _content[1]; //content
};

//! log4z class
class ILog4zManager
{
public:
    ILog4zManager(){};
    virtual ~ILog4zManager(){};

    //! Log4z Singleton
    
    static ILog4zManager * getInstance();
    inline static ILog4zManager & getRef(){return *getInstance();}
    inline static ILog4zManager * getPtr(){return getInstance();}

    //! Config or overwrite configure
    //! Needs to be called before ILog4zManager::Start,, OR Do not call.
    virtual bool config(const char * configPath) = 0;
    virtual bool configFromString(const char * configContent) = 0;

    //! Create or overwrite logger.
    //! Needs to be called before ILog4zManager::Start, OR Do not call.
    virtual LoggerId createLogger(const char* key) = 0;

    //! Start Log Thread. This method can only be called once by one process.
    virtual bool start() = 0;

    //! Default the method will be calling at process exit auto.
    //! Default no need to call and no recommended.
    virtual bool stop() = 0;

    //! Find logger. thread safe.
    virtual LoggerId findLogger(const char* key) =0;

    //pre-check the log filter. if filter out return false. 
    virtual bool prePushLog(LoggerId id, int level) = 0;
    //! Push log, thread safe.
    virtual bool pushLog(LogData * pLog, const char * file = NULL, int line = 0) = 0;

    //! set logger's attribute, thread safe.
    virtual bool enableLogger(LoggerId id, bool enable) = 0; // immediately when enable, and queue up when disable. 
    virtual bool setLoggerName(LoggerId id, const char * name) = 0;
    virtual bool setLoggerPath(LoggerId id, const char * path) = 0;
    virtual bool setLoggerLevel(LoggerId id, int nLevel) = 0; // immediately when enable, and queue up when disable. 
    virtual bool setLoggerFileLine(LoggerId id, bool enable) = 0;
    virtual bool setLoggerDisplay(LoggerId id, bool enable) = 0;
    virtual bool setLoggerOutFile(LoggerId id, bool enable) = 0;
    virtual bool setLoggerLimitsize(LoggerId id, unsigned int limitsize) = 0;
	virtual bool setLoggerMonthdir(LoggerId id, bool enable) = 0;
	virtual bool setLoggerReserveTime(LoggerId id, time_t sec) = 0;


    //! Update logger's attribute from config file, thread safe.
    virtual bool setAutoUpdate(int interval/*per second, 0 is disable auto update*/) = 0;
    virtual bool updateConfig() = 0;

    //! Log4z status statistics, thread safe.
    virtual bool isLoggerEnable(LoggerId id) = 0;
    virtual unsigned long long getStatusTotalWriteCount() = 0;
    virtual unsigned long long getStatusTotalWriteBytes() = 0;
    virtual unsigned long long getStatusTotalPushQueue() = 0;
    virtual unsigned long long getStatusTotalPopQueue() = 0;
    virtual unsigned int getStatusActiveLoggers() = 0;

    virtual LogData * makeLogData(LoggerId id, int level) = 0;
    virtual void freeLogData(LogData * log) = 0;
};

class Log4zStream;
class Log4zBinary;

#ifndef _ZSUMMER_END
#define _ZSUMMER_END }
#endif  
#ifndef _ZSUMMER_LOG4Z_END
#define _ZSUMMER_LOG4Z_END }
#endif

_ZSUMMER_LOG4Z_END
_ZSUMMER_END



//! base macro.
#define LOG_STREAM(id, level, file, line, log)\
do{\
    if (zsummer::log4z::ILog4zManager::getPtr()->prePushLog(id,level)) \
    {\
        zsummer::log4z::LogData * __pLog = zsummer::log4z::ILog4zManager::getPtr()->makeLogData(id, level); \
        zsummer::log4z::Log4zStream __ss(__pLog->_content + __pLog->_contentLen, LOG4Z_LOG_BUF_SIZE - __pLog->_contentLen);\
        __ss << log;\
        __pLog->_contentLen += __ss.getCurrentLen(); \
        zsummer::log4z::ILog4zManager::getPtr()->pushLog(__pLog, file, line);\
    }\
} while (0)


//! fast macro
#define LOG_TRACE(id, log) LOG_STREAM(id, LOG_LEVEL_TRACE, __FILE__, __LINE__, log)
#define LOG_DEBUG(id, log) LOG_STREAM(id, LOG_LEVEL_DEBUG, __FILE__, __LINE__, log)
#define LOG_INFO(id, log)  LOG_STREAM(id, LOG_LEVEL_INFO, __FILE__, __LINE__, log)
#define LOG_WARN(id, log)  LOG_STREAM(id, LOG_LEVEL_WARN, __FILE__, __LINE__, log)
#define LOG_ERROR(id, log) LOG_STREAM(id, LOG_LEVEL_ERROR, __FILE__, __LINE__, log)
#define LOG_ALARM(id, log) LOG_STREAM(id, LOG_LEVEL_ALARM, __FILE__, __LINE__, log)
#define LOG_FATAL(id, log) LOG_STREAM(id, LOG_LEVEL_FATAL, __FILE__, __LINE__, log)

//! super macro.
#define LOGT( log ) LOG_TRACE(LOG4Z_MAIN_LOGGER_ID, log )
#define LOGD( log ) LOG_DEBUG(LOG4Z_MAIN_LOGGER_ID, log )
#define LOGI( log ) LOG_INFO(LOG4Z_MAIN_LOGGER_ID, log )
#define LOGW( log ) LOG_WARN(LOG4Z_MAIN_LOGGER_ID, log )
#define LOGE( log ) LOG_ERROR(LOG4Z_MAIN_LOGGER_ID, log )
#define LOGA( log ) LOG_ALARM(LOG4Z_MAIN_LOGGER_ID, log )
#define LOGF( log ) LOG_FATAL(LOG4Z_MAIN_LOGGER_ID, log )


//! format input log.
#ifdef LOG4Z_FORMAT_INPUT_ENABLE
#ifdef WIN32
#define LOG_FORMAT(id, level, file, line, logformat, ...) \
do{ \
    if (zsummer::log4z::ILog4zManager::getPtr()->prePushLog(id,level)) \
    {\
        zsummer::log4z::LogData * __pLog = zsummer::log4z::ILog4zManager::getPtr()->makeLogData(id, level); \
        int __logLen = _snprintf_s(__pLog->_content + __pLog->_contentLen, LOG4Z_LOG_BUF_SIZE - __pLog->_contentLen, _TRUNCATE, logformat, ##__VA_ARGS__); \
        if (__logLen < 0) __logLen = LOG4Z_LOG_BUF_SIZE - __pLog->_contentLen; \
        __pLog->_contentLen += __logLen; \
        zsummer::log4z::ILog4zManager::getPtr()->pushLog(__pLog, file, line); \
    }\
} while (0)
#else
#define LOG_FORMAT(id, level, file, line, logformat, ...) \
do{ \
    if (zsummer::log4z::ILog4zManager::getPtr()->prePushLog(id,level)) \
    {\
        zsummer::log4z::LogData * __pLog = zsummer::log4z::ILog4zManager::getPtr()->makeLogData(id, level); \
        int __logLen = snprintf(__pLog->_content + __pLog->_contentLen, LOG4Z_LOG_BUF_SIZE - __pLog->_contentLen,logformat, ##__VA_ARGS__); \
        if (__logLen < 0) __logLen = 0; \
        if (__logLen > LOG4Z_LOG_BUF_SIZE - __pLog->_contentLen) __logLen = LOG4Z_LOG_BUF_SIZE - __pLog->_contentLen; \
        __pLog->_contentLen += __logLen; \
        zsummer::log4z::ILog4zManager::getPtr()->pushLog(__pLog, file, line); \
    } \
}while(0)
#endif
//!format string
#define LOGFMT_TRACE(id, fmt, ...)  LOG_FORMAT(id, LOG_LEVEL_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGFMT_DEBUG(id, fmt, ...)  LOG_FORMAT(id, LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGFMT_INFO(id, fmt, ...)  LOG_FORMAT(id, LOG_LEVEL_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGFMT_WARN(id, fmt, ...)  LOG_FORMAT(id, LOG_LEVEL_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGFMT_ERROR(id, fmt, ...)  LOG_FORMAT(id, LOG_LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGFMT_ALARM(id, fmt, ...)  LOG_FORMAT(id, LOG_LEVEL_ALARM, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGFMT_FATAL(id, fmt, ...)  LOG_FORMAT(id, LOG_LEVEL_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGFMTT( fmt, ...) LOGFMT_TRACE(LOG4Z_MAIN_LOGGER_ID, fmt,  ##__VA_ARGS__)
#define LOGFMTD( fmt, ...) LOGFMT_DEBUG(LOG4Z_MAIN_LOGGER_ID, fmt,  ##__VA_ARGS__)
#define LOGFMTI( fmt, ...) LOGFMT_INFO(LOG4Z_MAIN_LOGGER_ID, fmt,  ##__VA_ARGS__)
#define LOGFMTW( fmt, ...) LOGFMT_WARN(LOG4Z_MAIN_LOGGER_ID, fmt,  ##__VA_ARGS__)
#define LOGFMTE( fmt, ...) LOGFMT_ERROR(LOG4Z_MAIN_LOGGER_ID, fmt,  ##__VA_ARGS__)
#define LOGFMTA( fmt, ...) LOGFMT_ALARM(LOG4Z_MAIN_LOGGER_ID, fmt,  ##__VA_ARGS__)
#define LOGFMTF( fmt, ...) LOGFMT_FATAL(LOG4Z_MAIN_LOGGER_ID, fmt,  ##__VA_ARGS__)
#else
inline void empty_log_format_function1(LoggerId id, const char*, ...){}
inline void empty_log_format_function2(const char*, ...){}
#define LOGFMT_TRACE empty_log_format_function1
#define LOGFMT_DEBUG LOGFMT_TRACE
#define LOGFMT_INFO LOGFMT_TRACE
#define LOGFMT_WARN LOGFMT_TRACE
#define LOGFMT_ERROR LOGFMT_TRACE
#define LOGFMT_ALARM LOGFMT_TRACE
#define LOGFMT_FATAL LOGFMT_TRACE
#define LOGFMTT empty_log_format_function2
#define LOGFMTD LOGFMTT
#define LOGFMTI LOGFMTT
#define LOGFMTW LOGFMTT
#define LOGFMTE LOGFMTT
#define LOGFMTA LOGFMTT
#define LOGFMTF LOGFMTT
#endif


_ZSUMMER_BEGIN
_ZSUMMER_LOG4Z_BEGIN

//! optimze from std::stringstream to Log4zStream
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4996)
#endif
class Log4zBinary
{
public:
    Log4zBinary(const void * buf, size_t len)
    {
        this->buf = (const char *)buf;
        this->len = len;
    }
    const char * buf;
    size_t  len;
};

class Log4zString
{
public:
    Log4zString(const char * buf, size_t len)
    {
        this->buf = (const char *)buf;
        this->len = len;
    }
    const char * buf;
    size_t  len;
};

class Log4zStream
{
public:
    inline Log4zStream(char * buf, int len);
    inline int getCurrentLen(){return (int)(_cur - _begin);}
public:
    inline Log4zStream & writeLongLong(long long t, int width = 0, int dec = 10);
    inline Log4zStream & writeULongLong(unsigned long long t, int width = 0, int dec = 10);
    inline Log4zStream & writeDouble(double t, bool isSimple);
    inline Log4zStream & writePointer(const void * t);
    inline Log4zStream & writeString(const char * t) { return writeString(t, strlen(t)); };
    inline Log4zStream & writeString(const char * t, size_t len);
    inline Log4zStream & writeChar(char ch);
    inline Log4zStream & writeBinary(const Log4zBinary & t);
public:
    inline Log4zStream & operator <<(const void * t){ return  writePointer(t); }

    inline Log4zStream & operator <<(const char * t){return writeString(t);}

    inline Log4zStream & operator <<(bool t){ return (t ? writeString("true", 4) : writeString("false", 5));}

    inline Log4zStream & operator <<(char t){return writeChar(t);}

    inline Log4zStream & operator <<(unsigned char t){return writeULongLong(t);}

    inline Log4zStream & operator <<(short t){ return writeLongLong(t); }

    inline Log4zStream & operator <<(unsigned short t){ return writeULongLong(t); }

    inline Log4zStream & operator <<(int t){return writeLongLong(t);}

    inline Log4zStream & operator <<(unsigned int t){return writeULongLong(t);}

    inline Log4zStream & operator <<(long t) { return writeLongLong(t); }

    inline Log4zStream & operator <<(unsigned long t){ return writeULongLong(t); }

    inline Log4zStream & operator <<(long long t) { return writeLongLong(t); }

    inline Log4zStream & operator <<(unsigned long long t){ return writeULongLong(t); }

    inline Log4zStream & operator <<(float t){return writeDouble(t, true);}

    inline Log4zStream & operator <<(double t){return writeDouble(t, false);}

    template<class _Elem,class _Traits,class _Alloc> //support std::string, std::wstring
    inline Log4zStream & operator <<(const std::basic_string<_Elem, _Traits, _Alloc> & t){ return writeString(t.c_str(), t.length()); }

    inline Log4zStream & operator << (const zsummer::log4z::Log4zBinary & binary) { return writeBinary(binary); }

    inline Log4zStream & operator << (const zsummer::log4z::Log4zString & str) { return writeString(str.buf, str.len); }

    template<class _Ty1, class _Ty2>
    inline Log4zStream & operator <<(const std::pair<_Ty1, _Ty2> & t){ return *this << "pair(" << t.first << ":" << t.second << ")"; }

    template<class _Elem, class _Alloc>
    inline Log4zStream & operator <<(const std::vector<_Elem, _Alloc> & t)
    { 
        *this << "vector(" << t.size() << ")[";
        int inputCount = 0;
        for (typename std::vector<_Elem, _Alloc>::const_iterator iter = t.begin(); iter != t.end(); iter++)
        {
            inputCount++;
            if (inputCount > LOG4Z_LOG_CONTAINER_DEPTH)
            {
                *this << "..., ";
                break;
            }
            *this << *iter << ", ";
        }
        if (!t.empty())
        {
            _cur -= 2;
        }
        return *this << "]";
    }
    template<class _Elem, class _Alloc>
    inline Log4zStream & operator <<(const std::list<_Elem, _Alloc> & t)
    { 
        *this << "list(" << t.size() << ")[";
        int inputCount = 0;
        for (typename std::list<_Elem, _Alloc>::const_iterator iter = t.begin(); iter != t.end(); iter++)
        {
            inputCount++;
            if (inputCount > LOG4Z_LOG_CONTAINER_DEPTH)
            {
                *this << "..., ";
                break;
            }
            *this << *iter << ", ";
        }
        if (!t.empty())
        {
            _cur -= 2;
        }
        return *this << "]";
    }
    template<class _Elem, class _Alloc>
    inline Log4zStream & operator <<(const std::deque<_Elem, _Alloc> & t)
    {
        *this << "deque(" << t.size() << ")[";
        int inputCount = 0;
        for (typename std::deque<_Elem, _Alloc>::const_iterator iter = t.begin(); iter != t.end(); iter++)
        {
            inputCount++;
            if (inputCount > LOG4Z_LOG_CONTAINER_DEPTH)
            {
                *this << "..., ";
                break;
            }
            *this << *iter << ", ";
        }
        if (!t.empty())
        {
            _cur -= 2;
        }
        return *this << "]";
    }
    template<class _Elem, class _Alloc>
    inline Log4zStream & operator <<(const std::queue<_Elem, _Alloc> & t)
    {
        *this << "queue(" << t.size() << ")[";
        int inputCount = 0;
        for (typename std::queue<_Elem, _Alloc>::const_iterator iter = t.begin(); iter != t.end(); iter++)
        {
            inputCount++;
            if (inputCount > LOG4Z_LOG_CONTAINER_DEPTH)
            {
                *this << "..., ";
                break;
            }
            *this << *iter << ", ";
        }
        if (!t.empty())
        {
            _cur -= 2;
        }
        return *this << "]";
    }
    template<class _K, class _V, class _Pr, class _Alloc>
    inline Log4zStream & operator <<(const std::map<_K, _V, _Pr, _Alloc> & t)
    {
        *this << "map(" << t.size() << ")[";
        int inputCount = 0;
        for (typename std::map < _K, _V, _Pr, _Alloc>::const_iterator iter = t.begin(); iter != t.end(); iter++)
        {
            inputCount++;
            if (inputCount > LOG4Z_LOG_CONTAINER_DEPTH)
            {
                *this << "..., ";
                break;
            }
            *this << *iter << ", ";
        }
        if (!t.empty())
        {
            _cur -= 2;
        }
        return *this << "]";
    }

private:
    Log4zStream(){}
    Log4zStream(Log4zStream &){}
    char *  _begin;
    char *  _end;
    char *  _cur;
};

inline Log4zStream::Log4zStream(char * buf, int len)
{
    _begin = buf;
    _end = buf + len;
    _cur = _begin;
}



inline Log4zStream & Log4zStream::writeLongLong(long long t, int width, int dec)
{
    if (t < 0 )
    {
        t = -t;
        writeChar('-');
    }
    writeULongLong((unsigned long long)t, width, dec);
    return *this;
}

inline Log4zStream & Log4zStream::writeULongLong(unsigned long long t, int width, int dec)
{
    static const char * lut = 
        "0123456789abcdef";

    static const char *lutDec = 
        "00010203040506070809"
        "10111213141516171819"
        "20212223242526272829"
        "30313233343536373839"
        "40414243444546474849"
        "50515253545556575859"
        "60616263646566676869"
        "70717273747576777879"
        "80818283848586878889"
        "90919293949596979899";

    static const char *lutHex = 
        "000102030405060708090A0B0C0D0E0F"
        "101112131415161718191A1B1C1D1E1F"
        "202122232425262728292A2B2C2D2E2F"
        "303132333435363738393A3B3C3D3E3F"
        "404142434445464748494A4B4C4D4E4F"
        "505152535455565758595A5B5C5D5E5F"
        "606162636465666768696A6B6C6D6E6F"
        "707172737475767778797A7B7C7D7E7F"
        "808182838485868788898A8B8C8D8E8F"
        "909192939495969798999A9B9C9D9E9F"
        "A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
        "B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
        "C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
        "D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
        "E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
        "F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";

    const unsigned long long cacheSize = 64;

    if ((unsigned long long)(_end - _cur) > cacheSize)
    {
        char buf[cacheSize];
        unsigned long long val = t;
        unsigned long long i = cacheSize;
        unsigned long long digit = 0;



        if (dec == 10)
        {
            do
            {
                const unsigned long long m2 = (unsigned long long)((val % 100) * 2);
                *(buf + i - 1) = lutDec[m2 + 1];
                *(buf + i - 2) = lutDec[m2];
                i -= 2;
                val /= 100;
                digit += 2;
            } while (val && i >= 2);
            if (digit >= 2 && buf[cacheSize - digit] == '0')
            {
                digit--;
            }
        }
        else if (dec == 16)
        {
            do
            {
                const unsigned long long m2 = (unsigned long long)((val % 256) * 2);
                *(buf + i - 1) = lutHex[m2 + 1];
                *(buf + i - 2) = lutHex[m2];
                i -= 2;
                val /= 256;
                digit += 2;
            } while (val && i >= 2);
            if (digit >= 2 && buf[cacheSize - digit] == '0')
            {
                digit--;
            }
        }
        else
        {
            do
            {
                buf[--i] = lut[val % dec];
                val /= dec;
                digit++;
            } while (val && i > 0);
        }

        while (digit < (unsigned long long)width)
        {
            digit++;
            buf[cacheSize - digit] = '0';
        }

        writeString(buf + (cacheSize - digit), (size_t)digit);
    }
    return *this;
}
inline Log4zStream & Log4zStream::writeDouble(double t, bool isSimple)
{

#if __cplusplus >= 201103L
                using std::isnan;
                using std::isinf;
#endif
    if (isnan(t))
    {
        writeString("nan", 3);
        return *this;
    }
    else if (isinf(t))
    {
        writeString("inf", 3);
        return *this;
    }



    size_t count = _end - _cur;
    double fabst = fabs(t);
    if (count > 30)
    {
        if ( fabst < 0.0001 || (!isSimple && fabst > 4503599627370495ULL) || (isSimple && fabst > 8388607))
        {
            gcvt(t, isSimple ? 7 : 16, _cur);
            size_t len = strlen(_cur);
            if (len > count) len = count;
            _cur += len;
            return *this;
        }
        else
        {
            if (t < 0.0)
            {
                writeChar('-');
            }
            double intpart = 0;
            unsigned long long fractpart = (unsigned long long)(modf(fabst, &intpart) * 10000);
            writeULongLong((unsigned long long)intpart);
            if (fractpart > 0)
            {
                writeChar('.');
                writeULongLong(fractpart, 4);
            }
        }
    }

    return *this;
}

inline Log4zStream & Log4zStream::writePointer(const void * t)
{
    sizeof(t) == 8 ?  writeULongLong((unsigned long long)t, 16, 16): writeULongLong((unsigned long long)t, 8, 16);
    return *this;
}

inline Log4zStream & Log4zStream::writeBinary(const Log4zBinary & t)
{
    writeString("\r\n\t[");
    for (size_t i=0; i<(t.len / 32)+1; i++)
    {
        writeString("\r\n\t");
        *this << (void*)(t.buf + i*32);
        writeString(": ");
        for (size_t j = i * 32; j < (i + 1) * 32 && j < t.len; j++)
        {
            if (isprint((unsigned char)t.buf[j]))
            {
                writeChar(' ');
                writeChar(t.buf[j]);
                writeChar(' ');
            }
            else
            {
                *this << " . ";
            }
        }
        writeString("\r\n\t");
        *this << (void*)(t.buf + i * 32);
        writeString(": ");
        for (size_t j = i * 32; j < (i + 1) * 32 && j < t.len; j++)
        {
            writeULongLong((unsigned long long)(unsigned char)t.buf[j], 2, 16);
            writeChar(' ');
        }
    }

    writeString("\r\n\t]\r\n\t");
    return *this;
}
inline Log4zStream & zsummer::log4z::Log4zStream::writeChar(char ch)
{
    if (_end - _cur > 1)
    {
        _cur[0] = ch;
        _cur++;
    }
    return *this;
}

inline Log4zStream & zsummer::log4z::Log4zStream::writeString(const char * t, size_t len)
{
    size_t count = _end - _cur;
    if (len > count)
    {
        len = count;
    }
    if (len > 0)
    {
        memcpy(_cur, t, len);
        _cur += len;
    }
    
    return *this;
}




#ifdef WIN32
#pragma warning(pop)
#endif




/*
 * Log4z License
 * -----------
 * 
 * Log4z is licensed under the terms of the MIT license reproduced below.
 * This means that Log4z is free software and can be used for both academic
 * and commercial purposes at absolutely no cost.
 * 
 * 
 * ===============================================================================
 * 
 * Copyright (C) 2010-2017 YaweiZhang <yawei.zhang@foxmail.com>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * ===============================================================================
 * 
 * (end of COPYRIGHT)
 */


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <algorithm>
#include <iostream>


#ifdef WIN32
#include <io.h>
#include <shlwapi.h>
#include <process.h>
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "User32.lib")
#pragma warning(disable:4996)

#else
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/syscall.h>
#endif


#ifdef __APPLE__
#include "TargetConditionals.h"
#include <dispatch/dispatch.h>
#if !TARGET_OS_IPHONE
#define LOG4Z_HAVE_LIBPROC
#include <libproc.h>
#endif
#endif



_ZSUMMER_BEGIN
_ZSUMMER_LOG4Z_BEGIN

static const char *const LOG_STRING[]=
{
    "LOG_TRACE",
    "LOG_DEBUG",
    "LOG_INFO ",
    "LOG_WARN ",
    "LOG_ERROR",
    "LOG_ALARM",
    "LOG_FATAL",
};
static const size_t LOG_STRING_LEN[] =
{
    sizeof("LOG_TRACE") - 1,
    sizeof("LOG_DEBUG") - 1,
    sizeof("LOG_INFO") - 1,
    sizeof("LOG_WARN") - 1,
    sizeof("LOG_ERROR") - 1,
    sizeof("LOG_ALARM") - 1,
    sizeof("LOG_FATAL") - 1,
};

#ifdef WIN32
const static WORD LOG_COLOR[LOG_LEVEL_FATAL + 1] = {
    0,
    0,
    FOREGROUND_BLUE | FOREGROUND_GREEN,
    FOREGROUND_GREEN | FOREGROUND_RED,
    FOREGROUND_RED,
    FOREGROUND_GREEN,
    FOREGROUND_RED | FOREGROUND_BLUE };
#else

const static char LOG_COLOR[LOG_LEVEL_FATAL + 1][50] = {
    "\e[0m",
    "\e[0m",
    "\e[34m\e[1m",//hight blue
    "\e[33m", //yellow
    "\e[31m", //red
    "\e[32m", //green
    "\e[35m" };
#endif

//////////////////////////////////////////////////////////////////////////
//! Log4zFileHandler
//////////////////////////////////////////////////////////////////////////
class Log4zFileHandler
{
public:
    Log4zFileHandler(){ _file = NULL; }
    ~Log4zFileHandler(){ close(); }
    inline bool isOpen(){ return _file != NULL; }
    inline long open(const char *path, const char * mod)
    {
        if (_file != NULL){fclose(_file);_file = NULL;}
        _file = fopen(path, mod);
        if (_file)
        {
            long tel = 0;
            long cur = ftell(_file);
            fseek(_file, 0L, SEEK_END);
            tel = ftell(_file);
            fseek(_file, cur, SEEK_SET);
            return tel;
        }
        return -1;
    }
    inline void clean(int index, int len)
    {
#if !defined(__APPLE__) && !defined(WIN32) 
       if (_file != NULL)
       {
          int fd = fileno(_file);
          fsync(fd);
          posix_fadvise(fd, index, len, POSIX_FADV_DONTNEED);
          fsync(fd);
       }
#endif
    }
    inline void close()
    {
        if (_file != NULL){clean(0, 0); fclose(_file);_file = NULL;}
    }
    inline void write(const char * data, size_t len)
    {
        if (_file && len > 0)
        {
            if (fwrite(data, 1, len, _file) != len)
            {
                close();
            }
        }
    }
    inline void flush(){ if (_file) fflush(_file); }

    inline std::string readLine()
    {
        char buf[500] = { 0 };
        if (_file && fgets(buf, 500, _file) != NULL)
        {
            return std::string(buf);
        }
        return std::string();
    }
    inline const std::string readContent();
	inline bool removeFile(const std::string & path) { return ::remove(path.c_str()) == 0; }
public:
    FILE *_file;
};


//////////////////////////////////////////////////////////////////////////
//! UTILITY
//////////////////////////////////////////////////////////////////////////


static void fixPath(std::string &path);
static void trimLogConfig(std::string &str, std::string extIgnore = std::string());
static std::pair<std::string, std::string> splitPairString(const std::string & str, const std::string & delimiter);


static bool isDirectory(std::string path);
static bool createRecursionDir(std::string path);
static std::string getProcessID();
static std::string getProcessName();



//////////////////////////////////////////////////////////////////////////
//! LockHelper
//////////////////////////////////////////////////////////////////////////
class LockHelper
{
public:
    LockHelper();
    virtual ~LockHelper();

public:
    void lock();
    void unLock();
private:
#ifdef WIN32
    CRITICAL_SECTION _crit;
#else
    pthread_mutex_t  _crit;
#endif
};

//////////////////////////////////////////////////////////////////////////
//! AutoLock
//////////////////////////////////////////////////////////////////////////
class AutoLock
{
public:
    explicit AutoLock(LockHelper & lk):_lock(lk){_lock.lock();}
    ~AutoLock(){_lock.unLock();}
private:
    LockHelper & _lock;
};






//////////////////////////////////////////////////////////////////////////
//! SemHelper
//////////////////////////////////////////////////////////////////////////
class SemHelper
{
public:
    SemHelper();
    virtual ~SemHelper();
public:
    bool create(int initcount);
    bool wait(int timeout = 0);
    bool post();
private:
#ifdef WIN32
    HANDLE _hSem;
#elif defined(__APPLE__)
    dispatch_semaphore_t _semid;
#else
    sem_t _semid;
    bool  _isCreate;
#endif

};



//////////////////////////////////////////////////////////////////////////
//! ThreadHelper
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
static unsigned int WINAPI  threadProc(LPVOID lpParam);
#else
static void * threadProc(void * pParam);
#endif

class ThreadHelper
{
public:
    ThreadHelper(){_hThreadID = 0;}
    virtual ~ThreadHelper(){}
public:
    bool start();
    bool wait();
    virtual void run() = 0;
private:
    unsigned long long _hThreadID;
#ifndef WIN32
    pthread_t _phtreadID;
#endif
};

#ifdef WIN32
unsigned int WINAPI  threadProc(LPVOID lpParam)
{
    ThreadHelper * p = (ThreadHelper *) lpParam;
    p->run();
    return 0;
}
#else
void * threadProc(void * pParam)
{
    ThreadHelper * p = (ThreadHelper *) pParam;
    p->run();
    return NULL;
}
#endif


//////////////////////////////////////////////////////////////////////////
//! LogData
//////////////////////////////////////////////////////////////////////////
enum LogDataType
{
    LDT_GENERAL,
    LDT_ENABLE_LOGGER,
    LDT_SET_LOGGER_NAME,
    LDT_SET_LOGGER_PATH,
    LDT_SET_LOGGER_LEVEL,
    LDT_SET_LOGGER_FILELINE,
    LDT_SET_LOGGER_DISPLAY,
    LDT_SET_LOGGER_OUTFILE,
    LDT_SET_LOGGER_LIMITSIZE,
	LDT_SET_LOGGER_MONTHDIR,
	LDT_SET_LOGGER_RESERVETIME,
	//    LDT_SET_LOGGER_,
};


//////////////////////////////////////////////////////////////////////////
//! LoggerInfo
//////////////////////////////////////////////////////////////////////////
struct LoggerInfo 
{
    //! attribute
    std::string _key;   //logger key
    std::string _name;    // one logger one name.
    std::string _path;    //path for log file.
    int  _level;        //filter level
    bool _display;        //display to screen 
    bool _outfile;        //output to file
    bool _monthdir;        //create directory per month 
    unsigned int _limitsize; //limit file's size, unit Million byte.
    bool _enable;        //logger is enable 
    bool _fileLine;        //enable/disable the log's suffix.(file name:line number)
	time_t _logReserveTime; //log file reserve time. unit is time second.
    //! runtime info
    time_t _curFileCreateTime;    //file create time
    time_t _curFileCreateDay;    //file create day time
    unsigned int _curFileIndex; //rolling file index
    unsigned int _curWriteLen;  //current file length
    Log4zFileHandler    _handle;        //file handle.
	//!history
	std::list<std::pair<time_t, std::string> > _historyLogs;

    
    LoggerInfo()
    {
        _enable = false; 
        _path = LOG4Z_DEFAULT_PATH; 
        _level = LOG4Z_DEFAULT_LEVEL; 
        _display = LOG4Z_DEFAULT_DISPLAY; 
        _outfile = LOG4Z_DEFAULT_OUTFILE;

        _monthdir = LOG4Z_DEFAULT_MONTHDIR; 
        _limitsize = LOG4Z_DEFAULT_LIMITSIZE;
        _fileLine = LOG4Z_DEFAULT_SHOWSUFFIX;

        _curFileCreateTime = 0;
        _curFileCreateDay = 0;
        _curFileIndex = 0;
        _curWriteLen = 0;
		_logReserveTime = 0;
    }
};


//////////////////////////////////////////////////////////////////////////
//! LogerManager
//////////////////////////////////////////////////////////////////////////
class LogerManager : public ThreadHelper, public ILog4zManager
{
public:
    LogerManager();
    virtual ~LogerManager();
    
    bool configFromStringImpl(std::string content, bool isUpdate);
    //! 读取配置文件并覆写
    virtual bool config(const char* configPath);
    virtual bool configFromString(const char* configContent);

    //! 覆写式创建
    virtual LoggerId createLogger(const char* key);
    virtual bool start();
    virtual bool stop();
    virtual bool prePushLog(LoggerId id, int level);
    virtual bool pushLog(LogData * pLog, const char * file, int line);
    //! 查找ID
    virtual LoggerId findLogger(const char*  key);
    bool hotChange(LoggerId id, LogDataType ldt, int num, const std::string & text);
    virtual bool enableLogger(LoggerId id, bool enable);
    virtual bool setLoggerName(LoggerId id, const char * name);
    virtual bool setLoggerPath(LoggerId id, const char * path);
    virtual bool setLoggerLevel(LoggerId id, int nLevel);
    virtual bool setLoggerFileLine(LoggerId id, bool enable);
    virtual bool setLoggerDisplay(LoggerId id, bool enable);
    virtual bool setLoggerOutFile(LoggerId id, bool enable);
    virtual bool setLoggerLimitsize(LoggerId id, unsigned int limitsize);
    virtual bool setLoggerMonthdir(LoggerId id, bool enable);
	virtual bool setLoggerReserveTime(LoggerId id, time_t sec);
    virtual bool setAutoUpdate(int interval);
    virtual bool updateConfig();
    virtual bool isLoggerEnable(LoggerId id);
    virtual unsigned long long getStatusTotalWriteCount(){return _ullStatusTotalWriteFileCount;}
    virtual unsigned long long getStatusTotalWriteBytes() { return _ullStatusTotalWriteFileBytes; }
    virtual unsigned long long getStatusTotalPushQueue() { return _ullStatusTotalPushLog; }
    virtual unsigned long long getStatusTotalPopQueue() { return _ullStatusTotalPopLog; }
    virtual unsigned int getStatusActiveLoggers();
protected:
    virtual LogData * makeLogData(LoggerId id, int level);
    virtual void freeLogData(LogData * log);
    void showColorText(const char *text, int level = LOG_LEVEL_DEBUG);
    bool onHotChange(LoggerId id, LogDataType ldt, int num, const std::string & text);
    bool openLogger(LogData * log);
    bool closeLogger(LoggerId id);
    bool popLog(LogData *& log);
    virtual void run();
private:

    //! thread status.
    bool        _runing;
    //! wait thread started.
    SemHelper        _semaphore;

    //! hot change name or path for one logger
    int _hotUpdateInterval;
    unsigned int _checksum;

    //! the process info.
    std::string _pid;
    std::string _proName;

    //! config file name
    std::string _configFile;

    //! logger id manager, [logger name]:[logger id].
    std::map<std::string, LoggerId> _ids; 
    // the last used id of _loggers
    LoggerId    _lastId; 
    LoggerInfo _loggers[LOG4Z_LOGGER_MAX];

    
    //! log queue
    char _chunk1[256];
    LockHelper    _logLock;
    std::deque<LogData *> _logs;
    unsigned long long _ullStatusTotalPushLog;

    char _chunk2[256];
    LockHelper    _freeLock;
    std::vector<LogData*> _freeLogDatas;

    char _chunk3[256];
    //show color lock
    LockHelper _scLock;
    //status statistics
    //write file
    char _chunk4[256];
    std::deque<LogData *> _logsCache;
    unsigned long long _ullStatusTotalPopLog;
    unsigned long long _ullStatusTotalWriteFileCount;
    unsigned long long _ullStatusTotalWriteFileBytes;
};




//////////////////////////////////////////////////////////////////////////
//! Log4zFileHandler
//////////////////////////////////////////////////////////////////////////

const std::string Log4zFileHandler::readContent()
{
    std::string content;

    if (!_file)
    {
        return content;
    }
    char buf[BUFSIZ];
    size_t ret = 0;
    do  
    {
        ret = fread(buf, sizeof(char), BUFSIZ, _file);
        content.append(buf, ret);
    }
    while (ret == BUFSIZ);

    return content;
}




//////////////////////////////////////////////////////////////////////////
//! utility
//////////////////////////////////////////////////////////////////////////


static inline void sleepMillisecond(unsigned int ms)
{
#ifdef WIN32
    ::Sleep(ms);
#else
    usleep(1000*ms);
#endif
}

static inline struct tm timeToTm(time_t t)
{
#ifdef WIN32
#if _MSC_VER < 1400 //VS2003
    return * localtime(&t);
#else //vs2005->vs2013->
    struct tm tt = { 0 };
    localtime_s(&tt, &t);
    return tt;
#endif
#else //linux
    struct tm tt = { 0 };
    localtime_r(&t, &tt);
    return tt;
#endif
}




static void fixPath(std::string &path)
{
    if (path.empty()){return;}
    for (std::string::iterator iter = path.begin(); iter != path.end(); ++iter)
    {
        if (*iter == '\\'){*iter = '/';}
    }
    if (path.at(path.length()-1) != '/'){path.append("/");}
}

static void trimLogConfig(std::string &str, std::string extIgnore)
{
    if (str.empty()){return;}
    extIgnore += "\r\n\t ";
    int length = (int)str.length();
    int posBegin = 0;
    int posEnd = 0;

    //trim utf8 file bom
    if (str.length() >= 3 
        && (unsigned char)str[0] == 0xef
        && (unsigned char)str[1] == 0xbb
        && (unsigned char)str[2] == 0xbf)
    {
        posBegin = 3;
    }

    //trim character 
    for (int i = posBegin; i<length; i++)
    {
        bool bCheck = false;
        for (int j = 0; j < (int)extIgnore.length(); j++)
        {
            if (str[i] == extIgnore[j])
            {
                bCheck = true;
            }
        }
        if (bCheck)
        {
            if (i == posBegin)
            {
                posBegin++;
            }
        }
        else
        {
            posEnd = i + 1;
        }
    }
    if (posBegin < posEnd)
    {
        str = str.substr(posBegin, posEnd-posBegin);
    }
    else
    {
        str.clear();
    }
}

//split
static std::pair<std::string, std::string> splitPairString(const std::string & str, const std::string & delimiter)
{
    std::string::size_type pos = str.find(delimiter.c_str());
    if (pos == std::string::npos)
    {
        return std::make_pair(str, "");
    }
    return std::make_pair(str.substr(0, pos), str.substr(pos+delimiter.length()));
}

static bool parseConfigLine(const std::string& line, int curLineNum, std::string & key, std::map<std::string, LoggerInfo> & outInfo)
{
    std::pair<std::string, std::string> kv = splitPairString(line, "=");
    if (kv.first.empty())
    {
        return false;
    }

    trimLogConfig(kv.first);
    trimLogConfig(kv.second);
    if (kv.first.empty() || kv.first.at(0) == '#')
    {
        return true;
    }

    if (kv.first.at(0) == '[')
    {
        trimLogConfig(kv.first, "[]");
        key = kv.first;
        {
            std::string tmpstr = kv.first;
            std::transform(tmpstr.begin(), tmpstr.end(), tmpstr.begin(), ::tolower);
            if (tmpstr == "main")
            {
                key = "Main";
            }
        }
        std::map<std::string, LoggerInfo>::iterator iter = outInfo.find(key);
        if (iter == outInfo.end())
        {
            LoggerInfo li;
            li._enable = true;
            li._key = key;
            li._name = key;
            outInfo.insert(std::make_pair(li._key, li));
        }
        else
        {
			printf("log4z configure warning: duplicate logger key:[%s] at line: %d \r\n", key.c_str(), curLineNum);
        }
        return true;
    }
    trimLogConfig(kv.first);
    trimLogConfig(kv.second);
    std::map<std::string, LoggerInfo>::iterator iter = outInfo.find(key);
    if (iter == outInfo.end())
    {
		printf("log4z configure warning: not found current logger name:[%s] at line:%d, key=%s, value=%s \r\n", 
			key.c_str(), curLineNum, kv.first.c_str(), kv.second.c_str());
        return true;
    }
    std::transform(kv.first.begin(), kv.first.end(), kv.first.begin(), ::tolower);
    //! path
    if (kv.first == "path")
    {
        iter->second._path = kv.second;
        return true;
    }
    else if (kv.first == "name")
    {
        iter->second._name = kv.second;
        return true;
    }
    std::transform(kv.second.begin(), kv.second.end(), kv.second.begin(), ::tolower);
    //! level
    if (kv.first == "level")
    {
        if (kv.second == "trace" || kv.second == "all")
        {
            iter->second._level = LOG_LEVEL_TRACE;
        }
        else if (kv.second == "debug")
        {
            iter->second._level = LOG_LEVEL_DEBUG;
        }
        else if (kv.second == "info")
        {
            iter->second._level = LOG_LEVEL_INFO;
        }
        else if (kv.second == "warn" || kv.second == "warning")
        {
            iter->second._level = LOG_LEVEL_WARN;
        }
        else if (kv.second == "error")
        {
            iter->second._level = LOG_LEVEL_ERROR;
        }
        else if (kv.second == "alarm")
        {
            iter->second._level = LOG_LEVEL_ALARM;
        }
        else if (kv.second == "fatal")
        {
            iter->second._level = LOG_LEVEL_FATAL;
        }
    }
    //! display
    else if (kv.first == "display")
    {
        if (kv.second == "false" || kv.second == "0")
        {
            iter->second._display = false;
        }
        else
        {
            iter->second._display = true;
        }
    }
    //! output to file
    else if (kv.first == "outfile")
    {
        if (kv.second == "false" || kv.second == "0")
        {
            iter->second._outfile = false;
        }
        else
        {
            iter->second._outfile = true;
        }
    }
    //! monthdir
    else if (kv.first == "monthdir")
    {
        if (kv.second == "false" || kv.second == "0")
        {
            iter->second._monthdir = false;
        }
        else
        {
            iter->second._monthdir = true;
        }
    }
    //! limit file size
    else if (kv.first == "limitsize")
    {
        iter->second._limitsize = atoi(kv.second.c_str());
    }
    //! display log in file line
    else if (kv.first == "fileline")
    {
        if (kv.second == "false" || kv.second == "0")
        {
            iter->second._fileLine = false;
        }
        else
        {
            iter->second._fileLine = true;
        }
    }
    //! enable/disable one logger
    else if (kv.first == "enable")
    {
        if (kv.second == "false" || kv.second == "0")
        {
            iter->second._enable = false;
        }
        else
        {
            iter->second._enable = true;
        }
    }
	//! set reserve time 
	else if (kv.first == "reserve")
	{
		iter->second._logReserveTime = atoi(kv.second.c_str());
	}
    return true;
}

static bool parseConfigFromString(std::string content, std::map<std::string, LoggerInfo> & outInfo)
{

    std::string key;
    int curLine = 1;
    std::string line;
    std::string::size_type curPos = 0;
    if (content.empty())
    {
        return true;
    }
    do
    {
        std::string::size_type pos = std::string::npos;
        for (std::string::size_type i = curPos; i < content.length(); ++i)
        {
            //support linux/unix/windows LRCF
            if (content[i] == '\r' || content[i] == '\n')
            {
                pos = i;
                break;
            }
        }
        line = content.substr(curPos, pos - curPos);
        parseConfigLine(line, curLine, key, outInfo);
        curLine++;

        if (pos == std::string::npos)
        {
            break;
        }
        else
        {
            curPos = pos+1;
        }
    } while (1);
    return true;
}



bool isDirectory(std::string path)
{
#ifdef WIN32
    return PathIsDirectoryA(path.c_str()) ? true : false;
#else
    DIR * pdir = opendir(path.c_str());
    if (pdir == NULL)
    {
        return false;
    }
    else
    {
        closedir(pdir);
        pdir = NULL;
        return true;
    }
#endif
}



bool createRecursionDir(std::string path)
{
    if (path.length() == 0) return true;
    std::string sub;
    fixPath(path);

    std::string::size_type pos = path.find('/');
    while (pos != std::string::npos)
    {
        std::string cur = path.substr(0, pos-0);
        if (cur.length() > 0 && !isDirectory(cur))
        {
            bool ret = false;
#ifdef WIN32
            ret = CreateDirectoryA(cur.c_str(), NULL) ? true : false;
#else
            ret = (mkdir(cur.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) == 0);
#endif
            if (!ret)
            {
                return false;
            }
        }
        pos = path.find('/', pos+1);
    }

    return true;
}

std::string getProcessID()
{
    std::string pid = "0";
    char buf[260] = {0};
#ifdef WIN32
    DWORD winPID = GetCurrentProcessId();
    sprintf(buf, "%06u", winPID);
    pid = buf;
#else
    sprintf(buf, "%06d", getpid());
    pid = buf;
#endif
    return pid;
}


std::string getProcessName()
{
    std::string name = "process";
    char buf[260] = {0};
#ifdef WIN32
    if (GetModuleFileNameA(NULL, buf, 259) > 0)
    {
        name = buf;
    }
    std::string::size_type pos = name.rfind("\\");
    if (pos != std::string::npos)
    {
        name = name.substr(pos+1, std::string::npos);
    }
    pos = name.rfind(".");
    if (pos != std::string::npos)
    {
        name = name.substr(0, pos-0);
    }

#elif defined(LOG4Z_HAVE_LIBPROC)
    proc_name(getpid(), buf, 260);
    name = buf;
    return name;;
#else
    sprintf(buf, "/proc/%d/cmdline", (int)getpid());
    Log4zFileHandler i;
    i.open(buf, "rb");
    if (!i.isOpen())
    {
        return name;
    }
    name = i.readLine();
    i.close();

    std::string::size_type pos = name.rfind("/");
    if (pos != std::string::npos)
    {
        name = name.substr(pos+1, std::string::npos);
    }
#endif


    return name;
}






//////////////////////////////////////////////////////////////////////////
// LockHelper
//////////////////////////////////////////////////////////////////////////
LockHelper::LockHelper()
{
#ifdef WIN32
    InitializeCriticalSection(&_crit);
#else
    //_crit = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    //pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_TIMED_NP);
    //pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&_crit, &attr);
    pthread_mutexattr_destroy(&attr);
#endif
}
LockHelper::~LockHelper()
{
#ifdef WIN32
    DeleteCriticalSection(&_crit);
#else
    pthread_mutex_destroy(&_crit);
#endif
}

void LockHelper::lock()
{
#ifdef WIN32
    EnterCriticalSection(&_crit);
#else
    pthread_mutex_lock(&_crit);
#endif
}
void LockHelper::unLock()
{
#ifdef WIN32
    LeaveCriticalSection(&_crit);
#else
    pthread_mutex_unlock(&_crit);
#endif
}
//////////////////////////////////////////////////////////////////////////
// SemHelper
//////////////////////////////////////////////////////////////////////////
SemHelper::SemHelper()
{
#ifdef WIN32
    _hSem = NULL;
#elif defined(__APPLE__)
    _semid = NULL;
#else
    _isCreate = false;
#endif

}
SemHelper::~SemHelper()
{
#ifdef WIN32
    if (_hSem != NULL)
    {
        CloseHandle(_hSem);
        _hSem = NULL;
    }
#elif defined(__APPLE__)
    if (_semid)
    {
        dispatch_release(_semid);
        _semid = NULL;
    }
#else
    if (_isCreate)
    {
        _isCreate = false;
        sem_destroy(&_semid);
    }
#endif

}

bool SemHelper::create(int initcount)
{
    if (initcount < 0)
    {
        initcount = 0;
    }
#ifdef WIN32
    if (initcount > 64)
    {
        return false;
    }
    _hSem = CreateSemaphore(NULL, initcount, 64, NULL);
    if (_hSem == NULL)
    {
        return false;
    }
#elif defined(__APPLE__)
    _semid = dispatch_semaphore_create(initcount);
    if (!_semid)
    {
        return false;
    }
#else
    if (sem_init(&_semid, 0, initcount) != 0)
    {
        return false;
    }
    _isCreate = true;
#endif

    return true;
}
bool SemHelper::wait(int timeout)
{
#ifdef WIN32
    if (timeout <= 0)
    {
        timeout = INFINITE;
    }
    if (WaitForSingleObject(_hSem, timeout) != WAIT_OBJECT_0)
    {
        return false;
    }
#elif defined(__APPLE__)
    if (dispatch_semaphore_wait(_semid, dispatch_time(DISPATCH_TIME_NOW, timeout*1000)) != 0)
    {
        return false;
    }
#else
    if (timeout <= 0)
    {
        return (sem_wait(&_semid) == 0);
    }
    else
    {
        struct timeval tm;
        gettimeofday(&tm, NULL);
        long long endtime = tm.tv_sec *1000 + tm.tv_usec/1000 + timeout;
        do 
        {
            sleepMillisecond(50);
            int ret = sem_trywait(&_semid);
            if (ret == 0)
            {
                return true;
            }
            struct timeval tv_cur;
            gettimeofday(&tv_cur, NULL);
            if (tv_cur.tv_sec*1000 + tv_cur.tv_usec/1000 > endtime)
            {
                return false;
            }

            if (ret == -1 && errno == EAGAIN)
            {
                continue;
            }
            else
            {
                return false;
            }
        } while (true);
        return false;
    }
#endif
    return true;
}

bool SemHelper::post()
{
#ifdef WIN32
    return ReleaseSemaphore(_hSem, 1, NULL) ? true : false;
#elif defined(__APPLE__)
    return dispatch_semaphore_signal(_semid) == 0;
#else
    return (sem_post(&_semid) == 0);
#endif

}

//////////////////////////////////////////////////////////////////////////
//! ThreadHelper
//////////////////////////////////////////////////////////////////////////
bool ThreadHelper::start()
{
#ifdef WIN32
    unsigned long long ret = _beginthreadex(NULL, 0, threadProc, (void *) this, 0, NULL);

    if (ret == -1 || ret == 0)
    {
		printf("log4z: create log4z thread error! \r\n");
        return false;
    }
    _hThreadID = ret;
#else
    int ret = pthread_create(&_phtreadID, NULL, threadProc, (void*)this);
    if (ret != 0)
    {
		printf("log4z: create log4z thread error! \r\n");
        return false;
    }
#endif
    return true;
}

bool ThreadHelper::wait()
{
#ifdef WIN32
    if (WaitForSingleObject((HANDLE)_hThreadID, INFINITE) != WAIT_OBJECT_0)
    {
        return false;
    }
#else
    if (pthread_join(_phtreadID, NULL) != 0)
    {
        return false;
    }
#endif
    return true;
}

//////////////////////////////////////////////////////////////////////////
//! LogerManager
//////////////////////////////////////////////////////////////////////////
LogerManager::LogerManager()
{
    _runing = false;
    _lastId = LOG4Z_MAIN_LOGGER_ID;
    _hotUpdateInterval = 0;

    _ullStatusTotalPushLog = 0;
    _ullStatusTotalPopLog = 0;
    _ullStatusTotalWriteFileCount = 0;
    _ullStatusTotalWriteFileBytes = 0;
    
    _pid = getProcessID();
    _proName = getProcessName();
    _loggers[LOG4Z_MAIN_LOGGER_ID]._enable = true;
    _ids[LOG4Z_MAIN_LOGGER_KEY] = LOG4Z_MAIN_LOGGER_ID;
    _loggers[LOG4Z_MAIN_LOGGER_ID]._key = LOG4Z_MAIN_LOGGER_KEY;
    _loggers[LOG4Z_MAIN_LOGGER_ID]._name = LOG4Z_MAIN_LOGGER_KEY;

    _chunk1[0] = '\0';
    _chunk2[1] = '\0';
    _chunk3[2] = '\0';
    _chunk4[3] = '\0';
}
LogerManager::~LogerManager()
{
    stop();
}


LogData * LogerManager::makeLogData(LoggerId id, int level)
{
    LogData * pLog = NULL;
    if (true)
    {
        if (!_freeLogDatas.empty())
        {
            AutoLock l(_freeLock);
            if (!_freeLogDatas.empty())
            {
                pLog = _freeLogDatas.back();
                _freeLogDatas.pop_back();
            }
        }
        if (pLog == NULL)
        {
            pLog = new(malloc(sizeof(LogData) + LOG4Z_LOG_BUF_SIZE-1))LogData();
        }
    }
    //append precise time to log
    if (true)
    {
        pLog->_id = id;
        pLog->_level = level;
        pLog->_type = LDT_GENERAL;
        pLog->_typeval = 0;
        pLog->_threadID = 0;
        pLog->_contentLen = 0;
#ifdef WIN32
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        unsigned long long now = ft.dwHighDateTime;
        now <<= 32;
        now |= ft.dwLowDateTime;
        now /= 10;
        now -= 11644473600000000ULL;
        now /= 1000;
        pLog->_time = now / 1000;
        pLog->_precise = (unsigned int)(now % 1000);
#else
        struct timeval tm;
        gettimeofday(&tm, NULL);
        pLog->_time = tm.tv_sec;
        pLog->_precise = tm.tv_usec / 1000;
#endif
#ifdef WIN32
        pLog->_threadID = GetCurrentThreadId();
#elif defined(__APPLE__)
        unsigned long long tid = 0;
        pthread_threadid_np(NULL, &tid);
        pLog->_threadID = (unsigned int) tid;
#else
        pLog->_threadID = (unsigned int)syscall(SYS_gettid);
#endif
    }

    //format log
    if (true)
    {
#ifdef WIN32
        static __declspec(thread) tm g_tt = { 0 };
        static __declspec(thread) time_t g_curDayTime =  0 ;
#else
        static __thread tm g_tt = { 0 };
        static __thread time_t g_curDayTime = 0;
#endif // WIN32
        if (pLog->_time < g_curDayTime || pLog->_time >= g_curDayTime + 24*3600)
        {
            g_tt = timeToTm(pLog->_time);
            g_tt.tm_hour = 0;
            g_tt.tm_min = 0;
            g_tt.tm_sec = 0;
            g_curDayTime = mktime(&g_tt);
        }
        time_t sec = pLog->_time - g_curDayTime;
        Log4zStream ls(pLog->_content, LOG4Z_LOG_BUF_SIZE);
        ls.writeULongLong(g_tt.tm_year + 1900, 4);
        ls.writeChar('-');
        ls.writeULongLong(g_tt.tm_mon + 1, 2);
        ls.writeChar('-');
        ls.writeULongLong(g_tt.tm_mday, 2);
        ls.writeChar(' ');
        ls.writeULongLong(sec/3600, 2);
        ls.writeChar(':');
        ls.writeULongLong((sec % 3600)/60 , 2);
        ls.writeChar(':');
        ls.writeULongLong(sec % 60, 2);
        ls.writeChar('.');
        ls.writeULongLong(pLog->_precise, 3);
        ls.writeChar(' ');
        ls.writeChar('[');
        ls.writeULongLong(pLog->_threadID, 4);
        ls.writeChar(']');

        ls.writeChar(' ');
        ls.writeString(LOG_STRING[pLog->_level], LOG_STRING_LEN[pLog->_level]);
        ls.writeChar(' ');
        pLog->_contentLen = ls.getCurrentLen();
    }
    return pLog;
}
void LogerManager::freeLogData(LogData * log)
{
    if (_freeLogDatas.size() < 200)
    {
        AutoLock l(_freeLock);
        _freeLogDatas.push_back(log);
    }
    else
    {
        log->~LogData();
        free( log);
    }
}

void LogerManager::showColorText(const char *text, int level)
{

#if defined(WIN32) && defined(LOG4Z_OEM_CONSOLE)
    char oem[LOG4Z_LOG_BUF_SIZE] = { 0 };
    CharToOemBuffA(text, oem, LOG4Z_LOG_BUF_SIZE);
#endif

    if (level <= LOG_LEVEL_DEBUG || level > LOG_LEVEL_FATAL)
    {
#if defined(WIN32) && defined(LOG4Z_OEM_CONSOLE)
        printf("%s", oem);
#else
        printf("%s", text);
#endif
        return;
    }
#ifndef WIN32
    printf("%s%s\e[0m", LOG_COLOR[level], text);
#else
    AutoLock l(_scLock);
    HANDLE hStd = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStd == INVALID_HANDLE_VALUE) return;
    CONSOLE_SCREEN_BUFFER_INFO oldInfo;
    if (!GetConsoleScreenBufferInfo(hStd, &oldInfo))
    {
        return;
    }
    else
    {
        SetConsoleTextAttribute(hStd, LOG_COLOR[level]);
#ifdef LOG4Z_OEM_CONSOLE
		printf("%s", oem);
#else
		printf("%s", text);
#endif
		SetConsoleTextAttribute(hStd, oldInfo.wAttributes);
    }
#endif
    return;
}

bool LogerManager::configFromStringImpl(std::string content, bool isUpdate)
{
    unsigned int sum = 0;
    for (std::string::iterator iter = content.begin(); iter != content.end(); ++iter)
    {
        sum += (unsigned char)*iter;
    }
    if (sum == _checksum)
    {
        return true;
    }
    _checksum = sum;
    

    std::map<std::string, LoggerInfo> loggerMap;
    if (!parseConfigFromString(content, loggerMap))
    {
        printf(" !!! !!! !!! !!!\r\n");
		printf(" !!! !!! log4z load config file error \r\n");
		printf(" !!! !!! !!! !!!\r\n");
        return false;
    }
    for (std::map<std::string, LoggerInfo>::iterator iter = loggerMap.begin(); iter != loggerMap.end(); ++iter)
    {
        LoggerId id = LOG4Z_INVALID_LOGGER_ID;
        id = findLogger(iter->second._key.c_str());
        if (id == LOG4Z_INVALID_LOGGER_ID)
        {
            if (isUpdate)
            {
                continue;
            }
            else
            {
                id = createLogger(iter->second._key.c_str());
                if (id == LOG4Z_INVALID_LOGGER_ID)
                {
                    continue;
                }
            }
        }
        enableLogger(id, iter->second._enable);
        setLoggerName(id, iter->second._name.c_str());
        setLoggerPath(id, iter->second._path.c_str());
        setLoggerLevel(id, iter->second._level);
        setLoggerFileLine(id, iter->second._fileLine);
        setLoggerDisplay(id, iter->second._display);
        setLoggerOutFile(id, iter->second._outfile);
        setLoggerLimitsize(id, iter->second._limitsize);
        setLoggerMonthdir(id, iter->second._monthdir);
    }
    return true;
}

//! read configure and create with overwriting
bool LogerManager::config(const char* configPath)
{
    if (!_configFile.empty())
    {
		printf(" !!! !!! !!! !!!\r\n");
		printf(" !!! !!! log4z configure error: too many calls to Config. the old config file=%s,  the new config file=%s !!! !!! \r\n"
		, _configFile.c_str(), configPath);
		printf(" !!! !!! !!! !!!\r\n");
        return false;
    }
    _configFile = configPath;

    Log4zFileHandler f;
    f.open(_configFile.c_str(), "rb");
    if (!f.isOpen())
    {
		printf(" !!! !!! !!! !!!\r\n");
		printf(" !!! !!! log4z load config file error. filename=%s  !!! !!! \r\n", configPath);
		printf(" !!! !!! !!! !!!\r\n");
        return false;
    }
    return configFromStringImpl(f.readContent().c_str(), false);
}

//! read configure and create with overwriting
bool LogerManager::configFromString(const char* configContent)
{
    return configFromStringImpl(configContent, false);
}

//! create with overwriting
LoggerId LogerManager::createLogger(const char* key)
{
    if (key == NULL)
    {
        return LOG4Z_INVALID_LOGGER_ID;
    }
    
    std::string copyKey = key;
    trimLogConfig(copyKey);

    LoggerId newID = LOG4Z_INVALID_LOGGER_ID;
    {
        std::map<std::string, LoggerId>::iterator iter = _ids.find(copyKey);
        if (iter != _ids.end())
        {
            newID = iter->second;
        }
    }
    if (newID == LOG4Z_INVALID_LOGGER_ID)
    {
        if (_lastId +1 >= LOG4Z_LOGGER_MAX)
        {
            showColorText("log4z: CreateLogger can not create|writeover, because loggerid need < LOGGER_MAX! \r\n", LOG_LEVEL_FATAL);
            return LOG4Z_INVALID_LOGGER_ID;
        }
        newID = ++ _lastId;
        _ids[copyKey] = newID;
        _loggers[newID]._enable = true;
        _loggers[newID]._key = copyKey;
        _loggers[newID]._name = copyKey;
    }

    return newID;
}


bool LogerManager::start()
{
    if (_runing)
    {
        showColorText("log4z already start \r\n", LOG_LEVEL_FATAL);
        return false;
    }
    _semaphore.create(0);
    bool ret = ThreadHelper::start();
    return ret && _semaphore.wait(3000);
}
bool LogerManager::stop()
{
    if (_runing)
    {
        showColorText("log4z stopping \r\n", LOG_LEVEL_FATAL);
        _runing = false;
        wait();
        while (!_freeLogDatas.empty())
        {
            delete _freeLogDatas.back();
            _freeLogDatas.pop_back();
        }
        return true;
    }
    return false;
}
bool LogerManager::prePushLog(LoggerId id, int level)
{
    if (id < 0 || id > _lastId || !_runing || !_loggers[id]._enable)
    {
        return false;
    }
    if (level < _loggers[id]._level)
    {
        return false;
    }
    size_t count = _logs.size();

    if (count > LOG4Z_LOG_QUEUE_LIMIT_SIZE)
    {
        size_t rate = (count - LOG4Z_LOG_QUEUE_LIMIT_SIZE) * 100 / LOG4Z_LOG_QUEUE_LIMIT_SIZE;
        if (rate > 100)
        {
            rate = 100;
        }
        if ((size_t)rand() % 100 < rate)
        {
            if (rate > 50)
            {
                AutoLock l(_logLock);
                count = _logs.size();
            }
            if (count > LOG4Z_LOG_QUEUE_LIMIT_SIZE)
            {
                sleepMillisecond((unsigned int)(rate));
            }
        }
    }
    return true;
}
bool LogerManager::pushLog(LogData * pLog, const char * file, int line)
{
    // discard log
    if (pLog->_id < 0 || pLog->_id > _lastId || !_runing || !_loggers[pLog->_id]._enable)
    {
        freeLogData(pLog);
        return false;
    }

    //filter log
    if (pLog->_level < _loggers[pLog->_id]._level)
    {
        freeLogData(pLog);
        return false;
    }
    if (_loggers[pLog->_id]._fileLine && file)
    {
        const char * pNameEnd = file + strlen(file);
        const char * pNameBegin = pNameEnd;
        do
        {
            if (*pNameBegin == '\\' || *pNameBegin == '/') { pNameBegin++; break; }
            if (pNameBegin == file) { break; }
            pNameBegin--;
        } while (true);
        zsummer::log4z::Log4zStream ss(pLog->_content + pLog->_contentLen, LOG4Z_LOG_BUF_SIZE - pLog->_contentLen); 
        ss.writeChar(' ');
        ss.writeString(pNameBegin, pNameEnd - pNameBegin);
        ss.writeChar(':');
        ss.writeULongLong((unsigned long long)line);
        pLog->_contentLen += ss.getCurrentLen();
    }

    if (pLog->_contentLen +3 > LOG4Z_LOG_BUF_SIZE ) pLog->_contentLen = LOG4Z_LOG_BUF_SIZE - 3;
    pLog->_content[pLog->_contentLen + 0] = '\r';
    pLog->_content[pLog->_contentLen + 1] = '\n';
    pLog->_content[pLog->_contentLen + 2] = '\0';
    pLog->_contentLen += 2;


    if (_loggers[pLog->_id]._display && LOG4Z_ALL_SYNCHRONOUS_OUTPUT)
    {
        showColorText(pLog->_content, pLog->_level);
    }

    if (LOG4Z_ALL_DEBUGOUTPUT_DISPLAY && LOG4Z_ALL_SYNCHRONOUS_OUTPUT)
    {
#ifdef WIN32
        OutputDebugStringA(pLog->_content);
#endif
    }

    if (_loggers[pLog->_id]._outfile && LOG4Z_ALL_SYNCHRONOUS_OUTPUT)
    {
        AutoLock l(_logLock);
        if (openLogger(pLog))
        {
            _loggers[pLog->_id]._handle.write(pLog->_content, pLog->_contentLen);
            _loggers[pLog->_id]._curWriteLen += (unsigned int)pLog->_contentLen;
            closeLogger(pLog->_id);
            _ullStatusTotalWriteFileCount++;
            _ullStatusTotalWriteFileBytes += pLog->_contentLen;
        }
    }

    if (LOG4Z_ALL_SYNCHRONOUS_OUTPUT)
    {
        freeLogData(pLog);
        return true;
    }
    
    AutoLock l(_logLock);
    _logs.push_back(pLog);
    _ullStatusTotalPushLog ++;
    return true;
}

//! 查找ID
LoggerId LogerManager::findLogger(const char * key)
{
    std::map<std::string, LoggerId>::iterator iter;
    iter = _ids.find(key);
    if (iter != _ids.end())
    {
        return iter->second;
    }
    return LOG4Z_INVALID_LOGGER_ID;
}

bool LogerManager::hotChange(LoggerId id, LogDataType ldt, int num, const std::string & text)
{
    if (id <0 || id > _lastId) return false;
    if (text.length() >= LOG4Z_LOG_BUF_SIZE) return false;
    if (!_runing || LOG4Z_ALL_SYNCHRONOUS_OUTPUT)
    {
        return onHotChange(id, ldt, num, text);
    }
    LogData * pLog = makeLogData(id, LOG4Z_DEFAULT_LEVEL);
    pLog->_id = id;
    pLog->_type = ldt;
    pLog->_typeval = num;
    memcpy(pLog->_content, text.c_str(), text.length());
    pLog->_contentLen = (int)text.length();
    AutoLock l(_logLock);
    _logs.push_back(pLog);
    return true;
}

bool LogerManager::onHotChange(LoggerId id, LogDataType ldt, int num, const std::string & text)
{
    if (id < LOG4Z_MAIN_LOGGER_ID || id > _lastId)
    {
        return false;
    }
    LoggerInfo & logger = _loggers[id];
    if (ldt == LDT_ENABLE_LOGGER) logger._enable = num != 0;
    else if (ldt == LDT_SET_LOGGER_NAME) logger._name = text;
    else if (ldt == LDT_SET_LOGGER_PATH) logger._path = text;
    else if (ldt == LDT_SET_LOGGER_LEVEL) logger._level = num;
    else if (ldt == LDT_SET_LOGGER_FILELINE) logger._fileLine = num != 0;
    else if (ldt == LDT_SET_LOGGER_DISPLAY) logger._display = num != 0;
    else if (ldt == LDT_SET_LOGGER_OUTFILE) logger._outfile = num != 0;
    else if (ldt == LDT_SET_LOGGER_LIMITSIZE) logger._limitsize = num;
	else if (ldt == LDT_SET_LOGGER_MONTHDIR) logger._monthdir = num != 0;
	else if (ldt == LDT_SET_LOGGER_RESERVETIME) logger._logReserveTime = num >= 0 ? num : 0;
	return true;
}

bool LogerManager::enableLogger(LoggerId id, bool enable) 
{
    if (id < 0 || id > _lastId) return false;
    if (enable)
    {
        _loggers[id]._enable = true;
        return true;
    }
    return hotChange(id, LDT_ENABLE_LOGGER, false, ""); 
}
bool LogerManager::setLoggerLevel(LoggerId id, int level) 
{ 
    if (id < 0 || id > _lastId) return false;
    if (level <= _loggers[id]._level)
    {
        _loggers[id]._level = level;
        return true;
    }
    return hotChange(id, LDT_SET_LOGGER_LEVEL, level, ""); 
}
bool LogerManager::setLoggerDisplay(LoggerId id, bool enable) { return hotChange(id, LDT_SET_LOGGER_DISPLAY, enable, ""); }
bool LogerManager::setLoggerOutFile(LoggerId id, bool enable) { return hotChange(id, LDT_SET_LOGGER_OUTFILE, enable, ""); }
bool LogerManager::setLoggerMonthdir(LoggerId id, bool enable) { return hotChange(id, LDT_SET_LOGGER_MONTHDIR, enable, ""); }
bool LogerManager::setLoggerFileLine(LoggerId id, bool enable) { return hotChange(id, LDT_SET_LOGGER_FILELINE, enable, ""); }
bool LogerManager::setLoggerReserveTime(LoggerId id, time_t sec) { return hotChange(id, LDT_SET_LOGGER_RESERVETIME, (int)sec, ""); }
bool LogerManager::setLoggerLimitsize(LoggerId id, unsigned int limitsize)
{
    if (limitsize == 0 ) {limitsize = (unsigned int)-1;}
    return hotChange(id, LDT_SET_LOGGER_LIMITSIZE, limitsize, "");
}

bool LogerManager::setLoggerName(LoggerId id, const char * name)
{
    if (id <0 || id > _lastId) return false;
    //the name by main logger is the process name and it's can't change. 
//    if (id == LOG4Z_MAIN_LOGGER_ID) return false; 
    
    if (name == NULL || strlen(name) == 0) 
    {
        return false;
    }
    return hotChange(id, LDT_SET_LOGGER_NAME, 0, name);
}

bool LogerManager::setLoggerPath(LoggerId id, const char * path)
{
    if (id <0 || id > _lastId) return false;
    if (path == NULL || strlen(path) == 0)  return false;
    std::string copyPath = path;
    {
        char ch = copyPath.at(copyPath.length() - 1);
        if (ch != '\\' && ch != '/')
        {
            copyPath.append("/");
        }
    }
    return hotChange(id, LDT_SET_LOGGER_PATH, 0, copyPath);
}
bool LogerManager::setAutoUpdate(int interval)
{
    _hotUpdateInterval = interval;
    return true;
}
bool LogerManager::updateConfig()
{
    if (_configFile.empty())
    {
        //LOGW("log4z update config file error. filename is empty.");
        return false;
    }
    Log4zFileHandler f;
    f.open(_configFile.c_str(), "rb");
    if (!f.isOpen())
    {
		printf(" !!! !!! !!! !!!\r\n");
		printf(" !!! !!! log4z load config file error. filename=%s  !!! !!! \r\n", _configFile.c_str());
		printf(" !!! !!! !!! !!!\r\n");
        return false;
    }
    return configFromStringImpl(f.readContent().c_str(), true);
}

bool LogerManager::isLoggerEnable(LoggerId id)
{
    if (id <0 || id > _lastId) return false;
    return _loggers[id]._enable;
}

unsigned int LogerManager::getStatusActiveLoggers()
{
    unsigned int actives = 0;
    for (int i=0; i<= _lastId; i++)
    {
        if (_loggers[i]._enable)
        {
            actives ++;
        }
    }
    return actives;
}


bool LogerManager::openLogger(LogData * pLog)
{
    int id = pLog->_id;
    if (id < 0 || id >_lastId)
    {
        showColorText("log4z: openLogger can not open, invalide logger id! \r\n", LOG_LEVEL_FATAL);
        return false;
    }

    LoggerInfo * pLogger = &_loggers[id];
    if (!pLogger->_enable || !pLogger->_outfile || pLog->_level < pLogger->_level)
    {
        return false;
    }

    bool sameday = pLog->_time >= pLogger->_curFileCreateDay && pLog->_time - pLogger->_curFileCreateDay < 24*3600;
    bool needChageFile = pLogger->_curWriteLen > pLogger->_limitsize * 1024 * 1024;
    if (!sameday || needChageFile)
    {
        if (!sameday)
        {
            pLogger->_curFileIndex = 0;
        }
        else
        {
            pLogger->_curFileIndex++;
        }
        if (pLogger->_handle.isOpen())
        {
            pLogger->_handle.close();
        }
    }
    if (!pLogger->_handle.isOpen())
    {
        pLogger->_curFileCreateTime = pLog->_time;
        pLogger->_curWriteLen = 0;

        tm t = timeToTm(pLogger->_curFileCreateTime);
        if (true) //process day time   
        {
            tm day = t;
            day.tm_hour = 0;
            day.tm_min = 0;
            day.tm_sec = 0;
            pLogger->_curFileCreateDay = mktime(&day);
        }
        
        std::string name;
        std::string path;

        name = pLogger->_name;
        path = pLogger->_path;

        
        char buf[500] = { 0 };
        if (pLogger->_monthdir)
        {
            sprintf(buf, "%04d_%02d/", t.tm_year + 1900, t.tm_mon + 1);
            path += buf;
        }

        if (!isDirectory(path))
        {
            createRecursionDir(path);
        }
        if (LOG4Z_ALL_SYNCHRONOUS_OUTPUT)
        {
            sprintf(buf, "%s_%s_%04d%02d%02d%02d_%s_%03u.log",
                _proName.c_str(), name.c_str(), t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                t.tm_hour, _pid.c_str(), pLogger->_curFileIndex);
        }
        else
        {

            sprintf(buf, "%s_%s_%04d%02d%02d%02d%02d_%s_%03u.log",
                _proName.c_str(), name.c_str(), t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                t.tm_hour, t.tm_min, _pid.c_str(), pLogger->_curFileIndex);
        }
        path += buf;
        long curLen = pLogger->_handle.open(path.c_str(), "ab");
        if (!pLogger->_handle.isOpen() || curLen < 0)
        {
			sprintf(buf, "log4z: can not open log file %s. \r\n", path.c_str());
            showColorText("!!!!!!!!!!!!!!!!!!!!!!!!!! \r\n", LOG_LEVEL_FATAL);
            showColorText(buf, LOG_LEVEL_FATAL);
            showColorText("!!!!!!!!!!!!!!!!!!!!!!!!!! \r\n", LOG_LEVEL_FATAL);
            pLogger->_outfile = false;
            return false;
        }
        pLogger->_curWriteLen = (unsigned int)curLen;

		if (pLogger->_logReserveTime > 0 )
		{
			if (pLogger->_historyLogs.size() > LOG4Z_FORCE_RESERVE_FILE_COUNT)
			{
				while (!pLogger->_historyLogs.empty() && pLogger->_historyLogs.front().first < time(NULL) - pLogger->_logReserveTime)
				{
					pLogger->_handle.removeFile(pLogger->_historyLogs.front().second.c_str());
					pLogger->_historyLogs.pop_front();
				}
			}
            if (pLogger->_historyLogs.empty() || pLogger->_historyLogs.back().second != path)
            {
                pLogger->_historyLogs.push_back(std::make_pair(time(NULL), path));
            }
		}
        return true;
    }
    return true;
}
bool LogerManager::closeLogger(LoggerId id)
{
    if (id < 0 || id >_lastId)
    {
        showColorText("log4z: closeLogger can not close, invalide logger id! \r\n", LOG_LEVEL_FATAL);
        return false;
    }
    LoggerInfo * pLogger = &_loggers[id];
    if (pLogger->_handle.isOpen())
    {
        pLogger->_handle.close();
        return true;
    }
    return false;
}
bool LogerManager::popLog(LogData *& log)
{
    if (_logsCache.empty())
    {
        if (!_logs.empty())
        {
            AutoLock l(_logLock);
            if (_logs.empty())
            {
                return false;
            }
            _logsCache.swap(_logs);
        }
    }
    if (!_logsCache.empty())
    {
        log = _logsCache.front();
        _logsCache.pop_front();
        return true;
    }
    return false;
}

void LogerManager::run()
{
    _runing = true;
    LOGA("-----------------  log4z thread started!   ----------------------------");
    for (int i = 0; i <= _lastId; i++)
    {
        if (_loggers[i]._enable)
        {
            LOGA("logger id=" << i
                << " key=" << _loggers[i]._key
                << " name=" << _loggers[i]._name
                << " path=" << _loggers[i]._path
                << " level=" << _loggers[i]._level
                << " display=" << _loggers[i]._display);
        }
    }

    _semaphore.post();


    LogData * pLog = NULL;
    int needFlush[LOG4Z_LOGGER_MAX] = {0};
    time_t lastCheckUpdate = time(NULL);


    while (true)
    {
        while(popLog(pLog))
        {
            if (pLog->_id <0 || pLog->_id > _lastId)
            {
                freeLogData(pLog);
                continue;
            }
            LoggerInfo & curLogger = _loggers[pLog->_id];

            if (pLog->_type != LDT_GENERAL)
            {
                onHotChange(pLog->_id, (LogDataType)pLog->_type, pLog->_typeval, std::string(pLog->_content, pLog->_contentLen));
                curLogger._handle.close();
                freeLogData(pLog);
                continue;
            }
            
            //
            _ullStatusTotalPopLog ++;
            //discard
            
            if (!curLogger._enable || pLog->_level <curLogger._level  )
            {
                freeLogData(pLog);
                continue;
            }


            if (curLogger._display)
            {
                showColorText(pLog->_content, pLog->_level);
            }
            if (LOG4Z_ALL_DEBUGOUTPUT_DISPLAY )
            {
#ifdef WIN32
                OutputDebugStringA(pLog->_content);
#endif
            }


            if (curLogger._outfile )
            {
                if (!openLogger(pLog))
                {
                    freeLogData(pLog);
                    continue;
                }

                curLogger._handle.write(pLog->_content, pLog->_contentLen);
                curLogger._curWriteLen += (unsigned int)pLog->_contentLen;
                needFlush[pLog->_id] ++;
                _ullStatusTotalWriteFileCount++;
                _ullStatusTotalWriteFileBytes += pLog->_contentLen;
            }
            else 
            {
                _ullStatusTotalWriteFileCount++;
                _ullStatusTotalWriteFileBytes += pLog->_contentLen;
            }

            freeLogData(pLog);
        }

        for (int i=0; i<=_lastId; i++)
        {
            if (_loggers[i]._enable && needFlush[i] > 0)
            {
                _loggers[i]._handle.flush();
                needFlush[i] = 0;
            }
            if(!_loggers[i]._enable && _loggers[i]._handle.isOpen())
            {
                _loggers[i]._handle.close();
            }
        }

        //! delay. 
        sleepMillisecond(50);

        //! quit
        if (!_runing && _logs.empty())
        {
            break;
        }
        
        if (_hotUpdateInterval != 0 && time(NULL) - lastCheckUpdate > _hotUpdateInterval)
        {
            updateConfig();
            lastCheckUpdate = time(NULL);
        }
        


    }

    for (int i=0; i <= _lastId; i++)
    {
        if (_loggers[i]._enable)
        {
            _loggers[i]._enable = false;
            closeLogger(i);
        }
    }

}

//////////////////////////////////////////////////////////////////////////
//ILog4zManager::getInstance
//////////////////////////////////////////////////////////////////////////
ILog4zManager * ILog4zManager::getInstance()
{
    static LogerManager m;
    return &m;
}






_ZSUMMER_LOG4Z_END
_ZSUMMER_END

#endif //_ZSUMMER_LOG4Z_H_