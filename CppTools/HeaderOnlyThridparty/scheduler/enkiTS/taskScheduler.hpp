// Copyright (c) 2013 Doug Binks
// 
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
// 
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgement in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

/*************************************************************************************
 * This file has been modified.
 * https://github.com/dougbinks/enkiTS
 * 
 *************************************************************************************/

#pragma once

#ifndef _TASKSCHEDULER_HPP_
#define _TASKSCHEDULER_HPP_

#include <atomic>
#include <thread>
#include <condition_variable>
#include <stdint.h>
#include <functional>

// ENKITS_TASK_PRIORITIES_NUM can be set from 1 to 5.
// 1 corresponds to effectively no priorities.
#ifndef ENKITS_TASK_PRIORITIES_NUM
    #define ENKITS_TASK_PRIORITIES_NUM 3
#endif

#ifndef	ENKITS_API
#if   defined(_WIN32) && defined(ENKITS_BUILD_DLL)
    // Building enkiTS as a DLL
    #define ENKITS_API __declspec(dllexport)
#elif defined(_WIN32) && defined(ENKITS_DLL)
    // Using enkiTS as a DLL
    #define ENKITS_API __declspec(dllimport)
#elif defined(__GNUC__) && defined(ENKITS_BUILD_DLL)
    // Building enkiTS as a shared library
    #define ENKITS_API __attribute__((visibility("default")))
#else
    #define ENKITS_API
#endif
#endif

// Define ENKI_CUSTOM_ALLOC_FILE_AND_LINE (at project level) to get file and line report in custom allocators,
// this is default in Debug - to turn off define ENKI_CUSTOM_ALLOC_NO_FILE_AND_LINE
#ifndef ENKI_CUSTOM_ALLOC_FILE_AND_LINE
#if defined(_DEBUG ) && !defined(ENKI_CUSTOM_ALLOC_NO_FILE_AND_LINE)
#define ENKI_CUSTOM_ALLOC_FILE_AND_LINE
#endif
#endif

#ifndef ENKI_ASSERT
#include <assert.h>
#define ENKI_ASSERT(x) assert(x)
#endif

namespace enki
{

    struct TaskSetPartition
    {
        uint32_t start;
        uint32_t end;
    };

    class  TaskScheduler;
    class  TaskPipe;
    class  PinnedTaskList;
    class  Dependency;
    struct ThreadArgs;
    struct ThreadDataStore;
    struct SubTaskSet;
    struct semaphoreid_t;

    uint32_t GetNumHardwareThreads();

    enum TaskPriority
    {
        TASK_PRIORITY_HIGH   = 0,
#if ( ENKITS_TASK_PRIORITIES_NUM > 3 )
        TASK_PRIORITY_MED_HI,
#endif
#if ( ENKITS_TASK_PRIORITIES_NUM > 2 )
        TASK_PRIORITY_MED,
#endif
#if ( ENKITS_TASK_PRIORITIES_NUM > 4 )
        TASK_PRIORITY_MED_LO,
#endif 
#if ( ENKITS_TASK_PRIORITIES_NUM > 1 )
        TASK_PRIORITY_LOW,
#endif
        TASK_PRIORITY_NUM
    };

    // ICompletable is a base class used to check for completion.
    // Can be used with dependencies to wait for their completion.
    // Derive from ITaskSet or IPinnedTask for running parallel tasks.
    class ICompletable
    {
    public:
        bool    GetIsComplete() const {
            return 0 == m_RunningCount.load( std::memory_order_acquire );
        }

        virtual ~ICompletable();

        // Dependency helpers, see Dependencies.cpp
        void SetDependency( Dependency& dependency_, const ICompletable* pDependencyTask_ );
        template<typename D, typename T, int SIZE> void SetDependenciesArr( D& dependencyArray_ , const T(&taskArray_)[SIZE] );
        template<typename D, typename T>           void SetDependenciesArr( D& dependencyArray_, std::initializer_list<T*> taskpList_ );
        template<typename D, typename T, int SIZE> void SetDependenciesArr( D(&dependencyArray_)[SIZE], const T(&taskArray_)[SIZE] );
        template<typename D, typename T, int SIZE> void SetDependenciesArr( D(&dependencyArray_)[SIZE], std::initializer_list<T*> taskpList_ );
        template<typename D, typename T, int SIZE> void SetDependenciesVec( D& dependencyVec_, const T(&taskArray_)[SIZE] );
        template<typename D, typename T>           void SetDependenciesVec( D& dependencyVec_, std::initializer_list<T*> taskpList_ );

        TaskPriority                   m_Priority            = TASK_PRIORITY_HIGH;
    protected:
        // Deriving from an ICompletable and overriding OnDependenciesComplete is advanced use.
        // If you do override OnDependenciesComplete() call:
        // ICompletable::OnDependenciesComplete( pTaskScheduler_, threadNum_ );
        // in your implementation.
        virtual void                   OnDependenciesComplete( TaskScheduler* pTaskScheduler_, uint32_t threadNum_ );
    private:
        friend class                   TaskScheduler;
        friend class                   Dependency;
        std::atomic<int32_t>           m_RunningCount               = {0};
        std::atomic<int32_t>           m_DependenciesCompletedCount = {0};
        int32_t                        m_DependenciesCount          = 0;
        mutable std::atomic<int32_t>   m_WaitingForTaskCount        = {0};
        mutable Dependency*            m_pDependents                = NULL;
    };

    // Subclass ITaskSet to create tasks.
    // TaskSets can be re-used, but check completion first.
    class ITaskSet : public ICompletable
    {
    public:
        ITaskSet() = default;
        ITaskSet( uint32_t setSize_ )
            : m_SetSize( setSize_ )
        {}

        ITaskSet( uint32_t setSize_, uint32_t minRange_ )
            : m_SetSize( setSize_ )
            , m_MinRange( minRange_ )
            , m_RangeToRun(minRange_)
        {}

        // Execute range should be overloaded to process tasks. It will be called with a
        // range_ where range.start >= 0; range.start < range.end; and range.end < m_SetSize;
        // The range values should be mapped so that linearly processing them in order is cache friendly
        // i.e. neighbouring values should be close together.
        // threadnum should not be used for changing processing of data, it's intended purpose
        // is to allow per-thread data buckets for output.
        virtual void ExecuteRange( TaskSetPartition range_, uint32_t threadnum_  ) = 0;

        // Set Size - usually the number of data items to be processed, see ExecuteRange. Defaults to 1
        uint32_t     m_SetSize  = 1;

        // Min Range - Minimum size of of TaskSetPartition range when splitting a task set into partitions.
        // Designed for reducing scheduling overhead by preventing set being
        // divided up too small. Ranges passed to ExecuteRange will *not* be a mulitple of this,
        // only attempts to deliver range sizes larger than this most of the time.
        // This should be set to a value which results in computation effort of at least 10k
        // clock cycles to minimize task scheduler overhead.
        // NOTE: The last partition will be smaller than m_MinRange if m_SetSize is not a multiple
        // of m_MinRange.
        // Also known as grain size in literature.
        uint32_t     m_MinRange  = 1;

    private:
        friend class TaskScheduler;
        void         OnDependenciesComplete( TaskScheduler* pTaskScheduler_, uint32_t threadNum_ ) override final;
        uint32_t     m_RangeToRun = 1;
    };

    // Subclass IPinnedTask to create tasks which can be run on a given thread only.
    class IPinnedTask : public ICompletable
    {
    public:
        IPinnedTask() = default;
        IPinnedTask( uint32_t threadNum_ ) : threadNum(threadNum_) {}  // default is to run a task on main thread

        // IPinnedTask needs to be non abstract for intrusive list functionality.
        // Should never be called as should be overridden.
        virtual void Execute() { ENKI_ASSERT(false); }

        uint32_t                  threadNum = 0; // thread to run this pinned task on
        std::atomic<IPinnedTask*> pNext = {NULL};
    private:
        void         OnDependenciesComplete( TaskScheduler* pTaskScheduler_, uint32_t threadNum_ ) override final;
    };

    // TaskSet - a utility task set for creating tasks based on std::func.
    typedef std::function<void (TaskSetPartition range, uint32_t threadnum  )> TaskSetFunction;
    class TaskSet : public ITaskSet
    {
    public:
        TaskSet() = default;
        TaskSet( TaskSetFunction func_ ) : m_Function( func_ ) {}
        TaskSet( uint32_t setSize_, TaskSetFunction func_ ) : ITaskSet( setSize_ ), m_Function( func_ ) {}

        void ExecuteRange( TaskSetPartition range_, uint32_t threadnum_  ) override { m_Function( range_, threadnum_ ); }
        TaskSetFunction m_Function;
    };

    // LambdaPinnedTask - a utility pinned task for creating tasks based on std::func.
    typedef std::function<void ()> PinnedTaskFunction;
    class LambdaPinnedTask : public IPinnedTask
    {
    public:
        LambdaPinnedTask() = default;
        LambdaPinnedTask( PinnedTaskFunction func_ ) : m_Function( func_ ) {}
        LambdaPinnedTask( uint32_t threadNum_, PinnedTaskFunction func_ ) : IPinnedTask( threadNum_ ), m_Function( func_ ) {}

        void Execute() override { m_Function(); }
        PinnedTaskFunction m_Function;
    };

    class Dependency
    {
    public:
                        Dependency() = default; 
                        Dependency( const Dependency& ) = delete;
        ENKITS_API      Dependency( Dependency&& ) noexcept;	
        ENKITS_API      Dependency(    const ICompletable* pDependencyTask_, ICompletable* pTaskToRunOnCompletion_ );
        ENKITS_API      ~Dependency();

        ENKITS_API void SetDependency( const ICompletable* pDependencyTask_, ICompletable* pTaskToRunOnCompletion_ );
        ENKITS_API void ClearDependency();
              ICompletable* GetTaskToRunOnCompletion() { return pTaskToRunOnCompletion; }
        const ICompletable* GetDependencyTask()        { return pDependencyTask; }
    private:
        friend class TaskScheduler; friend class ICompletable;
        ICompletable*       pTaskToRunOnCompletion = NULL;
        const ICompletable* pDependencyTask        = NULL;
        Dependency*         pNext                  = NULL;
    };

