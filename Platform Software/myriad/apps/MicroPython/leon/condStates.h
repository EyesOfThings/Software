
#ifndef COND_STATES
#define COND_STATES


#include <rtems.h>
#include <semaphore.h>
#include <pthread.h>

typedef enum 
{
    EXECUTE_PYTHON,
    SHOW_IMAGE,
    PYTHON_END,
    SEND_OUTPUT
}condState; 

extern condState state;

extern pthread_cond_t   pythonTurn;
extern pthread_cond_t   pulgaTurn;
extern pthread_mutex_t  mutex;


#endif
