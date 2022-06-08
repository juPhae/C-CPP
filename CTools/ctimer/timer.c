#include "timer.h"

#define STOPFLAG 0
#define RUNFLAG 1 

static int tick = 1000 * 1000;
static struct TaskItem head = {
    .next = NULL,
};
static pthread_t loop_thread;
static int flag = STOPFLAG;
static int tasknum  = 0;

void setTick(int val){
    tick = val;
}

void* EventLoop(void* arg){
    
    struct TaskItem *task = head.next;
    struct TaskItem *pretask = &head;

    while(flag == RUNFLAG && tasknum > 0){
        
        while(task != NULL){
            if(task->counter == 0){ // it is time for doing task
                if(task->flag == STOPFLAG){ // task is not created
                    if(0 != pthread_create(&(task->th), NULL, task->task, task->arg)){ // do a task
                        printf("Failed to create user's task");
                    }
                    else{
                        task->flag = RUNFLAG;
                    }
                }
                else{
                    if(0 != pthread_kill(task->th, 0)){ // current task is completed
                        if(task->autoreset == AUTORESET){ // repeat execute
                            task->counter = task->init_counter;
                            task->flag = STOPFLAG;
                        }
                        else{ // delete a task
                            pretask->next = task->next;
                            free(task);
                            task = pretask->next;
                            tasknum--;
                            continue;
                        }
                    }
                }
            }
            else{
                task->counter--;
            }
            pretask = pretask->next;
            task = task->next;
        }
        
        usleep(tick); // sleep a tick
        task = head.next;
        pretask = &head;

    }

    flag = STOPFLAG;

}

void addTimerTask(TimerTask task, int val, int autoreset, void *arg){
    struct TaskItem *node;

    node = (struct TaskItem*)malloc(sizeof(struct TaskItem));
    node->next = head.next;
    head.next = node;

    node->arg = arg;
    node->counter = val;
    node->init_counter = val;
    node->task = task;
    node->flag = STOPFLAG;
    node->autoreset = autoreset;

    tasknum++;

}

void TimerWait(){

    pthread_join(loop_thread, NULL);

}

void TimerStop(){
    flag = STOPFLAG;
}

void StartTimer(){
    
    flag = RUNFLAG;
    if(0 != pthread_create(&loop_thread, NULL, EventLoop, NULL)){
        printf("Failed to create loop task.\n");
    }

}