    // TaskScheduler implements several callbacks intended for profilers
    typedef void (*ProfilerCallbackFunc)( uint32_t threadnum_ );
    struct ProfilerCallbacks
    {
        ProfilerCallbackFunc threadStart;
        ProfilerCallbackFunc threadStop;
        ProfilerCallbackFunc waitForNewTaskSuspendStart;      // thread suspended waiting for new tasks
        ProfilerCallbackFunc waitForNewTaskSuspendStop;       // thread unsuspended
        ProfilerCallbackFunc waitForTaskCompleteStart;        // thread waiting for task completion
        ProfilerCallbackFunc waitForTaskCompleteStop;         // thread stopped waiting
        ProfilerCallbackFunc waitForTaskCompleteSuspendStart; // thread suspended waiting task completion
        ProfilerCallbackFunc waitForTaskCompleteSuspendStop;  // thread unsuspended
    };

    // Custom allocator, set in TaskSchedulerConfig. Also see ENKI_CUSTOM_ALLOC_FILE_AND_LINE for file_ and line_
    typedef void* (*AllocFunc)( size_t align_, size_t size_, void* userData_, const char* file_, int line_ );
    typedef void  (*FreeFunc)(  void* ptr_,    size_t size_, void* userData_, const char* file_, int line_ );
    ENKITS_API void* DefaultAllocFunc(  size_t align_, size_t size_, void* userData_, const char* file_, int line_ );
    ENKITS_API void  DefaultFreeFunc(   void* ptr_,    size_t size_, void* userData_, const char* file_, int line_ );
    struct CustomAllocator
    {
        AllocFunc alloc    = DefaultAllocFunc;
        FreeFunc  free     = DefaultFreeFunc;
        void*     userData = nullptr;
    };

    // TaskSchedulerConfig - configuration struct for advanced Initialize
    struct TaskSchedulerConfig
    {
        // numTaskThreadsToCreate - Number of tasking threads the task scheduler will create. Must be > 0.
        // Defaults to GetNumHardwareThreads()-1 threads as thread which calls initialize is thread 0.
        uint32_t          numTaskThreadsToCreate = GetNumHardwareThreads()-1;

        // numExternalTaskThreads - Advanced use. Number of external threads which need to use TaskScheduler API.
        // See TaskScheduler::RegisterExternalTaskThread() for usage.
        // Defaults to 0. The thread used to initialize the TaskScheduler can also use the TaskScheduler API.
        // Thus there are (numTaskThreadsToCreate + numExternalTaskThreads + 1) able to use the API, with this
        // defaulting to the number of harware threads available to the system.
        uint32_t          numExternalTaskThreads = 0;

        ProfilerCallbacks profilerCallbacks = {};

        CustomAllocator   customAllocator;
    };

    class TaskScheduler
    {
    public:
        ENKITS_API TaskScheduler();
        ENKITS_API ~TaskScheduler();

        // Call an Initialize function before adding tasks.

        // Initialize() will create GetNumHardwareThreads()-1 tasking threads, which is
        // sufficient to fill the system when including the main thread.
        // Initialize can be called multiple times - it will wait for completion
        // before re-initializing.
        ENKITS_API void            Initialize();

        // Initialize( numThreadsTotal_ )
        // will create numThreadsTotal_-1 threads, as thread 0 is
        // the thread on which the initialize was called.
        // numThreadsTotal_ must be > 0
        ENKITS_API void            Initialize( uint32_t numThreadsTotal_ );

        // Initialize with advanced TaskSchedulerConfig settings. See TaskSchedulerConfig.
        ENKITS_API void            Initialize( TaskSchedulerConfig config_ );

        // Get config. Can be called before Initialize to get the defaults.
        ENKITS_API TaskSchedulerConfig GetConfig() const;

        // while( GetIsRunning() ) {} can be used in tasks which loop, to check if enkiTS has been shutdown.
        // If GetIsRunning() returns false should then exit. Not required for finite tasks
        inline     bool            GetIsRunning() const { return m_bRunning.load( std::memory_order_acquire ); }

        // while( !GetIsWaitforAllCalled() ) {} can be used in tasks which loop, to check if WaitforAll() has been called.
        // If GetIsWaitforAllCalled() returns false should then exit. Not required for finite tasks
        // This is intended to be used with code which calls WaitforAll() with flag WAITFORALLFLAGS_INC_WAIT_NEW_PINNED_TASKS set.
        // This is also set when the the task manager is shutting down, so no need to have an additional check for GetIsRunning()
        inline     bool            GetIsWaitforAllCalled() const { return m_bWaitforAllCalled.load( std::memory_order_acquire ); }

        // Adds the TaskSet to pipe and returns if the pipe is not full.
        // If the pipe is full, pTaskSet is run.
        // should only be called from main thread, or within a task
        ENKITS_API void            AddTaskSetToPipe( ITaskSet* pTaskSet_ );

        // Thread 0 is main thread, otherwise use threadNum
        // Pinned tasks can be added from any thread
        ENKITS_API void            AddPinnedTask( IPinnedTask* pTask_ );

        // This function will run any IPinnedTask* for current thread, but not run other
        // Main thread should call this or use a wait to ensure it's tasks are run.
        ENKITS_API void            RunPinnedTasks();

        // Runs the TaskSets in pipe until true == pTaskSet->GetIsComplete();
        // should only be called from thread which created the taskscheduler , or within a task
        // if called with 0 it will try to run tasks, and return if none available.
        // To run only a subset of tasks, set priorityOfLowestToRun_ to a high priority.
        // Default is lowest priority available.
        // Only wait for child tasks of the current task otherwise a deadlock could occur.
        ENKITS_API void            WaitforTask( const ICompletable* pCompletable_, enki::TaskPriority priorityOfLowestToRun_ = TaskPriority(TASK_PRIORITY_NUM - 1) );

        // Waits for all task sets to complete - not guaranteed to work unless we know we
        // are in a situation where tasks aren't being continuously added.
        // If you are running tasks which loop, make sure to check GetIsWaitforAllCalled() and exit
        ENKITS_API void            WaitforAll();

        // Waits for all task sets to complete and shutdown threads - not guaranteed to work unless we know we
        // are in a situation where tasks aren't being continuously added.
        // This function can be safely called even if TaskScheduler::Initialize() has not been called.
        ENKITS_API void            WaitforAllAndShutdown();

        // Waits for the current thread to receive a PinnedTask
        // Will not run any tasks - use with RunPinnedTasks()
        // Can be used with both ExternalTaskThreads or with an enkiTS tasking thread to create
        // a thread which only runs pinned tasks. If enkiTS threads are used can create
        // extra enkiTS task threads to handle non blocking computation via normal tasks.
        ENKITS_API void            WaitForNewPinnedTasks();

        // Returns the number of threads created for running tasks + number of external threads
        // plus 1 to account for the thread used to initialize the task scheduler.
        // Equivalent to config values: numTaskThreadsToCreate + numExternalTaskThreads + 1.
        // It is guaranteed that GetThreadNum() < GetNumTaskThreads()
        ENKITS_API uint32_t        GetNumTaskThreads() const;

        // Returns the current task threadNum
        // Will return 0 for thread which initialized the task scheduler,
        // and all other non-enkiTS threads which have not been registered ( see RegisterExternalTaskThread() ),
        // and < GetNumTaskThreads() for all threads.
        // It is guaranteed that GetThreadNum() < GetNumTaskThreads()
        ENKITS_API uint32_t        GetThreadNum() const;

         // Call on a thread to register the thread to use the TaskScheduling API.
        // This is implicitly done for the thread which initializes the TaskScheduler
        // Intended for developers who have threads who need to call the TaskScheduler API
        // Returns true if successfull, false if not.
        // Can only have numExternalTaskThreads registered at any one time, which must be set
        // at initialization time.
        ENKITS_API bool            RegisterExternalTaskThread();

        // As RegisterExternalTaskThread() but explicitly requests a given thread number.
        // threadNumToRegister_ must be  >= GetNumFirstExternalTaskThread()
        // and < ( GetNumFirstExternalTaskThread() + numExternalTaskThreads )
        ENKITS_API bool            RegisterExternalTaskThread( uint32_t threadNumToRegister_ );

        // Call on a thread on which RegisterExternalTaskThread has been called to deregister that thread.
        ENKITS_API void            DeRegisterExternalTaskThread();

        // Get the number of registered external task threads.
        ENKITS_API uint32_t        GetNumRegisteredExternalTaskThreads();

        // Get the thread number of the first external task thread. This thread
        // is not guaranteed to be registered, but threads are registered in order
        // from GetNumFirstExternalTaskThread() up to ( GetNumFirstExternalTaskThread() + numExternalTaskThreads )
        // Note that if numExternalTaskThreads == 0 a for loop using this will be valid:
        // for( uint32_t externalThreadNum = GetNumFirstExternalTaskThread();
        //      externalThreadNum < ( GetNumFirstExternalTaskThread() + numExternalTaskThreads
        //      ++externalThreadNum ) { // do something with externalThreadNum }
        inline static constexpr uint32_t  GetNumFirstExternalTaskThread() { return 1; }

        // ------------- Start DEPRECATED Functions -------------
        // DEPRECATED - WaitforTaskSet, deprecated interface use WaitforTask
        inline void                WaitforTaskSet( const ICompletable* pCompletable_ ) { WaitforTask( pCompletable_ ); }

        // DEPRECATED - GetProfilerCallbacks.  Use TaskSchedulerConfig instead
        // Returns the ProfilerCallbacks structure so that it can be modified to
        // set the callbacks. Should be set prior to initialization.
        inline ProfilerCallbacks* GetProfilerCallbacks() { return &m_Config.profilerCallbacks; }
        // -------------  End DEPRECATED Functions  -------------

