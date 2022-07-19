/******************************************************************************* 
  *FileName: dlogger.hpp
  *version: 
  *Author: phae
  *Date: 2022-07-14 08:48:25
  *Description: 
  *LastEditors: LZH
  *LastEditTime: 2022-07-19 08:30:11
 ********************************************************************************/

#ifndef __DLOGGER_HPP__
#define __DLOGGER_HPP__
#ifndef FILEHELPER_H_
#define FILEHELPER_H_
#include <string>
#include<cstring>
#include <vector>
#include <fstream>
#include <stdio.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include "unistd.h"
#include <stdarg.h>
#include <sys/stat.h>
#endif

namespace FrameWork {
#ifdef _WIN32
#define ACCESS _access
#define MKDIR(a) _mkdir((a))
#else
#define ACCESS access
#define MKDIR(a) mkdir((a),0755)
#endif

class FileHelper {
public:
    static bool save(const std::string filename, std::string& content) {
        FILE *file = fopen(filename.c_str(), "wb");

        if (file == NULL)
            return false;
        fwrite(content.c_str(), sizeof(char), content.size(), file);
        fclose(file);
        return true;
    }

    // used to open binary file
    static bool open(const std::string filename, std::string& content) {
        FILE *file = fopen(filename.c_str(), "rb");

        if (file == NULL)
            return false;

        fseek(file, 0, SEEK_END);
        int len = ftell(file);
        rewind(file);
        content.clear();
        char *buffer = new char[len];
        fread(buffer, sizeof(char), len, file);
        content.assign(buffer, len);
        delete[] buffer;

        //int nRead;
        //content.clear();
        //char buffer[80];
        //while(!feof(file)){
        //  nRead = fread(buffer,sizeof(char),sizeof(buffer),file);
        //  if(nRead > 0){
        //      content.append(buffer);
        //  }
        //}
        fclose(file);
        return true;
    }

    // used to open text file
    static bool open(const std::string file_name,
            std::vector<std::string>& lines) {
        std::ifstream file(file_name.c_str(), std::ios::in);
        if (!file) {
            return false;
        }

        lines.clear();
        char buffer[BUFFER_SIZE];

        while (file.getline(buffer, BUFFER_SIZE, '\n')) {
            lines.push_back(buffer);
        }

        return true;
    }
    static bool CreateDir(const char *pszDir) {
        size_t i = 0;
        size_t iRet;
        size_t iLen = strlen(pszDir);
        char* buf = new char[iLen + 1];
        strncpy(buf, pszDir, iLen + 1);
        for (i = 0; i < iLen; i++) {
            if (pszDir[i] == '\\' || pszDir[i] == '/') {
                buf[i] = '\0';
                //如果不存在,创建
                iRet = ACCESS(buf, 0);
                if (iRet != 0) {
                    iRet = MKDIR(buf);
                    if (iRet != 0) {
                        delete[] buf;
                        return false;
                    }
                }
                //支持linux,将所有\换成/
                buf[i] = '/';
            }
        }
        delete[] buf;
        return true;
    }

private:

    enum {
        BUFFER_SIZE = 3000
    };

};

} /* namespace FrameWork */
#endif /* FILEHELPER_H_ */

/*
 * Mutex.h
 *
 *  Created on: 2016-01-07
 *      Author: Administrator
 */

#ifndef FRAME_LOG_MUTEX_H_
#define FRAME_LOG_MUTEX_H_

#include <pthread.h>
#include <stdlib.h>
namespace FrameWork {
typedef pthread_mutex_t MutexType;

class Mutex {
public:
    // Create a Mutex that is not held by anybody.  This constructor is
    // typically used for Mutexes allocated on the heap or the stack.
    // See below for a recommendation for constructing global Mutex
    // objects.
    inline Mutex();

    // Destructor
    inline ~Mutex();

