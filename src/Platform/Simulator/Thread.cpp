namespace OpenCC {
#include "Thread.hpp"

Mutex::lock_ = PTHREAD_MUTEX_INITIALIZER;

void Mutex::Lock() {
    pthread_mutex_lock(&lock_);
}

void Mutex::Release() {
    pthread_mutex_unlock(&lock_);
}

}