    private:
        friend class ICompletable; friend class ITaskSet; friend class IPinnedTask;
        static void TaskingThreadFunction( const ThreadArgs& args_ );
        bool        HaveTasks( uint32_t threadNum_ );
        void        WaitForNewTasks( uint32_t threadNum_ );
        void        WaitForTaskCompletion( const ICompletable* pCompletable_, uint32_t threadNum_ );
        void        RunPinnedTasks( uint32_t threadNum_, uint32_t priority_ );
        bool        TryRunTask( uint32_t threadNum_, uint32_t& hintPipeToCheck_io_ );
        bool        TryRunTask( uint32_t threadNum_, uint32_t priority_, uint32_t& hintPipeToCheck_io_ );
        void        StartThreads();
        void        StopThreads( bool bWait_ );
        void        SplitAndAddTask( uint32_t threadNum_, SubTaskSet subTask_, uint32_t rangeToSplit_ );
        void        WakeThreadsForNewTasks();
        void        WakeThreadsForTaskCompletion();
        bool        WakeSuspendedThreadsWithPinnedTasks();
        void        InitDependencies( ICompletable* pCompletable_  );
        ENKITS_API void TaskComplete( ICompletable* pTask_, bool bWakeThreads_, uint32_t threadNum_ );
        ENKITS_API void AddTaskSetToPipeInt( ITaskSet* pTaskSet_, uint32_t threadNum_ );
        ENKITS_API void AddPinnedTaskInt( IPinnedTask* pTask_ );

        template< typename T > T*   NewArray( size_t num_, const char* file_, int line_  );
        template< typename T > void DeleteArray( T* p_, size_t num_, const char* file_, int line_ );
        template<class T, class... Args> T* New( const char* file_, int line_,  Args&&... args_ );
        template< typename T > void Delete( T* p_, const char* file_, int line_ );
        template< typename T > T*   Alloc( const char* file_, int line_ );
        template< typename T > void Free( T* p_, const char* file_, int line_ );
        semaphoreid_t* SemaphoreNew();
        void SemaphoreDelete( semaphoreid_t* pSemaphore_ );

        TaskPipe*              m_pPipesPerThread[ TASK_PRIORITY_NUM ];
        PinnedTaskList*        m_pPinnedTaskListPerThread[ TASK_PRIORITY_NUM ];

        uint32_t               m_NumThreads;
        ThreadDataStore*       m_pThreadDataStore;
        std::thread*           m_pThreads;
        std::atomic<bool>      m_bRunning;
        std::atomic<bool>      m_bWaitforAllCalled;
        std::atomic<int32_t>   m_NumInternalTaskThreadsRunning;
        std::atomic<int32_t>   m_NumThreadsWaitingForNewTasks;
        std::atomic<int32_t>   m_NumThreadsWaitingForTaskCompletion;
        uint32_t               m_NumPartitions;
        semaphoreid_t*         m_pNewTaskSemaphore;
        semaphoreid_t*         m_pTaskCompleteSemaphore;
        uint32_t               m_NumInitialPartitions;
        bool                   m_bHaveThreads;
        TaskSchedulerConfig    m_Config;
        std::atomic<int32_t>   m_NumExternalTaskThreadsRegistered;

        TaskScheduler( const TaskScheduler& nocopy_ );
        TaskScheduler& operator=( const TaskScheduler& nocopy_ );

    protected:
        void SetCustomAllocator( CustomAllocator customAllocator_ ); // for C interface
    };

    inline uint32_t GetNumHardwareThreads()
    {
        return std::thread::hardware_concurrency();
    }

    inline void ICompletable::OnDependenciesComplete( TaskScheduler* pTaskScheduler_, uint32_t threadNum_ )
    {
        m_RunningCount.fetch_sub( 1, std::memory_order_release );
        pTaskScheduler_->TaskComplete( this, true, threadNum_ );
    }

    inline void ITaskSet::OnDependenciesComplete( TaskScheduler* pTaskScheduler_, uint32_t threadNum_ )
    {
        pTaskScheduler_->AddTaskSetToPipeInt( this, threadNum_ );
    }

    inline void IPinnedTask::OnDependenciesComplete( TaskScheduler* pTaskScheduler_, uint32_t threadNum_ )
    {
        (void)threadNum_;
        pTaskScheduler_->AddPinnedTaskInt( this );
    }

    inline ICompletable::~ICompletable()
    {
        ENKI_ASSERT( GetIsComplete() ); // this task is still waiting to run
        Dependency* pDependency = m_pDependents;
        while( pDependency )
        {
            Dependency* pNext = pDependency->pNext;
            pDependency->pDependencyTask = NULL;
            pDependency->pNext = NULL;
            pDependency = pNext;
        }
    }

    inline void ICompletable::SetDependency( Dependency& dependency_, const ICompletable* pDependencyTask_ )
    {
        ENKI_ASSERT( pDependencyTask_ != this );
        dependency_.SetDependency( pDependencyTask_, this );
    }

    template<typename D, typename T, int SIZE>
    void ICompletable::SetDependenciesArr( D& dependencyArray_ , const T(&taskArray_)[SIZE] ) {
        static_assert( std::tuple_size<D>::value >= SIZE, "Size of dependency array too small" );
        for( int i = 0; i < SIZE; ++i )
        {
            dependencyArray_[i].SetDependency( &taskArray_[i], this );
        }
    }
    template<typename D, typename T>
    void ICompletable::SetDependenciesArr( D& dependencyArray_, std::initializer_list<T*> taskpList_ ) {
        ENKI_ASSERT( std::tuple_size<D>::value >= taskpList_.size() );
        int i = 0;
        for( auto pTask : taskpList_ )
        {
            dependencyArray_[i++].SetDependency( pTask, this );
        }
    }
    template<typename D, typename T, int SIZE>
    void ICompletable::SetDependenciesArr( D(&dependencyArray_)[SIZE], const T(&taskArray_)[SIZE] ) {
        for( int i = 0; i < SIZE; ++i )
        {
            dependencyArray_[i].SetDependency( &taskArray_[i], this );
        }
    }
    template<typename D, typename T, int SIZE>
    void ICompletable::SetDependenciesArr( D(&dependencyArray_)[SIZE], std::initializer_list<T*> taskpList_ ) {
        ENKI_ASSERT( SIZE >= taskpList_.size() );
        int i = 0;
        for( auto pTask : taskpList_ )
        {
            dependencyArray_[i++].SetDependency( pTask, this );
        }
    }
    template<typename D, typename T, int SIZE>
    void ICompletable::SetDependenciesVec( D& dependencyVec_, const T(&taskArray_)[SIZE] ) {
        dependencyVec_.resize( SIZE );
        for( int i = 0; i < SIZE; ++i )
        {
            dependencyVec_[i].SetDependency( &taskArray_[i], this );
        }
    }

    template<typename D, typename T>
    void ICompletable::SetDependenciesVec( D& dependencyVec_, std::initializer_list<T*> taskpList_ ) {
        dependencyVec_.resize( taskpList_.size() );
        int i = 0;
        for( auto pTask : taskpList_ )
        {
            dependencyVec_[i++].SetDependency( pTask, this );
        }
    }
}


// Copyright (c) 2013 Doug Binks
// 
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
// 
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgement in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "lockLessMultiReadPipe.hpp"

#include <algorithm>

#if defined __i386__ || defined __x86_64__
#include "x86intrin.h"
#elif defined _WIN32
#include <intrin.h>
#endif

using namespace enki;

#if defined(ENKI_CUSTOM_ALLOC_FILE_AND_LINE)
#define ENKI_FILE_AND_LINE __FILE__, __LINE__
#else
namespace
{
    const char* gc_File    = "";
    const uint32_t gc_Line = 0;
}
#define ENKI_FILE_AND_LINE  gc_File, gc_Line
#endif

namespace enki
{
    static const int32_t  gc_TaskStartCount          = 2;
    static const int32_t  gc_TaskAlmostCompleteCount = 1; // GetIsComplete() will return false, but execution is done and about to complete
    static const uint32_t gc_PipeSizeLog2            = 8;
    static const uint32_t gc_SpinCount               = 10;
    static const uint32_t gc_SpinBackOffMulitplier   = 100;
    static const uint32_t gc_MaxNumInitialPartitions = 8;
    static const uint32_t gc_CacheLineSize           = 64;
    // awaiting std::hardware_constructive_interference_size
};

// thread_local not well supported yet by C++11 compilers.
#ifdef _MSC_VER
    #if _MSC_VER <= 1800
        #define thread_local __declspec(thread)
    #endif
#elif __APPLE__
        // Apple thread_local currently not implemented despite it being in Clang.
        #define thread_local __thread
#endif


// each software thread gets it's own copy of gtl_threadNum, so this is safe to use as a static variable
static thread_local uint32_t                             gtl_threadNum       = 0;

namespace enki 
{
    struct SubTaskSet
    {
        ITaskSet*           pTask;
        TaskSetPartition    partition;
    };

    // we derive class TaskPipe rather than typedef to get forward declaration working easily
    class TaskPipe : public LockLessMultiReadPipe<gc_PipeSizeLog2,enki::SubTaskSet> {};

    enum ThreadState : int32_t
    {
        ENKI_THREAD_STATE_NONE,                  // shouldn't get this value
        ENKI_THREAD_STATE_NOT_LAUNCHED,          // for debug purposes - indicates enki task thread not yet launched
        ENKI_THREAD_STATE_RUNNING,
        ENKI_THREAD_STATE_PRIMARY_REGISTERED,    // primary thread is the one enkiTS was initialized on
        ENKI_THREAD_STATE_EXTERNAL_REGISTERED,
        ENKI_THREAD_STATE_EXTERNAL_UNREGISTERED,
        ENKI_THREAD_STATE_WAIT_TASK_COMPLETION,
        ENKI_THREAD_STATE_WAIT_NEW_TASKS,
        ENKI_THREAD_STATE_WAIT_NEW_PINNED_TASKS,
        ENKI_THREAD_STATE_STOPPED,
    };

    struct ThreadArgs
    {
        uint32_t                 threadNum;
        TaskScheduler*           pTaskScheduler;
    };

