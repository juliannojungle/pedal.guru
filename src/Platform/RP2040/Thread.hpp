#pragma once

namespace OpenCC {

extern "C" {
    #include <pico/mutex.h>
}

class Mutex {
private:
    static mutex_t lock_;
public:
    void Lock();
    void Release();
};

}