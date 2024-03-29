/*******************
 * 
 * //  https://github.com/TonyXMH/log/
 * */



#ifndef __XMLOG_HPP__
#define __XMLOG_HPP__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>//getpid, gettid

#include <errno.h>
#include <unistd.h>//access, getpid
#include <assert.h>//assert
#include <stdarg.h>//va_list
#include <sys/stat.h>//mkdir
#include <sys/syscall.h>//system call

#define MEM_USE_LIMIT (3u * 1024 * 1024 * 1024)//3GB
#define LOG_USE_LIMIT (1u * 1024 * 1024 * 1024)//1GB
#define LOG_LEN_LIMIT (4 * 1024)//4K
#define RELOG_THRESOLD 5
#define BUFF_WAIT_TIME 1

enum LOG_LEVEL
{
    FATAL = 1,
    ERROR,
    WARN,
    INFO,
    DEBUG,
    TRACE,
};

extern pid_t gettid();

struct utc_timer
{
    utc_timer()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        //set _sys_acc_sec, _sys_acc_min
        _sys_acc_sec = tv.tv_sec;
        _sys_acc_min = _sys_acc_sec / 60;
        //use _sys_acc_sec calc year, mon, day, hour, min, sec
        struct tm cur_tm;
        localtime_r((time_t*)&_sys_acc_sec, &cur_tm);
        year = cur_tm.tm_year + 1900;
        mon  = cur_tm.tm_mon + 1;
        day  = cur_tm.tm_mday;
        hour  = cur_tm.tm_hour;
        min  = cur_tm.tm_min;
        sec  = cur_tm.tm_sec;
        reset_utc_fmt();
    }

    uint64_t get_curr_time(int* p_msec = NULL)
    {
        struct timeval tv;
        //get current ts
        gettimeofday(&tv, NULL);
        if (p_msec)
            *p_msec = tv.tv_usec / 1000;
        //if not in same seconds
        if ((uint32_t)tv.tv_sec != _sys_acc_sec)
        {
            sec = tv.tv_sec % 60;
            _sys_acc_sec = tv.tv_sec;
            //or if not in same minutes
            if (_sys_acc_sec / 60 != _sys_acc_min)
            {
                //use _sys_acc_sec update year, mon, day, hour, min, sec
                _sys_acc_min = _sys_acc_sec / 60;
                struct tm cur_tm;
                localtime_r((time_t*)&_sys_acc_sec, &cur_tm);
                year = cur_tm.tm_year + 1900;
                mon  = cur_tm.tm_mon + 1;
                day  = cur_tm.tm_mday;
                hour = cur_tm.tm_hour;
                min  = cur_tm.tm_min;
                //reformat utc format
                reset_utc_fmt();
            }
            else
            {
                //reformat utc format only sec
                reset_utc_fmt_sec();
            }
        }
        return tv.tv_sec;
    }

    int year, mon, day, hour, min, sec;
    char utc_fmt[20];

private:
    void reset_utc_fmt()
    {
        snprintf(utc_fmt, 20, "%d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min, sec);
    }
    
    void reset_utc_fmt_sec()
    {
        snprintf(utc_fmt + 17, 3, "%02d", sec);
    }

    uint64_t _sys_acc_min;
    uint64_t _sys_acc_sec;
};

class cell_buffer
{
public:
    enum buffer_status
    {
        FREE,
        FULL
    };

    cell_buffer(uint32_t len): 
    status(FREE), 
    prev(NULL), 
    next(NULL), 
    _total_len(len), 
    _used_len(0)
    {
        _data = new char[len];
        if (!_data)
        {
            fprintf(stderr, "no space to allocate _data\n");
            exit(1);
        }
    }

    uint32_t avail_len() const { return _total_len - _used_len; }

    bool empty() const { return _used_len == 0; }

    void append(const char* log_line, uint32_t len)
    {
        if (avail_len() < len)
            return ;
        memcpy(_data + _used_len, log_line, len);
        _used_len += len;
    }

    void clear()
    {
        _used_len = 0;
        status = FREE;
    }

    void persist(FILE* fp)
    {
        uint32_t wt_len = fwrite(_data, 1, _used_len, fp);
        if (wt_len != _used_len)
        {
            fprintf(stderr, "write log to disk error, wt_len %u\n", wt_len);
        }
    }

    buffer_status status;

    cell_buffer* prev;
    cell_buffer* next;

private:
    cell_buffer(const cell_buffer&);
    cell_buffer& operator=(const cell_buffer&);

    uint32_t _total_len;
    uint32_t _used_len;
    char* _data;
};