    struct alignas(enki::gc_CacheLineSize) ThreadDataStore 
    {
        semaphoreid_t*           pWaitNewPinnedTaskSemaphore = nullptr;
        std::atomic<ThreadState> threadState = { ENKI_THREAD_STATE_NONE };
        char prevent_false_Share[ enki::gc_CacheLineSize - sizeof(std::atomic<ThreadState>) - sizeof(semaphoreid_t*) ];
    };
    constexpr size_t SIZEOFTHREADDATASTORE = sizeof( ThreadDataStore ); // for easier inspection
    static_assert( SIZEOFTHREADDATASTORE == enki::gc_CacheLineSize, "ThreadDataStore may exhibit false sharing" );

    class PinnedTaskList : public LocklessMultiWriteIntrusiveList<IPinnedTask> {};

    semaphoreid_t* SemaphoreCreate();
    void SemaphoreDelete( semaphoreid_t* pSemaphore_ );
    void SemaphoreWait(   semaphoreid_t& semaphoreid );
    void SemaphoreSignal( semaphoreid_t& semaphoreid, int32_t countWaiting );
}

namespace
{
    SubTaskSet SplitTask( SubTaskSet& subTask_, uint32_t rangeToSplit_ )
    {
        SubTaskSet splitTask = subTask_;
        uint32_t rangeLeft = subTask_.partition.end - subTask_.partition.start;

        if( rangeToSplit_ > rangeLeft )
        {
            rangeToSplit_ = rangeLeft;
        }
        splitTask.partition.end = subTask_.partition.start + rangeToSplit_;
        subTask_.partition.start = splitTask.partition.end;
        return splitTask;
    }

    #if ( defined _WIN32 && ( defined _M_IX86  || defined _M_X64 ) ) || ( defined __i386__ || defined __x86_64__ )
    // Note: see https://software.intel.com/en-us/articles/a-common-construct-to-avoid-the-contention-of-threads-architecture-agnostic-spin-wait-loops
    static void SpinWait( uint32_t spinCount_ )
    {
        uint64_t end = __rdtsc() + spinCount_;
        while( __rdtsc() < end )
        {
            _mm_pause();
        }        
    }
    #else
    static void SpinWait( uint32_t spinCount_ )
    {
        while( spinCount_ )
        {
            // TODO: may have NOP or yield equiv
            --spinCount_;
        }        
    }
    #endif
}

static void SafeCallback( ProfilerCallbackFunc func_, uint32_t threadnum_ )
{
    if( func_ != nullptr )
    {
        func_( threadnum_ );
    }
}

   
ENKITS_API void* enki::DefaultAllocFunc( size_t align_, size_t size_, void* userData_, const char* file_, int line_ )
{ 
    (void)userData_; (void)file_; (void)line_;
    void* pRet;
#ifdef _WIN32
    pRet = (void*)_aligned_malloc( size_, align_ );
#else
    pRet = nullptr;
    if( align_ <= size_ && align_ <= alignof(int64_t) )
    {
        // no need for alignment, use malloc
        pRet = malloc( size_ );
    }
    else
    {
        int retval = posix_memalign( &pRet, align_, size_ );
        (void)retval;	//unused
    }
#endif
    return pRet;
};

ENKITS_API void  enki::DefaultFreeFunc(  void* ptr_,   size_t size_, void* userData_, const char* file_, int line_ )
{
     (void)size_; (void)userData_; (void)file_; (void)line_;
#ifdef _WIN32
    _aligned_free( ptr_ );
#else
    free( ptr_ );
#endif
};

bool TaskScheduler::RegisterExternalTaskThread()
{
    bool bRegistered = false;
    while( !bRegistered && m_NumExternalTaskThreadsRegistered < (int32_t)m_Config.numExternalTaskThreads  )
    {
        for(uint32_t thread = GetNumFirstExternalTaskThread(); thread < GetNumFirstExternalTaskThread() + m_Config.numExternalTaskThreads; ++thread )
        {
            ThreadState threadStateExpected = ENKI_THREAD_STATE_EXTERNAL_UNREGISTERED;
            if( m_pThreadDataStore[thread].threadState.compare_exchange_strong(
                threadStateExpected, ENKI_THREAD_STATE_EXTERNAL_REGISTERED ) )
            {
                ++m_NumExternalTaskThreadsRegistered;
                gtl_threadNum = thread;
                bRegistered = true;
                break;
            }
        }
    }
    return bRegistered;
}

bool TaskScheduler::RegisterExternalTaskThread( uint32_t threadNumToRegister_ )
{
    ENKI_ASSERT( threadNumToRegister_ >= GetNumFirstExternalTaskThread() );
    ENKI_ASSERT( threadNumToRegister_ < ( GetNumFirstExternalTaskThread() + m_Config.numExternalTaskThreads ) );
    ThreadState threadStateExpected = ENKI_THREAD_STATE_EXTERNAL_UNREGISTERED;
    if( m_pThreadDataStore[threadNumToRegister_].threadState.compare_exchange_strong(
        threadStateExpected, ENKI_THREAD_STATE_EXTERNAL_REGISTERED ) )
    {
        ++m_NumExternalTaskThreadsRegistered;
        gtl_threadNum = threadNumToRegister_;
        return true;
    }
    return false;
}


void TaskScheduler::DeRegisterExternalTaskThread()
{
    ENKI_ASSERT( gtl_threadNum );
    ThreadState threadState = m_pThreadDataStore[gtl_threadNum].threadState.load( std::memory_order_acquire );
    ENKI_ASSERT( threadState == ENKI_THREAD_STATE_EXTERNAL_REGISTERED );
    if( threadState == ENKI_THREAD_STATE_EXTERNAL_REGISTERED )
    {
        --m_NumExternalTaskThreadsRegistered;
        m_pThreadDataStore[gtl_threadNum].threadState.store( ENKI_THREAD_STATE_EXTERNAL_UNREGISTERED, std::memory_order_release );
        gtl_threadNum = 0;
    }
}

uint32_t TaskScheduler::GetNumRegisteredExternalTaskThreads()
{
    return m_NumExternalTaskThreadsRegistered;
}

void TaskScheduler::TaskingThreadFunction( const ThreadArgs& args_ )
{
    uint32_t threadNum  = args_.threadNum;
    TaskScheduler*  pTS = args_.pTaskScheduler;
    gtl_threadNum       = threadNum;

    pTS->m_pThreadDataStore[threadNum].threadState.store( ENKI_THREAD_STATE_RUNNING, std::memory_order_release );
    SafeCallback( pTS->m_Config.profilerCallbacks.threadStart, threadNum );

    uint32_t spinCount = 0;
    uint32_t hintPipeToCheck_io = threadNum + 1;    // does not need to be clamped.
    while( pTS->GetIsRunning() )
    {
        if( !pTS->TryRunTask( threadNum, hintPipeToCheck_io ) )
        {
            // no tasks, will spin then wait
            ++spinCount;
            if( spinCount > gc_SpinCount )
            {
                pTS->WaitForNewTasks( threadNum );
            }
            else
            {
                uint32_t spinBackoffCount = spinCount * gc_SpinBackOffMulitplier;
                SpinWait( spinBackoffCount );
            }
        }
        else
        {
            spinCount = 0; // have run a task so reset spin count.
        }
    }

    pTS->m_NumInternalTaskThreadsRunning.fetch_sub( 1, std::memory_order_release );
    pTS->m_pThreadDataStore[threadNum].threadState.store( ENKI_THREAD_STATE_STOPPED, std::memory_order_release );
    SafeCallback( pTS->m_Config.profilerCallbacks.threadStop, threadNum );
    return;

}


void TaskScheduler::StartThreads()
{
    if( m_bHaveThreads )
    {
        return;
    }

    m_NumThreads = m_Config.numTaskThreadsToCreate + m_Config.numExternalTaskThreads + 1;

    for( int priority = 0; priority < TASK_PRIORITY_NUM; ++priority )
    {
        m_pPipesPerThread[ priority ]          = NewArray<TaskPipe>( m_NumThreads, ENKI_FILE_AND_LINE );
        m_pPinnedTaskListPerThread[ priority ] = NewArray<PinnedTaskList>( m_NumThreads, ENKI_FILE_AND_LINE );
    }

    m_pNewTaskSemaphore      = SemaphoreNew();
    m_pTaskCompleteSemaphore = SemaphoreNew();

    // we create one less thread than m_NumThreads as the main thread counts as one
    m_pThreadDataStore   = NewArray<ThreadDataStore>( m_NumThreads, ENKI_FILE_AND_LINE );
    m_pThreads           = NewArray<std::thread>( m_NumThreads, ENKI_FILE_AND_LINE );
    m_bRunning = true;
    m_bWaitforAllCalled = false;

    // current thread is primary enkiTS thread
    m_pThreadDataStore[0].threadState = ENKI_THREAD_STATE_PRIMARY_REGISTERED;
    gtl_threadNum = 0;

    for( uint32_t thread = GetNumFirstExternalTaskThread(); thread < m_Config.numExternalTaskThreads + GetNumFirstExternalTaskThread(); ++thread )
    {
        m_pThreadDataStore[thread].threadState   = ENKI_THREAD_STATE_EXTERNAL_UNREGISTERED;
    }
    for( uint32_t thread = m_Config.numExternalTaskThreads + GetNumFirstExternalTaskThread(); thread < m_NumThreads; ++thread )
    {
        m_pThreadDataStore[thread].threadState   = ENKI_THREAD_STATE_NOT_LAUNCHED;
    }
    // only launch threads once all thread states are set
    for( uint32_t thread = m_Config.numExternalTaskThreads + GetNumFirstExternalTaskThread(); thread < m_NumThreads; ++thread )
    {
        m_pThreads[thread]                       = std::thread( TaskingThreadFunction, ThreadArgs{ thread, this } );
        ++m_NumInternalTaskThreadsRunning;
    }

    // Create Wait New Pinned Task Semaphores
    for( uint32_t threadNum = 0; threadNum < m_NumThreads; ++threadNum )
    {
        m_pThreadDataStore[threadNum].pWaitNewPinnedTaskSemaphore = SemaphoreNew();
    }

    // ensure we have sufficient tasks to equally fill either all threads including main
    // or just the threads we've launched, this is outside the firstinit as we want to be able
    // to runtime change it
    if( 1 == m_NumThreads )
    {
        m_NumPartitions        = 1;
        m_NumInitialPartitions = 1;
    }
    else
    {
        // There could be more threads than hardware threads if external threads are
        // being intended for blocking functionality such as io etc.
        // We only need to partition for a maximum of the available processor parallelism.
        uint32_t numThreadsToPartitionFor = std::min( m_NumThreads, GetNumHardwareThreads() );
        m_NumPartitions = numThreadsToPartitionFor * (numThreadsToPartitionFor - 1);
        // ensure m_NumPartitions, m_NumInitialPartitions non zero, can happen if m_NumThreads > 1 && GetNumHardwareThreads() == 1
        m_NumPartitions        = std::max( m_NumPartitions,              (uint32_t)1 );
        m_NumInitialPartitions = std::max( numThreadsToPartitionFor - 1, (uint32_t)1 );
        if( m_NumInitialPartitions > gc_MaxNumInitialPartitions )
        {
            m_NumInitialPartitions = gc_MaxNumInitialPartitions;
        }
    }

    m_bHaveThreads = true;
}

