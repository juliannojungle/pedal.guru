#pragma once

namespace PedalGuru {

extern "C" {
    #include <freertos/FreeRTOS.h>
    #include <freertos/semphr.h>
}

class Mutex {
private:
    static SemaphoreHandle_t lock_;
public:
    void Lock();
    void Release();
};

}