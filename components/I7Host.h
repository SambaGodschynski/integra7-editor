#pragma once

#include <stddef.h>

class I7Host
{
public:
    virtual void sendSysex(const unsigned char*, size_t numBytes) = 0;
};