void TaskScheduler::StopThreads( bool bWait_ )
{
    if( m_bHaveThreads )
    {
        // wait for them threads quit before deleting data
        m_bRunning.store( false, std::memory_order_release );
        m_bWaitforAllCalled.store( false, std::memory_order_release );


        while( bWait_ && m_NumInternalTaskThreadsRunning )
        {
            // keep firing event to ensure all threads pick up state of m_bRunning
           WakeThreadsForNewTasks();

           for( uint32_t threadId = 0; threadId < m_NumThreads; ++threadId )
           {
               // send wait for new pinned tasks signal to ensure any waiting are awoken
               SemaphoreSignal( *m_pThreadDataStore[ threadId ].pWaitNewPinnedTaskSemaphore, 1 );
           }
        }

        // detach threads starting with thread GetNumFirstExternalTaskThread() (as 0 is initialization thread).
        for( uint32_t thread = m_Config.numExternalTaskThreads +  GetNumFirstExternalTaskThread(); thread < m_NumThreads; ++thread )
        {
            ENKI_ASSERT( m_pThreads[thread].joinable() );
            m_pThreads[thread].join();
        }

        // delete any Wait New Pinned Task Semaphores
        for( uint32_t threadNum = 0; threadNum < m_NumThreads; ++threadNum )
        {
            SemaphoreDelete( m_pThreadDataStore[threadNum].pWaitNewPinnedTaskSemaphore );
        }

        DeleteArray( m_pThreadDataStore, m_NumThreads, ENKI_FILE_AND_LINE );
        DeleteArray( m_pThreads, m_NumThreads, ENKI_FILE_AND_LINE );
        m_pThreadDataStore = 0;
        m_pThreads = 0;

        SemaphoreDelete( m_pNewTaskSemaphore );
        m_pNewTaskSemaphore = 0;
        SemaphoreDelete( m_pTaskCompleteSemaphore );
        m_pTaskCompleteSemaphore = 0;

        m_bHaveThreads = false;
        m_NumThreadsWaitingForNewTasks = 0;
        m_NumThreadsWaitingForTaskCompletion = 0;
        m_NumInternalTaskThreadsRunning = 0;
        m_NumExternalTaskThreadsRegistered = 0;

        for( int priority = 0; priority < TASK_PRIORITY_NUM; ++priority )
        {
            DeleteArray( m_pPipesPerThread[ priority ], m_NumThreads, ENKI_FILE_AND_LINE );
            m_pPipesPerThread[ priority ] = NULL;
            DeleteArray( m_pPinnedTaskListPerThread[ priority ], m_NumThreads, ENKI_FILE_AND_LINE );
            m_pPinnedTaskListPerThread[ priority ] = NULL;
        }
        m_NumThreads = 0;
    }
}

bool TaskScheduler::TryRunTask( uint32_t threadNum_, uint32_t& hintPipeToCheck_io_ )
{
    for( int priority = 0; priority < TASK_PRIORITY_NUM; ++priority )
    {
        if( TryRunTask( threadNum_, priority, hintPipeToCheck_io_ ) )
        {
            return true;
        }
    }
    return false;
}

bool TaskScheduler::TryRunTask( uint32_t threadNum_, uint32_t priority_, uint32_t& hintPipeToCheck_io_ )
{
    // Run any tasks for this thread
    RunPinnedTasks( threadNum_, priority_ );

    // check for tasks
    SubTaskSet subTask;
    bool bHaveTask = m_pPipesPerThread[ priority_ ][ threadNum_ ].WriterTryReadFront( &subTask );

    uint32_t threadToCheck = hintPipeToCheck_io_;
    uint32_t checkCount = 0;
    while( !bHaveTask && checkCount < m_NumThreads )
    {
        threadToCheck = ( hintPipeToCheck_io_ + checkCount ) % m_NumThreads;
        if( threadToCheck != threadNum_ )
        {
            bHaveTask = m_pPipesPerThread[ priority_ ][ threadToCheck ].ReaderTryReadBack( &subTask );
        }
        ++checkCount;
    }
        
    if( bHaveTask )
    {
        // update hint, will preserve value unless actually got task from another thread.
        hintPipeToCheck_io_ = threadToCheck;

        uint32_t partitionSize = subTask.partition.end - subTask.partition.start;
        if( subTask.pTask->m_RangeToRun < partitionSize )
        {
            SubTaskSet taskToRun = SplitTask( subTask, subTask.pTask->m_RangeToRun );
            SplitAndAddTask( threadNum_, subTask, subTask.pTask->m_RangeToRun );
            taskToRun.pTask->ExecuteRange( taskToRun.partition, threadNum_ );
            int prevCount = taskToRun.pTask->m_RunningCount.fetch_sub(1,std::memory_order_release );
            if( gc_TaskStartCount == prevCount )
            {
                TaskComplete( taskToRun.pTask, true, threadNum_ );
            }
        }
        else
        {
            // the task has already been divided up by AddTaskSetToPipe, so just run it
            subTask.pTask->ExecuteRange( subTask.partition, threadNum_ );
            int prevCount = subTask.pTask->m_RunningCount.fetch_sub(1,std::memory_order_release );
            if( gc_TaskStartCount == prevCount )
            {
                TaskComplete( subTask.pTask, true, threadNum_ );
            }
        }
    }

    return bHaveTask;

}

void TaskScheduler::TaskComplete( ICompletable* pTask_, bool bWakeThreads_, uint32_t threadNum_ )
{
    // It must be impossible for a thread to enter the sleeping wait prior to the load of m_WaitingForTaskCount
    // in this function, so we introduce an gc_TaskAlmostCompleteCount to prevent this.
    ENKI_ASSERT( gc_TaskAlmostCompleteCount == pTask_->m_RunningCount.load( std::memory_order_acquire ) );
    bool bCallWakeThreads = bWakeThreads_ && pTask_->m_WaitingForTaskCount.load( std::memory_order_acquire );

    Dependency* pDependent = pTask_->m_pDependents;

    // Do not access pTask_ below this line unless we have dependencies.
    pTask_->m_RunningCount.store( 0, std::memory_order_release );

    if( bCallWakeThreads )
    {
        WakeThreadsForTaskCompletion();
    }

    while( pDependent )
    {
        int prevDeps = pDependent->pTaskToRunOnCompletion->m_DependenciesCompletedCount.fetch_add( 1, std::memory_order_release );
        ENKI_ASSERT( prevDeps < pDependent->pTaskToRunOnCompletion->m_DependenciesCount );
        if( pDependent->pTaskToRunOnCompletion->m_DependenciesCount == ( prevDeps + 1 ) )
        {
            // get temp copy of pDependent so OnDependenciesComplete can delete task if needed.
            Dependency* pDependentCurr = pDependent;
            pDependent = pDependent->pNext;
            // reset dependencies
            pDependentCurr->pTaskToRunOnCompletion->m_DependenciesCompletedCount.store(
                0,
                std::memory_order_release );
            pDependentCurr->pTaskToRunOnCompletion->OnDependenciesComplete( this, threadNum_ );
        }
        else
        {
            pDependent = pDependent->pNext;
        }
    }
}

bool TaskScheduler::HaveTasks(  uint32_t threadNum_ )
{
    for( int priority = 0; priority < TASK_PRIORITY_NUM; ++priority )
    {
        for( uint32_t thread = 0; thread < m_NumThreads; ++thread )
        {
            if( !m_pPipesPerThread[ priority ][ thread ].IsPipeEmpty() )
            {
                return true;
            }
        }
        if( !m_pPinnedTaskListPerThread[ priority ][ threadNum_ ].IsListEmpty() )
        {
            return true;
        }
    }
    return false;
}

void TaskScheduler::WaitForNewTasks( uint32_t threadNum_ )
{
    // We don't want to suspend this thread if there are task threads
    // with pinned tasks suspended, as it could result in this thread
    // being unsuspended and not the thread with pinned tasks
    if( WakeSuspendedThreadsWithPinnedTasks() )
    {
        return;
    }

    // We incrememt the number of threads waiting here in order
    // to ensure that the check for tasks occurs after the increment
    // to prevent a task being added after a check, then the thread waiting.
    // This will occasionally result in threads being mistakenly awoken,
    // but they will then go back to sleep.
    m_NumThreadsWaitingForNewTasks.fetch_add( 1, std::memory_order_acquire );
    ThreadState prevThreadState = m_pThreadDataStore[threadNum_].threadState.load( std::memory_order_relaxed );
    m_pThreadDataStore[threadNum_].threadState.store( ENKI_THREAD_STATE_WAIT_NEW_TASKS, std::memory_order_seq_cst );

    if( HaveTasks( threadNum_ ) )
    {
        m_NumThreadsWaitingForNewTasks.fetch_sub( 1, std::memory_order_release );
    }
    else
    {
        SafeCallback( m_Config.profilerCallbacks.waitForNewTaskSuspendStart, threadNum_ );
        SemaphoreWait( *m_pNewTaskSemaphore );
        SafeCallback( m_Config.profilerCallbacks.waitForNewTaskSuspendStop, threadNum_ );
    }

    m_pThreadDataStore[threadNum_].threadState.store( prevThreadState, std::memory_order_release );
}

