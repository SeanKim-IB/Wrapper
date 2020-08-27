#pragma once

#include <pthread.h>

class CIBCriticalSection {
public:
   CIBCriticalSection(VOID)
   {
      pthread_mutexattr_t mutexattr;   // Mutex attribute variable
      pthread_mutexattr_init(&mutexattr);
      // Set the mutex as a recursive mutex
      pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
      // create the mutex with the attributes set
      pthread_mutex_init(&mutex, &mutexattr);
      //After initializing the mutex, the thread attribute can be destroyed
      pthread_mutexattr_destroy(&mutexattr);
   }

   ~CIBCriticalSection(VOID)
   {
      // Destroy / close the mutex
      pthread_mutex_destroy (&mutex);
   }

   inline VOID Enter(VOID)
   {
      // Acquire the mutex to access the shared resource
      pthread_mutex_lock (&mutex);
   }

   inline VOID Leave(VOID)
   {
      // Release the mutex  and release the access to shared resource
      pthread_mutex_unlock (&mutex);
   }

private:
   pthread_mutex_t mutex;           // Mutex
};