class log
{
public:
    //for thread-safe singleton
    static log* ins()
    {
        pthread_once(&_once, log::init);
        return _ins;
    }

    static void init()
    {
        while (!_ins) _ins = new log();
    }

    void init_path(const char* log_dir, const char* prog_name, int level);

    int get_level() const { return _level; }

    void persist();

    void try_append(const char* lvl, const char* format, ...);

private:
    log();

    bool decis_file(int year, int mon, int day);

    log(const log&);
    const log& operator=(const log&);

    int _buff_cnt;

    cell_buffer* _curr_buf;
    cell_buffer* _prst_buf;

    cell_buffer* last_buf;

    FILE* _fp;
    pid_t _pid;
    int _year, _mon, _day, _log_cnt;
    char _prog_name[128];
    char _log_dir[512];

    bool _env_ok;//if log dir ok
    int _level;
    uint64_t _lst_lts;//last can't log error time(s) if value != 0, log error happened last time
    
    utc_timer _tm;

    static pthread_mutex_t _mutex;
    static pthread_cond_t _cond;

    static uint32_t _one_buff_len;

    //singleton
    static log* _ins;
    static pthread_once_t _once;
};

void* be_thdo(void* args);

#define LOG_MEM_SET(mem_lmt) \
    do \
    { \
        if (mem_lmt < 90 * 1024 * 1024) \
        { \
            mem_lmt = 90 * 1024 * 1024; \
        } \
        else if (mem_lmt > 1024 * 1024 * 1024) \
        { \
            mem_lmt = 1024 * 1024 * 1024; \
        } \
        log::_one_buff_len = mem_lmt; \
    } while (0)

#define LOG_INIT(log_dir, prog_name, level) \
    do \
    { \
        log::ins()->init_path(log_dir, prog_name, level); \
        pthread_t tid; \
        pthread_create(&tid, NULL, be_thdo, NULL); \
        pthread_detach(tid); \
    } while (0)

