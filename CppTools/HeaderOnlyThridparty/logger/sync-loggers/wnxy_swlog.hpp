
/*****
 * 
 * https://github.com/wnxy/LogLibrary/
 * */

#ifndef WNXY_SWLOG_HPP
#define WNXY_SWLOG_HPP

#include <cstdarg>
#include <vector>
#include <string>

/**
 * @brief String format output
 *
 * @param format
 * @param args
 * @return std::string
 */
inline std::string format_string(const char *format, va_list args)
{
    constexpr size_t oldlen = BUFSIZ;
    char buffer[oldlen]; // Buffers on the default stack
    va_list argscopy;
    va_copy(argscopy, args);
    size_t newlen = vsnprintf(&buffer[0], oldlen, format, args) + 1;
    newlen++;            // Add terminator '\0'
    if (newlen > oldlen) // Buffer on the default stack isn't enough, allocate from the heap
    {
        std::vector<char> newbuffer(newlen);
        vsnprintf(newbuffer.data(), newlen, format, argscopy);
        return newbuffer.data();
    }
    return buffer;
}

/**
 * @brief String format output
 *
 * @param format
 * @param ...
 * @return std::string
 */
inline std::string format_string(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    auto s = format_string(format, args);
    va_end(args);

    return s;
}



/**
 * @file swlog.h
 * @author XiaoYin Niu (you@domain.com)
 * @brief Header file of software library, the logs are printed synchronously, 
 * and the average printing time of each log is about 80ms.
 * @version 0.1
 * @date 2022-01-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef _SWLOG_H_
#define _SWLOG_H_

#ifdef _WIN64
#include <Windows.h>
#include <tchar.h>
#include <direct.h>
#include <io.h>
#elif __linux__
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#endif

#include <string>
#include <mutex>

typedef unsigned long DWORD;

// log level
enum SWLOG_LEVEL
{
    LOG_NONE,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
};

#ifdef _MSC_VER

#define LOG_INFO(...)    SWLog::Log(LOG_INFO, __FILE__, __FUNCSIG__, __LINE__, __VA_ARGS__);
#define LOG_WARNING(...) SWLog::Log(LOG_WARNING, __FILE__, __FUNCSIG__, __LINE__, __VA_ARGS__);
#define LOG_ERROR(...)   SWLog::Log(LOG_ERROR, __FILE__, __FUNCSIG__, __LINE__, __VA_ARGS__);

#elif __GNUC__

#define LOG_INFO(...)    SWLog::Log(LOG_INFO, __FILE__, __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__);
#define LOG_WARNING(...) SWLog::Log(LOG_WARNING, __FILE__, __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__);
#define LOG_ERROR(...)   SWLog::Log(LOG_ERROR, __FILE__, __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__);

#endif

#ifdef _WIN64
#define _PCSTR_ PCSTR
#elif __linux__
#define PATH_SIZE 255
#define _PCSTR_ const char *
#endif

/**
 * @brief Software log encapsulation class, realize log file initialization,
 *  log level definition, log printing, etc.
 */
class SWLog
{
public:
    static bool Init(bool bToFile, bool bTruncateLongLog, _PCSTR_ c_cLogFileName = nullptr);
    static void UnInit();
    // ANSI
    // static bool Log(long nLevel, PCSTR pszFileName, PCSTR pszFunctionSig, long nLineNo, PCTSTR pszFmt, ...);
    // Unicode
    static bool Log(long nLevel, _PCSTR_ pszFileName, _PCSTR_ pszFunctionSig, long nLineNo, _PCSTR_ pszFmt, ...);

private:
    SWLog() = delete;
    ~SWLog() = delete;

    SWLog(const SWLog &rhs) = delete;
    SWLog &operator=(const SWLog &rhs) = delete;

    static std::string GetLogTime();

private:
    static bool m_bToFile;          // Control logging to file or console
    static bool m_bTruncateLongLog; // Whether to truncate long logs
#ifdef _WIN64
    static HANDLE m_hLogFile; // Log file handle
#elif __linux__
    static int m_iLogFile; // Log file state(approximately equal to handle)
#endif
    static SWLOG_LEVEL m_nLogLevel; // Log level
};