void TaskScheduler::WaitForTaskCompletion( const ICompletable* pCompletable_, uint32_t threadNum_ )
{
    // We don't want to suspend this thread if there are task threads
    // with pinned tasks suspended, as the completable could be a pinned task
    // or it could be waiting on one.
    if( WakeSuspendedThreadsWithPinnedTasks() )
    {
        return;
    }

    m_NumThreadsWaitingForTaskCompletion.fetch_add( 1, std::memory_order_acquire );
    pCompletable_->m_WaitingForTaskCount.fetch_add( 1, std::memory_order_acquire );
    ThreadState prevThreadState = m_pThreadDataStore[threadNum_].threadState.load( std::memory_order_relaxed );
    m_pThreadDataStore[threadNum_].threadState.store( ENKI_THREAD_STATE_WAIT_TASK_COMPLETION, std::memory_order_seq_cst );

    // do not wait on semaphore if task in gc_TaskAlmostCompleteCount state.
    if( gc_TaskAlmostCompleteCount >= pCompletable_->m_RunningCount.load( std::memory_order_acquire ) || HaveTasks( threadNum_ ) )
    {
        m_NumThreadsWaitingForTaskCompletion.fetch_sub( 1, std::memory_order_release );
    }
    else
    {
        SafeCallback( m_Config.profilerCallbacks.waitForTaskCompleteSuspendStart, threadNum_ );
        std::atomic_thread_fence(std::memory_order_acquire);

        SemaphoreWait( *m_pTaskCompleteSemaphore );
        if( !pCompletable_->GetIsComplete() )
        {
            // This thread which may not the one which was supposed to be awoken
            WakeThreadsForTaskCompletion();
        }
        SafeCallback( m_Config.profilerCallbacks.waitForTaskCompleteSuspendStop, threadNum_ );
    }

    m_pThreadDataStore[threadNum_].threadState.store( prevThreadState, std::memory_order_release );
    pCompletable_->m_WaitingForTaskCount.fetch_sub( 1, std::memory_order_release );
}

void TaskScheduler::WakeThreadsForNewTasks()
{
    int32_t waiting = m_NumThreadsWaitingForNewTasks.load( std::memory_order_relaxed );
    while( waiting > 0 && !m_NumThreadsWaitingForNewTasks.compare_exchange_weak(waiting, 0, std::memory_order_release, std::memory_order_relaxed ) ) {}

    if( waiting > 0 )
    {
        SemaphoreSignal( *m_pNewTaskSemaphore, waiting );
    }

    // We also wake tasks waiting for completion as they can run tasks
    WakeThreadsForTaskCompletion();
}

void TaskScheduler::WakeThreadsForTaskCompletion()
{
    // m_NumThreadsWaitingForTaskCompletion can go negative as this indicates that
    // we signalled more threads than the number which ended up waiting
    int32_t waiting = m_NumThreadsWaitingForTaskCompletion.load( std::memory_order_relaxed );
    while( waiting > 0 && !m_NumThreadsWaitingForTaskCompletion.compare_exchange_weak(waiting, 0, std::memory_order_release, std::memory_order_relaxed ) ) {}

    if( waiting > 0 )
    {
        SemaphoreSignal( *m_pTaskCompleteSemaphore, waiting );
    }
}

bool TaskScheduler::WakeSuspendedThreadsWithPinnedTasks()
{
    uint32_t threadNum = gtl_threadNum;
    for( uint32_t t = 1; t < m_NumThreads; ++t )
    {
        // distribute thread checks more evenly by starting at our thread number rather than 0.
        uint32_t thread = ( threadNum + t ) % m_NumThreads;

        ThreadState state = m_pThreadDataStore[ thread ].threadState.load( std::memory_order_acquire );
            
        ENKI_ASSERT( state != ENKI_THREAD_STATE_NONE );

        if( state == ENKI_THREAD_STATE_WAIT_NEW_TASKS || state == ENKI_THREAD_STATE_WAIT_TASK_COMPLETION )
        {
            // thread is suspended, check if it has pinned tasks
            for( int priority = 0; priority < TASK_PRIORITY_NUM; ++priority )
            {
                if( !m_pPinnedTaskListPerThread[ priority ][ thread ].IsListEmpty() )
                {
                    WakeThreadsForNewTasks();
                    return true;
                }
            }
        }
    }
    return false;
}

void TaskScheduler::SplitAndAddTask( uint32_t threadNum_, SubTaskSet subTask_, uint32_t rangeToSplit_ )
{
    int32_t numAdded = 0;
    int32_t numRun   = 0;
    // ensure that an artificial completion is not registered whilst adding tasks by incrementing count
    subTask_.pTask->m_RunningCount.fetch_add( 1, std::memory_order_acquire );
    while( subTask_.partition.start != subTask_.partition.end )
    {
        SubTaskSet taskToAdd = SplitTask( subTask_, rangeToSplit_ );

        // add the partition to the pipe
        ++numAdded;
        subTask_.pTask->m_RunningCount.fetch_add( 1, std::memory_order_acquire );
        if( !m_pPipesPerThread[ subTask_.pTask->m_Priority ][ threadNum_ ].WriterTryWriteFront( taskToAdd ) )
        {
            if( numAdded > 1 )
            {
                WakeThreadsForNewTasks();
            }
            numAdded = 0;
            // alter range to run the appropriate fraction
            if( taskToAdd.pTask->m_RangeToRun < taskToAdd.partition.end - taskToAdd.partition.start )
            {
                taskToAdd.partition.end = taskToAdd.partition.start + taskToAdd.pTask->m_RangeToRun;
                ENKI_ASSERT( taskToAdd.partition.end <= taskToAdd.pTask->m_SetSize );
                subTask_.partition.start = taskToAdd.partition.end;
            }
            taskToAdd.pTask->ExecuteRange( taskToAdd.partition, threadNum_ );
            ++numRun;
        }
    }
    int prevCount = subTask_.pTask->m_RunningCount.fetch_sub( numRun + 1, std::memory_order_release );
    if( numRun + gc_TaskStartCount == prevCount )
    {
        TaskComplete( subTask_.pTask, false, threadNum_ );
    }

    // WakeThreadsForNewTasks also calls WakeThreadsForTaskCompletion() so do not need to do so above
    WakeThreadsForNewTasks();
}

TaskSchedulerConfig TaskScheduler::GetConfig() const
{
    return m_Config;
}

void TaskScheduler::AddTaskSetToPipeInt( ITaskSet* pTaskSet_, uint32_t threadNum_ )
{
    ENKI_ASSERT( pTaskSet_->m_RunningCount == gc_TaskStartCount );
    ThreadState prevThreadState = m_pThreadDataStore[threadNum_].threadState.load( std::memory_order_relaxed );
    m_pThreadDataStore[threadNum_].threadState.store( ENKI_THREAD_STATE_RUNNING, std::memory_order_relaxed );
    std::atomic_thread_fence(std::memory_order_acquire);


    // divide task up and add to pipe
    pTaskSet_->m_RangeToRun = pTaskSet_->m_SetSize / m_NumPartitions;
    if( pTaskSet_->m_RangeToRun < pTaskSet_->m_MinRange ) { pTaskSet_->m_RangeToRun = pTaskSet_->m_MinRange; }

    uint32_t rangeToSplit = pTaskSet_->m_SetSize / m_NumInitialPartitions;
    if( rangeToSplit < pTaskSet_->m_MinRange ) { rangeToSplit = pTaskSet_->m_MinRange; }

    SubTaskSet subTask;
    subTask.pTask = pTaskSet_;
    subTask.partition.start = 0;
    subTask.partition.end = pTaskSet_->m_SetSize;
    SplitAndAddTask( threadNum_, subTask, rangeToSplit );
    int prevCount = pTaskSet_->m_RunningCount.fetch_sub(1, std::memory_order_release );
    if( gc_TaskStartCount == prevCount )
    {
        TaskComplete( pTaskSet_, true, threadNum_ );
    }

    m_pThreadDataStore[threadNum_].threadState.store( prevThreadState, std::memory_order_release );
}

void TaskScheduler::AddTaskSetToPipe( ITaskSet* pTaskSet_ )
{
    ENKI_ASSERT( pTaskSet_->m_RunningCount == 0 );
    InitDependencies( pTaskSet_ );
    pTaskSet_->m_RunningCount.store( gc_TaskStartCount, std::memory_order_relaxed );
    AddTaskSetToPipeInt( pTaskSet_, gtl_threadNum );
}

void  TaskScheduler::AddPinnedTaskInt( IPinnedTask* pTask_ )
{
    ENKI_ASSERT( pTask_->m_RunningCount == gc_TaskStartCount );
    m_pPinnedTaskListPerThread[ pTask_->m_Priority ][ pTask_->threadNum ].WriterWriteFront( pTask_ );

    ThreadState statePinnedTaskThread = m_pThreadDataStore[ pTask_->threadNum ].threadState.load( std::memory_order_acquire );
    if( statePinnedTaskThread == ENKI_THREAD_STATE_WAIT_NEW_PINNED_TASKS )
    {
        SemaphoreSignal( *m_pThreadDataStore[ pTask_->threadNum ].pWaitNewPinnedTaskSemaphore, 1 );
    }
    else
    {
        WakeThreadsForNewTasks();
    }
}

void TaskScheduler::AddPinnedTask( IPinnedTask* pTask_ )
{
    ENKI_ASSERT( pTask_->m_RunningCount == 0 );
    InitDependencies( pTask_ );
    pTask_->m_RunningCount = gc_TaskStartCount;
    AddPinnedTaskInt( pTask_ );
}

void TaskScheduler::InitDependencies( ICompletable* pCompletable_ )
{
    // go through any dependencies and set thier running count so they show as not complete
    // and increment depedency count
    if( pCompletable_->m_RunningCount.load( std::memory_order_relaxed ) )
    {
        // already initialized
        return;
    }
    Dependency* pDependent = pCompletable_->m_pDependents;
    while( pDependent )
    {
        InitDependencies( pDependent->pTaskToRunOnCompletion );
        pDependent->pTaskToRunOnCompletion->m_RunningCount.store( gc_TaskStartCount, std::memory_order_relaxed );
        pDependent = pDependent->pNext;
    }
}


