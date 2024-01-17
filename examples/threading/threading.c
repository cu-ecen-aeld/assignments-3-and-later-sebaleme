#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)
// Arbitrary define 100 as max number of threads
#define MAX_NUM_OF_THREADS 100

void* threadfunc(void* thread_param)
{
    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    printf("[CHILD TREAD] Entering child thread\n");
    // Cast input param back to a useful type
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    // Wait
    usleep(thread_func_args->wait_to_obtain_ms*1000);
    // Obtain mutex
    int rc = pthread_mutex_lock(thread_func_args->mutex);
    if(rc != 0)
    {
        DEBUG_LOG("[CHILD TREAD] could not get mutex");
    }
    // Wait
    usleep(thread_func_args->wait_to_release_ms*1000);
    // Release mutex
    rc = pthread_mutex_unlock(thread_func_args->mutex);
    if(rc != 0)
    {
        ERROR_LOG("[CHILD TREAD] could not release mutex");
    }
    thread_func_args->thread_complete_success = true;
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    bool result = false;
    // Allocate parameter structur
    struct thread_data* thread_data_ptr = (struct thread_data*)malloc(sizeof(struct thread_data));
    // Fill parameter structur
    thread_data_ptr->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_data_ptr->wait_to_release_ms = wait_to_release_ms;
    thread_data_ptr->mutex = mutex;
    printf("result initial value: %d, thread ID is %ld\n",  result, *thread);
    // Create thread with default configuration
    int rc = pthread_create(thread, NULL, threadfunc, thread_data_ptr);
    printf("pthread_create return value: %d and thread ID is now %ld\n", rc, *thread);
    // Need to free the dynamic allocated struct if pthread creation fails
    if(rc != 0)
    {
        free(thread_data_ptr);
    }

    return rc == 0;
}