#endif // _SWLOG_H_


/**
 * @file SWLog.cpp
 * @author XiaoYin Niu (you@domain.com)
 * @brief Source file of software library, the logs are printed synchronously, 
 * and the average printing time of each log is about 80ms.
 * @version 0.1
 * @date 2022-01-11
 *
 * @copyright Copyright (c) 2022
 *
 */


#ifndef LOG_OUTPUT
#define LOG_OUTPUT
#endif

#define MAX_LINE_LENGTH 256

bool SWLog::m_bToFile = false;
bool SWLog::m_bTruncateLongLog = false;
#ifdef _WIN64
HANDLE SWLog::m_hLogFile = INVALID_HANDLE_VALUE;
#elif __linux__
int SWLog::m_iLogFile = -1;
#endif
SWLOG_LEVEL SWLog::m_nLogLevel = LOG_NONE;

std::mutex mut;

/**
 * @brief Log file initialier.
 * If the first parameter specifies the output log to the console, the third parameter can be omitted.
 * @param bToFile Define logging to file or console
 * @param bTruncateLongLog Truncate long log true/false
 * @param c_cLogFileName Log filename
 * @return true
 * @return false
 */
bool SWLog::Init(bool bToFile, bool bTruncateLongLog, _PCSTR_ c_cLogFileName)
{
#ifdef LOG_OUTPUT
    m_bToFile = bToFile;
    m_bTruncateLongLog = bTruncateLongLog;

    if (bToFile == true && c_cLogFileName == nullptr)
    {
        return false;
    }
    else if(bToFile == false)
    {
        return true;
    }
    else
    {
#ifdef _WIN64
        TCHAR fileDirectory[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, fileDirectory);

        // Log directory
        std::string logFileDirectory = format_string("%s%s", fileDirectory, "\\Log\\");
        if (_access(logFileDirectory.c_str(), 0) == -1)
        {
            _mkdir(logFileDirectory.c_str());
        }
#elif __linux__
        char fileDirectory[PATH_SIZE];
        getcwd(fileDirectory, PATH_SIZE);

        // Log directory
        std::string logFileDirectory = format_string("%s%s", fileDirectory, "/Log/");
        if (access(logFileDirectory.c_str(), F_OK) == -1)
        {
            mkdir(logFileDirectory.c_str(), S_IRWXU);
        }
#endif
        std::string logFileName = format_string("%s%s", logFileDirectory.c_str(), c_cLogFileName);
#ifdef _WIN64
        m_hLogFile = CreateFile(logFileName.c_str(),
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (m_hLogFile == INVALID_HANDLE_VALUE)
        {
            return false;
        }
#elif __linux__
        m_iLogFile = open(logFileName.c_str(), O_CREAT | O_APPEND | O_RDWR, S_IRWXU);
        if (m_iLogFile == -1)
        {
            return false;
        }
#endif
    }
    return true;
#endif // LOG_OUTPUT
}

/**
 * @brief Close log file handle
 *
 */
void SWLog::UnInit()
{
#ifdef LOG_OUTPUT
#ifdef _WIN64
    if (m_hLogFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hLogFile);
        m_hLogFile = INVALID_HANDLE_VALUE;
    }
#elif __linux__
    if (m_iLogFile != -1)
    {
        close(m_iLogFile);
        m_iLogFile = -1;
    }
#endif
#endif // LOG_OUTPUT
}

/**
 * @brief Get current time for print log
 * 
 * @return std::string 
 */