void TaskScheduler::RunPinnedTasks()
{
    uint32_t threadNum = gtl_threadNum;
    ThreadState prevThreadState = m_pThreadDataStore[threadNum].threadState.load( std::memory_order_relaxed );
    m_pThreadDataStore[threadNum].threadState.store( ENKI_THREAD_STATE_RUNNING, std::memory_order_relaxed );
    std::atomic_thread_fence(std::memory_order_acquire);
    for( int priority = 0; priority < TASK_PRIORITY_NUM; ++priority )
    {
        RunPinnedTasks( threadNum, priority );
    }
    m_pThreadDataStore[threadNum].threadState.store( prevThreadState, std::memory_order_release );
}

void TaskScheduler::RunPinnedTasks( uint32_t threadNum_, uint32_t priority_ )
{
    IPinnedTask* pPinnedTaskSet = NULL;
    do
    {
        pPinnedTaskSet = m_pPinnedTaskListPerThread[ priority_ ][ threadNum_ ].ReaderReadBack();
        if( pPinnedTaskSet )
        {
            pPinnedTaskSet->Execute();
            pPinnedTaskSet->m_RunningCount.fetch_sub(1,std::memory_order_release);
            TaskComplete( pPinnedTaskSet, true, threadNum_ );
        }
    } while( pPinnedTaskSet );
}

void    TaskScheduler::WaitforTask( const ICompletable* pCompletable_, enki::TaskPriority priorityOfLowestToRun_ )
{
    uint32_t threadNum = gtl_threadNum;
    uint32_t hintPipeToCheck_io = threadNum + 1;    // does not need to be clamped.

    // waiting for a task is equivalent to 'running' for thread state purpose as we may run tasks whilst waiting
    ThreadState prevThreadState = m_pThreadDataStore[threadNum].threadState.load( std::memory_order_relaxed );
    m_pThreadDataStore[threadNum].threadState.store( ENKI_THREAD_STATE_RUNNING, std::memory_order_relaxed );
    std::atomic_thread_fence(std::memory_order_acquire);


    if( pCompletable_ && !pCompletable_->GetIsComplete() )
    {
        SafeCallback( m_Config.profilerCallbacks.waitForTaskCompleteStart, threadNum );
        // We need to ensure that the task we're waiting on can complete even if we're the only thread,
        // so we clamp the priorityOfLowestToRun_ to no smaller than the task we're waiting for
        priorityOfLowestToRun_ = std::max( priorityOfLowestToRun_, pCompletable_->m_Priority );
        uint32_t spinCount = 0;
        while( !pCompletable_->GetIsComplete() )
        {
            ++spinCount;
            for( int priority = 0; priority <= priorityOfLowestToRun_; ++priority )
            {
                if( TryRunTask( threadNum, priority, hintPipeToCheck_io ) )
                {
                    spinCount = 0; // reset spin as ran a task
                    break;
                }
            }
            if( spinCount > gc_SpinCount )
            {
                WaitForTaskCompletion( pCompletable_, threadNum );
                spinCount = 0;
            }
            else
            {
                uint32_t spinBackoffCount = spinCount * gc_SpinBackOffMulitplier;
                SpinWait( spinBackoffCount );
            }
        }
        SafeCallback( m_Config.profilerCallbacks.waitForTaskCompleteStop, threadNum );
    }
    else if( nullptr == pCompletable_ )
    {
            for( int priority = 0; priority <= priorityOfLowestToRun_; ++priority )
            {
                if( TryRunTask( gtl_threadNum, priority, hintPipeToCheck_io ) )
                {
                    break;
                }
            }
    }

    m_pThreadDataStore[threadNum].threadState.store( prevThreadState, std::memory_order_release );

}

class TaskSchedulerWaitTask : public IPinnedTask
{
    void Execute() override
    {
        // do nothing
    }
};

void TaskScheduler::WaitforAll()
{
    m_bWaitforAllCalled.store( true, std::memory_order_release );

    bool bHaveTasks = true;
    uint32_t ourThreadNum = gtl_threadNum;
    uint32_t hintPipeToCheck_io = ourThreadNum  + 1;    // does not need to be clamped.
    bool otherThreadsRunning = false; // account for this thread
    uint32_t spinCount = 0;
    TaskSchedulerWaitTask dummyWaitTask;
    dummyWaitTask.threadNum = 0;
    while( bHaveTasks || otherThreadsRunning )
    {
        bHaveTasks = TryRunTask( ourThreadNum, hintPipeToCheck_io );
        ++spinCount;
        if( bHaveTasks )
        {
            spinCount = 0; // reset spin as ran a task
        }
        if( spinCount > gc_SpinCount )
        {
            // find a running thread and add a dummy wait task
            int32_t countThreadsToCheck = m_NumThreads - 1;
            bool bHaveThreadToWaitOn = false;
            do
            {
                --countThreadsToCheck;
                dummyWaitTask.threadNum = ( dummyWaitTask.threadNum + 1 ) % m_NumThreads;

                // We can only add a pinned task to wait on if we find an enki Task Thread which isn't this thread.
                // Otherwise we have to busy wait.
                if( dummyWaitTask.threadNum != ourThreadNum && dummyWaitTask.threadNum > m_Config.numExternalTaskThreads )
                {
                    ThreadState state = m_pThreadDataStore[ dummyWaitTask.threadNum ].threadState.load( std::memory_order_acquire );
                    if( state == ENKI_THREAD_STATE_RUNNING || state == ENKI_THREAD_STATE_WAIT_TASK_COMPLETION )
                    {
                        bHaveThreadToWaitOn = true;
                        break;
                    }
                }
            } while( countThreadsToCheck );

            if( bHaveThreadToWaitOn )
            {
                ENKI_ASSERT( dummyWaitTask.threadNum != ourThreadNum );
                AddPinnedTask( &dummyWaitTask );
                WaitforTask( &dummyWaitTask );
            }
            spinCount = 0;
        }
        else
        {
            uint32_t spinBackoffCount = spinCount * gc_SpinBackOffMulitplier;
            SpinWait( spinBackoffCount );
        }

        // count threads running
        otherThreadsRunning = false;
        for(uint32_t thread = 0; thread < m_NumThreads && !otherThreadsRunning; ++thread )
        {
            // ignore our thread
            if( thread != ourThreadNum )
            {
                switch( m_pThreadDataStore[thread].threadState.load( std::memory_order_acquire ) )
                {
                case ENKI_THREAD_STATE_NONE:
                    ENKI_ASSERT(false);
                    break;
                case ENKI_THREAD_STATE_NOT_LAUNCHED:
                case ENKI_THREAD_STATE_RUNNING:
                case ENKI_THREAD_STATE_WAIT_TASK_COMPLETION:
                    otherThreadsRunning = true;
                    break;
                case ENKI_THREAD_STATE_WAIT_NEW_PINNED_TASKS:
                    otherThreadsRunning = true;
                    SemaphoreSignal( *m_pThreadDataStore[thread].pWaitNewPinnedTaskSemaphore, 1 );
                    break;
                case ENKI_THREAD_STATE_PRIMARY_REGISTERED:
                case ENKI_THREAD_STATE_EXTERNAL_REGISTERED:
                case ENKI_THREAD_STATE_EXTERNAL_UNREGISTERED:
                case ENKI_THREAD_STATE_WAIT_NEW_TASKS:
                case ENKI_THREAD_STATE_STOPPED:
                    break;
                 };
            }
        }
        if( !otherThreadsRunning )
        {
            // check there are no tasks
            for(uint32_t thread = 0; thread < m_NumThreads && !otherThreadsRunning; ++thread )
            {
                // ignore our thread
                if( thread != ourThreadNum )
                {
                    otherThreadsRunning = HaveTasks( thread );
                }
            }
        }
     }

    m_bWaitforAllCalled.store( false, std::memory_order_release );
}

void    TaskScheduler::WaitforAllAndShutdown()
{
    if( m_bHaveThreads )
    {
        WaitforAll();
        StopThreads(true);
    }
}

void TaskScheduler::WaitForNewPinnedTasks()
{
    uint32_t threadNum = gtl_threadNum;
    ThreadState prevThreadState = m_pThreadDataStore[threadNum].threadState.load( std::memory_order_relaxed );
    m_pThreadDataStore[threadNum].threadState.store( ENKI_THREAD_STATE_WAIT_NEW_PINNED_TASKS, std::memory_order_seq_cst );

    // check if have tasks inside threadState change but before waiting
    bool bHavePinnedTasks = false;
    for( int priority = 0; priority < TASK_PRIORITY_NUM; ++priority )
    {
        if( !m_pPinnedTaskListPerThread[ priority ][ threadNum ].IsListEmpty() )
        {
            bHavePinnedTasks = true;
            break;
        }
    }

    if( !bHavePinnedTasks )
    {
        SafeCallback( m_Config.profilerCallbacks.waitForNewTaskSuspendStart, threadNum );
        SemaphoreWait( *m_pThreadDataStore[threadNum].pWaitNewPinnedTaskSemaphore );
        SafeCallback( m_Config.profilerCallbacks.waitForNewTaskSuspendStop, threadNum );
    }

    m_pThreadDataStore[threadNum].threadState.store( prevThreadState, std::memory_order_release );
}


uint32_t        TaskScheduler::GetNumTaskThreads() const
{
    return m_NumThreads;
}


uint32_t TaskScheduler::GetThreadNum() const
{
    return gtl_threadNum;
}

template<typename T>
T* TaskScheduler::NewArray( size_t num_, const char* file_, int line_  )
{
    T* pRet = (T*)m_Config.customAllocator.alloc( alignof(T), num_*sizeof(T), m_Config.customAllocator.userData, file_, line_ );
    if( !std::is_trivial<T>::value )
    {
		T* pCurr = pRet;
        for( size_t i = 0; i < num_; ++i )
        {
			void* pBuffer = pCurr;
            pCurr = new(pBuffer) T;
			++pCurr;
        }
    }
    return pRet;
}

