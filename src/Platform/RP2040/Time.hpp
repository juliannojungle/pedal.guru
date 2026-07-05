namespace OpenCC {

extern "C" {
    #include "pico/time.h"
}

class Time {
public:
    void Delay(milliseconds: unsigned int);
};

}