    inline void Lock();    // Block if needed until free then acquire exclusively
    inline void Unlock();  // Release a lock acquired via Lock()
    inline bool TryLock(); // If free, Lock() and return true, else return false
    // Note that on systems that don't support read-write locks, these may
    // be implemented as synonyms to Lock() and Unlock().  So you can use
    // these for efficiency, but don't use them anyplace where being able
    // to do shared reads is necessary to avoid deadlock.
    inline void ReaderLock();   // Block until free or shared then acquire a share
    inline void ReaderUnlock(); // Release a read share of this Mutex
    inline void WriterLock() { Lock(); }     // Acquire an exclusive lock
    inline void WriterUnlock() { Unlock(); } // Release a lock from WriterLock()

    // TODO(hamaji): Do nothing, implement correctly.
    inline void AssertHeld() {}
private:
    MutexType mutex_;
    // We want to make sure that the compiler sets is_safe_ to true only
    // when we tell it to, and never makes assumptions is_safe_ is
    // always true.  volatile is the most reliable way to do that.
    volatile bool is_safe_;

    inline void SetIsSafe() { is_safe_ = true; }

    // Catch the error of writing Mutex when intending MutexLock.
    Mutex(Mutex* /*ignored*/) {}
    // Disallow "evil" constructors
    Mutex(const Mutex&);
    void operator=(const Mutex&);
};
#define SAFE_PTHREAD(fncall)  do {   /* run fncall if is_safe_ is true */  \
    if (is_safe_ && fncall(&mutex_) != 0) abort();                           \
    } while (0)

    Mutex::Mutex()             {
        SetIsSafe();
        if (is_safe_ && pthread_mutex_init(&mutex_, NULL) != 0) abort();
    }
    Mutex::~Mutex()            { SAFE_PTHREAD(pthread_mutex_destroy); }
    void Mutex::Lock()         { SAFE_PTHREAD(pthread_mutex_lock); }
    void Mutex::Unlock()       { SAFE_PTHREAD(pthread_mutex_unlock); }
    bool Mutex::TryLock()      { return is_safe_ ?
        pthread_mutex_trylock(&mutex_) == 0 : true; }
    void Mutex::ReaderLock()   { Lock(); }
    void Mutex::ReaderUnlock() { Unlock(); }
    class MutexLock {
    public:
        explicit MutexLock(Mutex *mu) : mu_(mu) { mu_->Lock(); }
        ~MutexLock() { mu_->Unlock(); }
    private:
        Mutex * const mu_;
        // Disallow "evil" constructors
        MutexLock(const MutexLock&);
        void operator=(const MutexLock&);
    };

    // ReaderMutexLock and WriterMutexLock do the same, for rwlocks
    class ReaderMutexLock {
    public:
        explicit ReaderMutexLock(Mutex *mu) : mu_(mu) { mu_->ReaderLock(); }
        ~ReaderMutexLock() { mu_->ReaderUnlock(); }
    private:
        Mutex * const mu_;
        // Disallow "evil" constructors
        ReaderMutexLock(const ReaderMutexLock&);
        void operator=(const ReaderMutexLock&);
    };

    class WriterMutexLock {
    public:
        explicit WriterMutexLock(Mutex *mu) : mu_(mu) { mu_->WriterLock(); }
        ~WriterMutexLock() { mu_->WriterUnlock(); }
    private:
        Mutex * const mu_;
        // Disallow "evil" constructors
        WriterMutexLock(const WriterMutexLock&);
        void operator=(const WriterMutexLock&);
    };
} /* namespace FrameWork */

#endif /* FRAME_LOG_MUTEX_H_ */

#ifndef FRAME_LOG_LOGGER_H_
#define FRAME_LOG_LOGGER_H_
#include <iostream>
#include<fstream>
#include<string>

using namespace std;

const int MaxFilePathLen = 1024;
const char PathSplitChar = '/';
namespace FrameWork {
enum LogLevel{
    /// <summary>
    /// 调试
    /// </summary>
    DEBUG = -1,
    /// <summary>
    /// 普通日志
    /// </summary>
    INFO = 0,
    /// <summary>
    /// 警告
    /// </summary>
    WARN,
    /// <summary>
    /// 错误
    /// </summary>
    ERROR,
    /// <summary>
    /// 崩溃
    /// </summary>
    FATAL,
    /// <summary>
    /// 超出错误级别
    /// </summary>
    OFF
};


class ILogger {
public:
    //
    virtual ~ILogger() {
    }

//利用宏定义，来简化代码输出，值得推荐
#define ABSTRACT_LOG_FUNC(name) \
    virtual void name(string msg)=0; \
    virtual void name(const char* fmt,...)=0;

