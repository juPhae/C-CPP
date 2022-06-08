#ifndef TIMER_H
#define TIMER_H

#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>

#define AUTORESET 1
#define NORESET   0

#define TASK_START(name, arg) void* name(void *arg){
#define TASK_END return NULL;} 

typedef void* (*TimerTask)(void* arg);

struct TaskItem{
    
    TimerTask task;
    int init_counter;
    int counter;
    pthread_t th;
    void *arg;
    void *ret;
    int flag;
    int autoreset;

    struct TaskItem *next;
    
};

void setTick(int val);

void* EventLoop(void* arg);

void addTimerTask(TimerTask task, int val, int autoreset, void *arg);

void TimerWait();

void TimerStop();

void StartTimer();


#endif //TIMER_H
