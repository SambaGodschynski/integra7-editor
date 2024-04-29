#pragma once

#include <stddef.h>

class ISysexSender
{
public:
    virtual void sendSysex(const unsigned char*, size_t numBytes) = 0;
};