    ABSTRACT_LOG_FUNC(Debug)
    ABSTRACT_LOG_FUNC(Info)
    ABSTRACT_LOG_FUNC(Warn)
    ABSTRACT_LOG_FUNC(Error)
    ABSTRACT_LOG_FUNC(Fatal)

#undef ABSTRACT_LOG_FUNC
#define ABSTRACT_LOG_FUNC_X(name) \
    virtual void name(LogLevel lv,string msg)=0; \
    virtual void name(LogLevel lv,const char* fmt,...)=0;\
    virtual void name(const char* file,int line,LogLevel lv,string msg)=0;\
    virtual void name(const char* file,int line,LogLevel lv,const char* fmt,...)=0;
    ABSTRACT_LOG_FUNC_X(Log)

#undef LOG_FUNC_X
};

class DLogger: public ILogger {
    std::string logPath;
    std::string logPrefix;
    std::fstream logFile;
    LogLevel level;
    Mutex mutex;

    DLogger(LogLevel level, char * folder, char * prefix);

public:
    static DLogger& GetInstance();
    static DLogger* GetInstancePtr();
    virtual ~DLogger();
    inline fstream & stream() {
        return logFile;
    }

#define DECLARE_LOG_FUNC(name) \
    virtual void name(string msg); \
    virtual void name(const char* fmt,...);

#define DECLARE_LOG_FUNC_X(name) \
    virtual void name(LogLevel lv,string msg); \
    virtual void name(LogLevel lv,const char* fmt,...);\
    virtual void name(const char* file,int line,LogLevel lv,string msg);\
    virtual void name(const char* file,int line,LogLevel lv,const char* fmt,...);

    DECLARE_LOG_FUNC(Debug)
    DECLARE_LOG_FUNC(Info)
    DECLARE_LOG_FUNC(Warn)
    DECLARE_LOG_FUNC(Error)
    DECLARE_LOG_FUNC(Fatal)

    DECLARE_LOG_FUNC_X(Log)

#undef DECLARE_LOG_FUNC_X
#undef DECLARE_LOG_FUNC

};

class LogMessage {
    DLogger* logger;
    static Mutex mutex;
public:
    LogMessage(const char* file, int line, LogLevel lv);
    LogMessage(LogLevel lv);
    ostream& stream() {
        return logger->stream();
    }
    virtual ~LogMessage();
};

void InitLogging(const char* filename, LogLevel minlevel,
        const char* destFolder);
void CloseLogging();

#define LOG(level) LogMessage(__FILE__, __LINE__,level).stream()

#define LOGFILE(level) LogMessage(level).stream()

#define LOG_IF(severity, condition) \
    !(condition) ? (void) 0 : LOG(severity)
#define LOG_ASSERT(condition)  \
    LOG_IF(FATAL, !(condition)) << "Assert failed: " #condition
#define CHECK(condition)  \
    LOG_IF(FATAL, !(condition)) \
    << "Check failed: " #condition " "

} /* namespace FrameWork */

#endif /* FRAME_LOG_LOGGER_H_ */



#ifndef FRAME_LOG_LOGGER_CC_
#define FRAME_LOG_LOGGER_CC_



#include<cstring>
#include<time.h>
#include<cstdarg>
#include<cstdlib>
#include<assert.h>

