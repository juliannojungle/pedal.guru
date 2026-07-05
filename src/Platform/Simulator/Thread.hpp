#pragma once

namespace OpenCC {

extern "C" {
    #include <pthread.h>
}

class Mutex {
private:
    static pthread_mutex_t lock_;
public:
    void Lock();
    void Release();
};

}