template<typename T>
void TaskScheduler::DeleteArray( T* p_, size_t num_, const char* file_, int line_ )
{
    if( !std::is_trivially_destructible<T>::value )
    {
        size_t i = num_;
        while(i)
        {
            p_[--i].~T();
        }
    }
    m_Config.customAllocator.free( p_, sizeof(T)*num_, m_Config.customAllocator.userData, file_, line_ );
}

template<class T, class... Args>
T* TaskScheduler::New( const char* file_, int line_, Args&&... args_ )
{
    T* pRet = this->Alloc<T>( file_, line_ );
    return new(pRet) T( std::forward<Args>(args_)... );
}

template< typename T >
void TaskScheduler::Delete( T* p_, const char* file_, int line_  )
{
    p_->~T(); 
    this->Free(p_, file_, line_ );
}

template< typename T >
T* TaskScheduler::Alloc( const char* file_, int line_  )
{
    T* pRet = (T*)m_Config.customAllocator.alloc( alignof(T), sizeof(T), m_Config.customAllocator.userData, file_, line_ );
    return pRet;
}

template< typename T >
void TaskScheduler::Free( T* p_, const char* file_, int line_  )
{
    m_Config.customAllocator.free( p_, sizeof(T), m_Config.customAllocator.userData, file_, line_ );
}

TaskScheduler::TaskScheduler()
        : m_pPipesPerThread()
        , m_pPinnedTaskListPerThread()
        , m_NumThreads(0)
        , m_pThreadDataStore(NULL)
        , m_pThreads(NULL)
        , m_bRunning(0)
        , m_NumInternalTaskThreadsRunning(0)
        , m_NumThreadsWaitingForNewTasks(0)
        , m_NumThreadsWaitingForTaskCompletion(0)
        , m_NumPartitions(0)
        , m_pNewTaskSemaphore(NULL)
        , m_pTaskCompleteSemaphore(NULL)
        , m_NumInitialPartitions(0)
        , m_bHaveThreads(false)
        , m_NumExternalTaskThreadsRegistered(0)
{
}

TaskScheduler::~TaskScheduler()
{
    StopThreads( true ); // Stops threads, waiting for them.
}

void TaskScheduler::Initialize( uint32_t numThreadsTotal_ )
{
    ENKI_ASSERT( numThreadsTotal_ >= 1 );
    StopThreads( true ); // Stops threads, waiting for them.
    m_Config.numTaskThreadsToCreate = numThreadsTotal_ - 1;
    m_Config.numExternalTaskThreads = 0;
    StartThreads();}

void TaskScheduler::Initialize( TaskSchedulerConfig config_ )
{
    StopThreads( true ); // Stops threads, waiting for them.
    m_Config = config_;
    StartThreads();
}

void TaskScheduler::Initialize()
{
    Initialize( std::thread::hardware_concurrency() );
}

// Semaphore implementation
#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace enki
{
    struct semaphoreid_t
    {
        HANDLE      sem;
    };
    
    inline void SemaphoreCreate( semaphoreid_t& semaphoreid )
    {
#ifdef _XBOX_ONE
        semaphoreid.sem = CreateSemaphoreExW( NULL, 0, MAXLONG, NULL, 0, SEMAPHORE_ALL_ACCESS );
#else
        semaphoreid.sem = CreateSemaphore( NULL, 0, MAXLONG, NULL );
#endif
    }

    inline void SemaphoreClose( semaphoreid_t& semaphoreid )
    {
        CloseHandle( semaphoreid.sem );
    }

    inline void SemaphoreWait( semaphoreid_t& semaphoreid  )
    {
        DWORD retval = WaitForSingleObject( semaphoreid.sem, INFINITE );
        ENKI_ASSERT( retval != WAIT_FAILED );
        (void)retval; // only needed for ENKI_ASSERT
    }

    inline void SemaphoreSignal( semaphoreid_t& semaphoreid, int32_t countWaiting )
    {
        if( countWaiting )
        {
            ReleaseSemaphore( semaphoreid.sem, countWaiting, NULL );
        }
    }
}
#elif defined(__MACH__)


// OS X does not have POSIX semaphores
// Mach semaphores can now only be created by the kernel
// Named sempahores work, but would require unique name construction to ensure
// they are isolated to this process.
// Dispatch semaphores appear to be the way other developers use OSX Semaphores, e.g. Boost
// However the API could change
// OSX below 10.6 does not support dispatch, but I do not have an earlier OSX version
// to test alternatives
#include <dispatch/dispatch.h>

namespace enki
{
    
    struct semaphoreid_t
    {
        dispatch_semaphore_t   sem;
    };
    
    inline void SemaphoreCreate( semaphoreid_t& semaphoreid )
    {
        semaphoreid.sem = dispatch_semaphore_create(0);
    }
    
    inline void SemaphoreClose( semaphoreid_t& semaphoreid )
    {
        dispatch_release( semaphoreid.sem );
    }
    
    inline void SemaphoreWait( semaphoreid_t& semaphoreid  )
    {
        dispatch_semaphore_wait( semaphoreid.sem, DISPATCH_TIME_FOREVER );
    }
    
    inline void SemaphoreSignal( semaphoreid_t& semaphoreid, int32_t countWaiting )
    {
        while( countWaiting-- > 0 )
        {
            dispatch_semaphore_signal( semaphoreid.sem );
        }
    }
}

#else // POSIX

#include <semaphore.h>
#include <errno.h>

namespace enki
{
    
    struct semaphoreid_t
    {
        sem_t   sem;
    };
    
    inline void SemaphoreCreate( semaphoreid_t& semaphoreid )
    {
        int err = sem_init( &semaphoreid.sem, 0, 0 );
        ENKI_ASSERT( err == 0 );
        (void)err;
    }
    
    inline void SemaphoreClose( semaphoreid_t& semaphoreid )
    {
        sem_destroy( &semaphoreid.sem );
    }
    
    inline void SemaphoreWait( semaphoreid_t& semaphoreid  )
    {
        while( sem_wait( &semaphoreid.sem ) == -1 && errno == EINTR ) {}
    }
    
    inline void SemaphoreSignal( semaphoreid_t& semaphoreid, int32_t countWaiting )
    {
        while( countWaiting-- > 0 )
        {
            sem_post( &semaphoreid.sem );
        }
    }
}
#endif

semaphoreid_t* TaskScheduler::SemaphoreNew()
{
    semaphoreid_t* pSemaphore = this->Alloc<semaphoreid_t>( ENKI_FILE_AND_LINE );
    SemaphoreCreate( *pSemaphore );
    return pSemaphore;
}

void TaskScheduler::SemaphoreDelete( semaphoreid_t* pSemaphore_ )
{
    SemaphoreClose( *pSemaphore_ );
    this->Free( pSemaphore_, ENKI_FILE_AND_LINE );
}

void TaskScheduler::SetCustomAllocator( CustomAllocator customAllocator_ )
{
    m_Config.customAllocator = customAllocator_;
}

Dependency::Dependency( const ICompletable* pDependencyTask_, ICompletable* pTaskToRunOnCompletion_ ) 
    : pTaskToRunOnCompletion( pTaskToRunOnCompletion_ )
    , pDependencyTask( pDependencyTask_ )
    , pNext( pDependencyTask->m_pDependents )
{
    ENKI_ASSERT( pDependencyTask->GetIsComplete() );
    ENKI_ASSERT( pTaskToRunOnCompletion->GetIsComplete() );
    pDependencyTask->m_pDependents = this;
    ++pTaskToRunOnCompletion->m_DependenciesCount;
}

Dependency::Dependency( Dependency&& rhs_ ) noexcept
{
    pDependencyTask   = rhs_.pDependencyTask;
    pTaskToRunOnCompletion = rhs_.pTaskToRunOnCompletion;
    pNext             = rhs_.pNext;
    if( rhs_.pDependencyTask )
    {
        ENKI_ASSERT( rhs_.pTaskToRunOnCompletion );
        ENKI_ASSERT( rhs_.pDependencyTask->GetIsComplete() );
        ENKI_ASSERT( rhs_.pTaskToRunOnCompletion->GetIsComplete() );
        Dependency** ppDependent = &(pDependencyTask->m_pDependents);
        while( *ppDependent )
        {
            if( &rhs_ == *ppDependent )
            {
                *ppDependent = this;
                break;
            }
            ppDependent = &((*ppDependent)->pNext);
        }
    }
}


Dependency::~Dependency()
{
    ClearDependency();
}

void Dependency::SetDependency( const ICompletable* pDependencyTask_, ICompletable* pTaskToRunOnCompletion_ )
{
    ClearDependency();
    ENKI_ASSERT( pDependencyTask_->GetIsComplete() );
    ENKI_ASSERT( pTaskToRunOnCompletion_->GetIsComplete() );
    pDependencyTask = pDependencyTask_;
    pTaskToRunOnCompletion = pTaskToRunOnCompletion_;
    pNext = pDependencyTask->m_pDependents;
    pDependencyTask->m_pDependents = this;
    ++pTaskToRunOnCompletion->m_DependenciesCount;
}

void Dependency::ClearDependency()
{
    if( pDependencyTask )
    {
        ENKI_ASSERT( pTaskToRunOnCompletion );
        ENKI_ASSERT( pDependencyTask->GetIsComplete() );
        ENKI_ASSERT( pTaskToRunOnCompletion->GetIsComplete() );
        ENKI_ASSERT( pTaskToRunOnCompletion->m_DependenciesCount > 0 );
        Dependency* pDependent = pDependencyTask->m_pDependents;
        --pTaskToRunOnCompletion->m_DependenciesCount;
        if( this == pDependent )
        {
            pDependencyTask->m_pDependents = pDependent->pNext;
        }
        else
        {
            while( pDependent )
            {
                Dependency* pPrev = pDependent;
                pDependent = pDependent->pNext;
                if( this == pDependent )
                {
                    pPrev->pNext = pDependent->pNext;
                    break;
                }
            }
        }
    }
    pDependencyTask = NULL;
    pDependencyTask =  NULL;
    pNext = NULL;
}


#endif  //_TASKSCHEDULER_HPP_