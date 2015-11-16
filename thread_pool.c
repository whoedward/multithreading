#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "thread_pool.h"

/**
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  Feel free to make any modifications you want to the function prototypes and structs
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */

#define MAX_THREADS 20
#define STANDBY_SIZE 10

typedef struct {
    void (*function)(void *);
    void *argument;
    struct pool_task_t* next;
} pool_task_t;


struct pool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
  pool_task_t *queue;
  int thread_count;
  int task_queue_size_limit;
};

static void *thread_do_work(void *pool);


/*
 * Create a threadpool, initialize variables, etc
 *
 */
pool_t *pool_create(int queue_size, int num_threads)
{
   printf("creating threadpool\n");
   int i;
   //need to initailize lock, condition, attr, array of threads
   pthread_mutex_t mutex;
   pthread_cond_t cond;
   pthread_t * thread_array = (pthread_t*) malloc(sizeof(pthread_t)*num_threads);

   //set values
   pool_t* threadpool = (pool_t*) malloc(sizeof(pool_t));
   //init pthreads
   pthread_mutex_init(&mutex,NULL);
   threadpool->lock = mutex;
   pthread_cond_init(&cond,NULL);
   threadpool->notify = cond;

   threadpool->threads = thread_array;
   printf("here\n");
   for(i=0; i < num_threads; i++){
       pthread_create(&(threadpool->threads[i]),NULL,thread_do_work,(void*)threadpool);
   }
    threadpool->task_queue_size_limit = queue_size;
    threadpool->thread_count = num_threads;
printf("here\n");
    threadpool->queue = (pool_task_t*)malloc(sizeof(pool_task_t) * (queue_size+1));
    //puts("return threadpool");
    return threadpool;
}

/*
 * Add a task to the threadpool
 *
 */
int pool_add_task(pool_t *pool, void (*function)(void *), void *argument)
{
    int err = 0;

    pthread_mutex_t* lock = &(pool->lock);
    err = pthread_mutex_lock(lock);
    if(err){
      return -1;
    }

    //todo: create new task
    pool_task_t* task = (pool_task_t*) malloc(sizeof(pool_task_t));
    task->function = function;
    task->argument = argument;
    task->next = NULL;

    if(pool->queue){
      //queue not empty
      pool_task_t* current = pool->queue;
      //todo: basic linked list traversals
      while(current->next != NULL)
        current = (pool_task_t*)current->next;

      current->next = (struct pool_task_t*) task;
    }else{
      //queue is empty
      pool->queue = task;
    }

    err = pthread_cond_broadcast(&(pool->notify));
    if(err){
      // err in broadcoasting condition
      return -1;
    }
    err = pthread_mutex_unlock(lock);
    if(err)
      //error in unlocking the lock
      return -1;
    return err;
}

/*
 * Destroy the threadpool, free all memory, destroy treads, etc
 *
 */
int pool_destroy(pool_t *pool)
{
    int err = 0;
    int i;
    pool_add_task(pool, NULL, NULL);

    //join worker threads
    for(i =0; i < pool->thread_count;i++){
      pthread_join(pool->threads[i],NULL);
    }
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->notify);
    free((void*)pool->queue);
    free((void*)pool->threads);
    free((void*)pool);

    return err;
}



/*
 * Work loop for threads. Should be passed into the pthread_create() method.
 *
 */
static void *thread_do_work(void *pool)
{

    while(1) {
      pool_t* threadpool = (pool_t*) pool;
      pthread_mutex_t* lock = &(threadpool->lock);
      pthread_cond_t* cond = &(threadpool->notify);
      //check for errors in lock, condition
      pool_task_t* current = threadpool->queue;
      if(!current){
        pthread_mutex_unlock(lock);
      }

      if(current->function == NULL){
        pthread_mutex_unlock(&(threadpool->lock));
        pthread_exit(NULL);
        return NULL;
      }
      //delete from LL
      threadpool->queue = (pool_task_t*)threadpool->queue->next;

      //idk whats goin on here
      void (*function)(void*);
      void* argument;
      function = current->function;
      argument = current->argument;
      free ((void*)current);
      function(argument);

    }

    pthread_exit(NULL);
    return(NULL);
}
