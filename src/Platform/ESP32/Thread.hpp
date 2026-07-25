#pragma once

extern "C" {
    #include <freertos/FreeRTOS.h>
    #include <freertos/semphr.h>
    #include <freertos/task.h>
}

namespace PedalGuru {

class Mutex {
private:
    static SemaphoreHandle_t lock_;
public:
    void Lock();
    void Release();
};

class Thread {
public:
    static void NewThread(void (*entry)());
};

}