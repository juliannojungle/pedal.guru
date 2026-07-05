namespace OpenCC {
#include "Thread.hpp"

Mutex::lock_ = PTHREAD_MUTEX_INITIALIZER;

void Mutex::Lock() {
    if (!mutex_is_initialized(&lock_)) {
        mutex_init(&lock_);
    }
    mutex_enter_blocking(&lock_);
}

void Mutex::Release() {
    mutex_exit(&lock_);
}

}