namespace FrameWork {
Mutex LogMessage::mutex;
static char _defaltFolder[]="/var/tmp/";
static char _appName[MaxFilePathLen];
static char _appFolder[MaxFilePathLen];
static char _destFolder[MaxFilePathLen];
static char _destPrefix[MaxFilePathLen];
static LogLevel _destLevel;
static char _levelInfos[][16]={
    "Debug","Info","Warn","Error","Fatal"
};
const int BUFFER_SIZE = 8196;
static char _gBuffer[BUFFER_SIZE];
void combine_folder(char** destpath, char* basefolder,char* relativefolder)
{
    int lenb = strlen(basefolder);
    int lenr = strlen(relativefolder);
    char* pret = (char*)malloc((lenb+lenr+1)*sizeof(char));
    int pos=lenb-1;
    memset(pret,0,lenb+lenr+1);
    while(pos>0 && ( basefolder[pos]!='/'))
        pos--;
    strncpy(*destpath,basefolder,pos+1);
    if(relativefolder[0] == '\\' || relativefolder[0] == '/'){
        strncpy(*destpath+pos+1,relativefolder+1,lenr-1);
    }else{
        strncpy(*destpath+pos+1,relativefolder,lenr);
    }
}

static void InitPaths(const char* filename,const char* destFolder)
{
    memset(_appName,0,MaxFilePathLen);
    memset(_appFolder,0,MaxFilePathLen);
    memset(_destFolder,0,MaxFilePathLen);
    memset(_destPrefix,0,MaxFilePathLen);

    strcpy(_appName,filename);
    int len = strlen(filename),lend;
    int pos = len-1,posd,start;
    while(pos >0 && filename[pos] != PathSplitChar)
        pos--;
    strncpy(_appFolder,filename,pos+1);
    lend = strlen(destFolder);
    posd = lend-1;
    if(destFolder[lend-1] != PathSplitChar) {
        //has prefix
        while(posd >0 && destFolder[posd] != PathSplitChar)
            posd--;
    }
    if(destFolder[0] == '.' && destFolder[1] == PathSplitChar){
        strncpy(_destFolder,filename,pos+1);
        start = 2;
    }   else{
        pos = 8;
        strcpy(_destFolder,_defaltFolder);
        if(destFolder[0] != PathSplitChar){
            start = 0;
        }else{
            start = 1;
        }
    }
    strncpy(_destFolder+pos+1,destFolder+start,posd-start+1);
    strncpy(_destPrefix,filename,pos+1);
    strncpy(_destPrefix+pos+1,destFolder+start,lend-start);
}

void InitLogging(const char* filename,LogLevel minlevel,const char* destFolder)
{
    InitPaths(filename,destFolder);
    _destLevel = minlevel;
}



static string GetLocalDate(void)
{
    time_t t = time(0);
    tm *ld;
    char tmp[64] = "";
    ld=localtime(&t);
    strftime(tmp,sizeof(tmp),"%Y-%m-%d",ld);
    return string(tmp);
}
static string GetCurTime(void)
{
    time_t t = time(0);
    tm *ld;
    char tmp[64] = "";
    ld=localtime(&t);
    strftime(tmp,sizeof(tmp),"%Y-%m-%d %H:%M:%S",ld);
    return string(tmp);
}

DLogger::DLogger(LogLevel level,char * folder,char * prefix)
    :level(level)
{
    std::string path;
    path.append(prefix);
    path.append(GetLocalDate());
    path.append(".log");
    FileHelper::CreateDir(folder);
    logPrefix.append(prefix);
    logPath = path;
    logFile.open(path.c_str(),ios::app|ios::out);
    // logFile<<"Log file created at:"<<GetCurTime()<<endl;
}

DLogger::~DLogger() {
    logFile.close();
}

#define IMPLEMENT_LOG_FUNC1(cname,fname,lv) \
void cname::fname(string msg) {\
    if(level <= lv){\
        WriterMutexLock lock(&mutex);\
        logFile<<"["<<GetCurTime().c_str()<<"][" #lv "]"<<msg.c_str()<<endl;\
        logFile.flush();\
    }\
}

#define PRINT_ARGS_TO_BUFFER(fmt,buf) \
    {\
        memset(buf,0,sizeof(buf));\
        va_list argp;\
        va_start(argp,fmt);\
        vsprintf(buf,fmt,argp);\
        va_end(argp);\
    }

#define IMPLEMENT_LOG_FUNC2(cname,fname,lv) \
void cname::fname(const char* format,...) {\
    if(level <= lv){\
        WriterMutexLock lock(&mutex);\
        PRINT_ARGS_TO_BUFFER(format,_gBuffer)\
        logFile<<"["<<GetCurTime().c_str()<<"][" #lv "]"<<_gBuffer<<endl;\
        logFile.flush();\
    }\
}


#define IMPLEMENT_LOG_FUNC(cname,fname,lv) \
IMPLEMENT_LOG_FUNC1(cname,fname,lv)\
IMPLEMENT_LOG_FUNC2(cname,fname,lv)

IMPLEMENT_LOG_FUNC(DLogger,Debug,DEBUG)
IMPLEMENT_LOG_FUNC(DLogger,Info,INFO)
IMPLEMENT_LOG_FUNC(DLogger,Warn,WARN)
IMPLEMENT_LOG_FUNC(DLogger,Error,ERROR)
IMPLEMENT_LOG_FUNC(DLogger,Fatal,FATAL)

DLogger& DLogger::GetInstance() {
    static DLogger _logger(_destLevel,_destFolder,_destPrefix);
    return _logger;
}

void DLogger::Log(LogLevel lv, string msg) {
    if(level <= lv){
        WriterMutexLock lock(&mutex);
        logFile<<"["<<GetCurTime().c_str()<<"]["<<_levelInfos[lv+1]<<"]"<<msg.c_str()<<endl;
        logFile.flush();
    }
}

void DLogger::Log(LogLevel lv, const char* format,...) {
    if(level <= lv){
        WriterMutexLock lock(&mutex);
        PRINT_ARGS_TO_BUFFER(format,_gBuffer)
        logFile<<"["<<GetCurTime().c_str()<<"]["<<_levelInfos[lv+1]<<"]"<<_gBuffer<<endl;
        logFile.flush();
    }
}

void DLogger::Log(const char* file, int line, LogLevel lv, string msg) {
    if(level <= lv){
        WriterMutexLock lock(&mutex);
        logFile<<"["<<GetCurTime().c_str()<<"]["<<_levelInfos[lv+1]<<"]["<<file<<"]["<<line<<"]"<<msg.c_str();
        logFile.flush();
    }
}

DLogger* DLogger::GetInstancePtr() {
    return &GetInstance();
}

void DLogger::Log(const char* file, int line, LogLevel lv, const char* format,...) {
    if(level <= lv){
        WriterMutexLock lock(&mutex);
        PRINT_ARGS_TO_BUFFER(format,_gBuffer)
        logFile<<"["<<GetCurTime().c_str()<<"]["<<_levelInfos[lv+1]<<"]["<<file<<"]["<<line<<"]"<<_gBuffer;
        logFile.flush();
    }
}

LogMessage::LogMessage(const char* file, int line, LogLevel lv) {
    logger = DLogger::GetInstancePtr();
    mutex.Lock();
    logger->Log(file,line,lv,"");
}

LogMessage::LogMessage(LogLevel lv) {
    logger = DLogger::GetInstancePtr();
    mutex.Lock();
    logger->Log(lv,"");
}

LogMessage::~LogMessage() {
    logger->stream()<<endl;
    logger->stream().flush();
    mutex.Unlock();
}

} /* namespace FrameWork */

#endif  //  FRAME_LOG_LOGGER_CC_


#endif  // __DLOGGER_HPP__



/**********************************************************************
*     
*     

#include <iostream>
#include "dlogger.hpp"
using namespace std;
using namespace FrameWork;
int main(int argc, char* argv[]) {
    InitLogging(argv[0], INFO, "./log/test");
    cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
    LOG(INFO) << "info test";
    LOG(WARN) << "WARN TEST %d" << 20;
    LOG(ERROR) << "Error test %d %s" << 20 << "nihao";

    DLogger::GetInstance().Error("error test common");
    DLogger::GetInstance().Fatal("fatal test common %d ", 100);
    DLogger::GetInstance().Info("info test normal %d %s ", 50, "zhongguoren");
    return 0;
}

*     
**********************************************************************/