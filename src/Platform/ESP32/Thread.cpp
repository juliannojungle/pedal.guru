namespace PedalGuru {
#include "Thread.hpp"

Mutex::lock_ = xSemaphoreCreateMutex();

void Mutex::Lock() {
    xSemaphoreTake(lock_, portMAX_DELAY);
}

void Mutex::Release() {
    xSemaphoreGive(lock_);
}

void Thread::NewThread(void (*entry)()) {
    xTaskCreate([](void* param) {
        auto func = reinterpret_cast<void(*)()>(param);
        func();
        vTaskDelete(NULL);
    }, "NewThread", 4096, reinterpret_cast<void*>(entry), 1, NULL);
}

}