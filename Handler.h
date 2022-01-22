#ifndef HANDLER_H
#define HANDLER_H

#include <stdint.h>

struct Handler {
    virtual ~Handler(){}
    virtual void handleEvent(uint32_t events) = 0;
};

#endif