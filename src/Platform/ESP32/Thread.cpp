namespace PedalGuru {
#include "Thread.hpp"

Mutex::lock_ = xSemaphoreCreateMutex();

void Mutex::Lock() {
    xSemaphoreTake(lock_, portMAX_DELAY);
}

void Mutex::Release() {
    xSemaphoreGive(lock_);
}

}