//format: [LEVEL][yy-mm-dd h:m:s.ms][tid]file_name:line_no(func_name):content
#define LOG_TRACE(fmt, args...) \
    do \
    { \
        if (log::ins()->get_level() >= TRACE) \
        { \
            log::ins()->try_append("[TRACE]", "[%u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_DEBUG(fmt, args...) \
    do \
    { \
        if (log::ins()->get_level() >= DEBUG) \
        { \
            log::ins()->try_append("[DEBUG]", "[%u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_INFO(fmt, args...) \
    do \
    { \
        if (log::ins()->get_level() >= INFO) \
        { \
            log::ins()->try_append("[INFO]", "[%u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_NORMAL(fmt, args...) \
    do \
    { \
        if (log::ins()->get_level() >= INFO) \
        { \
            log::ins()->try_append("[INFO]", "[%u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_WARN(fmt, args...) \
    do \
    { \
        if (log::ins()->get_level() >= WARN) \
        { \
            log::ins()->try_append("[WARN]", "[%u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_ERROR(fmt, args...) \
    do \
    { \
        if (log::ins()->get_level() >= ERROR) \
        { \
            log::ins()->try_append("[ERROR]", "[%u]%s:%d(%s): " fmt "\n", \
                gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_FATAL(fmt, args...) \
    do \
    { \
        log::ins()->try_append("[FATAL]", "[%u]%s:%d(%s): " fmt "\n", \
            gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
    } while (0)

#define TRACE(fmt, args...) \
    do \
    { \
        if (log::ins()->get_level() >= TRACE) \
        { \
            log::ins()->try_append("[TRACE]", "[%u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define DEBUG(fmt, args...) \
    do \
    { \
        if (log::ins()->get_level() >= DEBUG) \
        { \
            log::ins()->try_append("[DEBUG]", "[%u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define INFO(fmt, args...) \
    do \
    { \
        if (log::ins()->get_level() >= INFO) \
        { \
            log::ins()->try_append("[INFO]", "[%u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define NORMAL(fmt, args...) \
    do \
    { \
        if (log::ins()->get_level() >= INFO) \
        { \
            log::ins()->try_append("[INFO]", "[%u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define WARN(fmt, args...) \
    do \
    { \
        if (log::ins()->get_level() >= WARN) \
        { \
            log::ins()->try_append("[WARN]", "[%u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define ERROR(fmt, args...) \
    do \
    { \
        if (log::ins()->get_level() >= ERROR) \
        { \
            log::ins()->try_append("[ERROR]", "[%u]%s:%d(%s): " fmt "\n", \
                gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define FATAL(fmt, args...) \
    do \
    { \
        log::ins()->try_append("[FATAL]", "[%u]%s:%d(%s): " fmt "\n", \
            gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
    } while (0)









pid_t gettid()
{
    return syscall(__NR_gettid);
}

pthread_mutex_t log::_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t log::_cond = PTHREAD_COND_INITIALIZER;

log* log::_ins = NULL;
pthread_once_t log::_once = PTHREAD_ONCE_INIT;
uint32_t log::_one_buff_len = 30*1024*1024;//30MB

log::log():
    _buff_cnt(3),
    _curr_buf(NULL),
    _prst_buf(NULL),
    _fp(NULL),
    _log_cnt(0),
    _env_ok(false),
    _level(INFO),
    _lst_lts(0),
    _tm()
{
    //create double linked list
    cell_buffer* head = new cell_buffer(_one_buff_len);
    if (!head)
    {
        fprintf(stderr, "no space to allocate cell_buffer\n");
        exit(1);
    }
    cell_buffer* current;
    cell_buffer* prev = head;
    for (int i = 1;i < _buff_cnt; ++i)
    {
        current = new cell_buffer(_one_buff_len);
        if (!current)
        {
            fprintf(stderr, "no space to allocate cell_buffer\n");
            exit(1);
        }
        current->prev = prev;
        prev->next = current;
        prev = current;
    }
    prev->next = head;
    head->prev = prev;

    _curr_buf = head;
    _prst_buf = head;

    _pid = getpid();
}

void log::init_path(const char* log_dir, const char* prog_name, int level)
{
    pthread_mutex_lock(&_mutex);

    strncpy(_log_dir, log_dir, 512);
    //name format:  name_year-mon-day-t[tid].log.n
    strncpy(_prog_name, prog_name, 128);

    mkdir(_log_dir, 0777);
    //查看是否存在此目录、目录下是否允许创建文件
    if (access(_log_dir, F_OK | W_OK) == -1)
    {
        fprintf(stderr, "logdir: %s error: %s\n", _log_dir, strerror(errno));
    }
    else
    {
        _env_ok = true;
    }
    if (level > TRACE)
        level = TRACE;
    if (level < FATAL)
        level = FATAL;
    _level = level;

    pthread_mutex_unlock(&_mutex);
}

void log::persist()
{
    while (true)
    {
        //check if _prst_buf need to be persist
        pthread_mutex_lock(&_mutex);
        if (_prst_buf->status == cell_buffer::FREE)
        {
            struct timespec tsp;
            struct timeval now;
            gettimeofday(&now, NULL);
            tsp.tv_sec = now.tv_sec;
            tsp.tv_nsec = now.tv_usec * 1000;//nanoseconds
            tsp.tv_sec += BUFF_WAIT_TIME;//wait for 1 seconds
            pthread_cond_timedwait(&_cond, &_mutex, &tsp);
        }
        if (_prst_buf->empty())
        {
            //give up, go to next turn
            pthread_mutex_unlock(&_mutex);
            continue;
        }

        if (_prst_buf->status == cell_buffer::FREE)
        {
            assert(_curr_buf == _prst_buf);//to test
            _curr_buf->status = cell_buffer::FULL;
            _curr_buf = _curr_buf->next;
        }

        int year = _tm.year, mon = _tm.mon, day = _tm.day;
        pthread_mutex_unlock(&_mutex);

        //decision which file to write
        if (!decis_file(year, mon, day))
            continue;
        //write
        _prst_buf->persist(_fp);
        fflush(_fp);

        pthread_mutex_lock(&_mutex);
        _prst_buf->clear();
        _prst_buf = _prst_buf->next;
        pthread_mutex_unlock(&_mutex);
    }
}

void log::try_append(const char* lvl, const char* format, ...)
{
    int ms;
    uint64_t curr_sec = _tm.get_curr_time(&ms);
    if (_lst_lts && curr_sec - _lst_lts < RELOG_THRESOLD)
        return ;

    char log_line[LOG_LEN_LIMIT];
    //int prev_len = snprintf(log_line, LOG_LEN_LIMIT, "%s[%d-%02d-%02d %02d:%02d:%02d.%03d]", lvl, _tm.year, _tm.mon, _tm.day, _tm.hour, _tm.min, _tm.sec, ms);
    int prev_len = snprintf(log_line, LOG_LEN_LIMIT, "%s[%s.%03d]", lvl, _tm.utc_fmt, ms);

    va_list arg_ptr;
    va_start(arg_ptr, format);

    //TO OPTIMIZE IN THE FUTURE: performance too low here!
    int main_len = vsnprintf(log_line + prev_len, LOG_LEN_LIMIT - prev_len, format, arg_ptr);

    va_end(arg_ptr);

    uint32_t len = prev_len + main_len;

    _lst_lts = 0;
    bool tell_back = false;

    pthread_mutex_lock(&_mutex);
    if (_curr_buf->status == cell_buffer::FREE && _curr_buf->avail_len() >= len)
    {
        _curr_buf->append(log_line, len);
    }
    else
    {
        //1. _curr_buf->status = cell_buffer::FREE but _curr_buf->avail_len() < len
        //2. _curr_buf->status = cell_buffer::FULL
        if (_curr_buf->status == cell_buffer::FREE)
        {
            _curr_buf->status = cell_buffer::FULL;//set to FULL
            cell_buffer* next_buf = _curr_buf->next;
            //tell backend thread
             tell_back = true;

            //it suggest that this buffer is under the persist job
            if (next_buf->status == cell_buffer::FULL)
            {
                //if mem use < MEM_USE_LIMIT, allocate new cell_buffer
                if (_one_buff_len * (_buff_cnt + 1) > MEM_USE_LIMIT)
                {
                    fprintf(stderr, "no more log space can use\n");
                    _curr_buf = next_buf;
                    _lst_lts = curr_sec;
                }
                else
                {
                    cell_buffer* new_buffer = new cell_buffer(_one_buff_len);
                    _buff_cnt += 1;
                    new_buffer->prev = _curr_buf;
                    _curr_buf->next = new_buffer;
                    new_buffer->next = next_buf;
                    next_buf->prev = new_buffer;
                    _curr_buf = new_buffer;
                }
            }
            else
            {
                //next buffer is free, we can use it
                _curr_buf = next_buf;
            }
            if (!_lst_lts)
                _curr_buf->append(log_line, len);
        }
        else//_curr_buf->status == cell_buffer::FULL, assert persist is on here too!
        {
            _lst_lts = curr_sec;
        }
    }
    pthread_mutex_unlock(&_mutex);
    if (tell_back)
    {
        pthread_cond_signal(&_cond);
    }
}

bool log::decis_file(int year, int mon, int day)
{
    //TODO: 是根据日志消息的时间写时间？还是自主写时间？  I select 自主写时间
    if (!_env_ok)
    {
        if (_fp)
            fclose(_fp);
        _fp = NULL;
        return false;
    }
    if (!_fp)
    {
        _year = year, _mon = mon, _day = day;
        char log_path[1024] = {};
        sprintf(log_path, "%s/%s.%d%02d%02d.%u.log", _log_dir, _prog_name, _year, _mon, _day, _pid);
        _fp = fopen(log_path, "w");
        if (_fp)
            _log_cnt += 1;
    }
    else if (_day != day)
    {
        fclose(_fp);
        char log_path[1024] = {};
        _year = year, _mon = mon, _day = day;
        sprintf(log_path, "%s/%s.%d%02d%02d.%u.log", _log_dir, _prog_name, _year, _mon, _day, _pid);
        _fp = fopen(log_path, "w");
        if (_fp)
            _log_cnt = 1;
    }
    else if (ftell(_fp) >= LOG_USE_LIMIT)
    {
        fclose(_fp);
        char old_path[1024] = {};
        char new_path[1024] = {};
        //mv xxx.log.[i] xxx.log.[i + 1]
        for (int i = _log_cnt - 1;i > 0; --i)
        {
            sprintf(old_path, "%s/%s.%d%02d%02d.%u.log.%d", _log_dir, _prog_name, _year, _mon, _day, _pid, i);
            sprintf(new_path, "%s/%s.%d%02d%02d.%u.log.%d", _log_dir, _prog_name, _year, _mon, _day, _pid, i + 1);
            rename(old_path, new_path);
        }
        //mv xxx.log xxx.log.1
        sprintf(old_path, "%s/%s.%d%02d%02d.%u.log", _log_dir, _prog_name, _year, _mon, _day, _pid);
        sprintf(new_path, "%s/%s.%d%02d%02d.%u.log.1", _log_dir, _prog_name, _year, _mon, _day, _pid);
        rename(old_path, new_path);
        _fp = fopen(old_path, "w");
        if (_fp)
            _log_cnt += 1;
    }
    return _fp != NULL;
}

void* be_thdo(void* args)
{
    log::ins()->persist();
    return NULL;
}




#endif  //__XMLOG_HPP__