std::string SWLog::GetLogTime()
{
#ifdef LOG_OUTPUT
#ifdef _WIN64
    SYSTEMTIME st = {0};
    GetLocalTime(&st);
    // Output format control: %04d Output in four-digit digital format, zeros are added to the left of less than 4 digits
    std::string strTime = format_string("[%04d-%02d-%02d %02d:%02d:%02d %04d]",
                                        st.wYear,
                                        st.wMonth,
                                        st.wDay,
                                        st.wHour,
                                        st.wMinute,
                                        st.wSecond,
                                        st.wMilliseconds);
#elif __linux__
    struct timeval tv;
    gettimeofday(&tv, NULL);
    // millisecond
    int millis = tv.tv_usec / 1000;
    struct tm nowTime;
    localtime_r(&tv.tv_sec, &nowTime);
    char buffer[80] = {0};
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &nowTime);
    std::string strTime = format_string("[%s %04d]", buffer, millis);
#endif
    return strTime;
#endif // LOG_OUTPUT
}

/**
 * @brief Log output interface
 *
 * @param nLevel Log level
 * @param pszFileName Current filename
 * @param pszFunctionSig Current function name
 * @param nLineNo Current lineNo
 * @param pszFmt Log message
 * @param ...
 * @return true Write log succeed
 * @return false Write log failed
 */
bool SWLog::Log(long nLevel, _PCSTR_ pszFileName, _PCSTR_ pszFunctionSig, long nLineNo, _PCSTR_ pszFmt, ...)
{
#ifdef LOG_OUTPUT
    if (nLevel <= m_nLogLevel)
    {
        return false;
    }
    std::string strLogLevel;
    if (nLevel == LOG_INFO)
    {
        strLogLevel = "[INFO]";
    }
    else if (nLevel == LOG_WARNING)
    {
        strLogLevel = "[WARNING]";
    }
    else if (nLevel == LOG_ERROR)
    {
        strLogLevel = "[ERROR]";
    }
    std::string strLogInfo = format_string("%s %s", GetLogTime().c_str(), strLogLevel.c_str());
    // Capture current thread ID
#ifdef _WIN64
    DWORD dwThreadID = GetCurrentThreadId();
#elif __linux__
    DWORD dwThreadID = pthread_self();
#endif
    strLogInfo = format_string("%s [ThreadID: %u] [%s Line: %u] [Function: %s] Message: ",
                               strLogInfo.c_str(), dwThreadID, pszFileName, nLineNo, pszFunctionSig);
    // Log message
    std::string strLogMsg;
    va_list ap;
    va_start(ap, pszFmt);
    strLogMsg = format_string(pszFmt, ap);
    va_end(ap);

    // If the log allows truncation, the long log only takes the first MAX_LINE_LENGTH characters
    if (m_bTruncateLongLog)
    {
        strLogMsg = strLogMsg.substr(0, MAX_LINE_LENGTH);
    }
    strLogInfo += strLogMsg;
    strLogInfo += "\r\n";
#ifdef _WIN64
    if (m_bToFile)
    {
        if (m_hLogFile == INVALID_HANDLE_VALUE)
        {
            return false;
        }
        {
            std::lock_guard<std::mutex> mtxLocker(mut);
            SetFilePointer(m_hLogFile, 0, NULL, FILE_END);
            DWORD dwByteWritten = 0;
            WriteFile(m_hLogFile, strLogInfo.c_str(), strLogInfo.length(), &dwByteWritten, NULL);
            FlushFileBuffers(m_hLogFile);
        }
        return true;
    }
    // Output the log to console in software release state
    std::lock_guard<std::mutex> mtxLocker(mut);
    OutputDebugStringA(strLogInfo.c_str());
#elif __linux__
    if (m_bToFile)
    {
        if (m_iLogFile == -1)
        {
            return false;
        }
        {
            std::lock_guard<std::mutex> mtxLocker(mut);
            lseek(m_iLogFile, 0, SEEK_SET);
            int size = write(m_iLogFile, strLogInfo.c_str(), strLogInfo.length());
            fsync(m_iLogFile);
        }
        return true;
    }

    // Output the log to console in software release state
    std::lock_guard<std::mutex> mtxLocker(mut);
    printf("%s", strLogInfo.c_str());
#endif

    return true;
#endif // LOG_OUTPUT
}



#endif  //WNXY_SWLOG_HPP