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
    
    // Cast input param back to a useful type
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    // Until now, threading has been successful
    thread_func_args->thread_complete_success = true;
    // Wait
    usleep(thread_func_args->wait_to_obtain_ms);
    // Obtain mutex
    int rc = pthread_mutex_lock(thread_func_args->mutex);
    if(rc != 0)
    {
        ERROR_LOG("could not get mutex");
        thread_func_args->thread_complete_success = false;
    }
    // Wait
    usleep(thread_func_args->wait_to_release_ms);
    // Release mutex
    rc = pthread_mutex_unlock(thread_func_args->mutex);
    if(rc != 0)
    {
        ERROR_LOG("could not release mutex");
        thread_func_args->thread_complete_success = false;
    }
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

    // Create thread with default configuration
    result = pthread_create(thread, NULL, threadfunc, thread_data_ptr);
    // Only proceed if the child thread could be created
    if(result)
    {
        // Wait until the thread is done before returning
        pthread_join(*thread, (void **) &thread_data_ptr);
        // Update result with the child thread return value
        result = thread_data_ptr->thread_complete_success;
        // Release the allocated heap
        free(thread_data_ptr);
    }
    return result;
}

