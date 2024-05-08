#pragma once

#include <integra7/Model.h>

class I7Host
{
public:
    i7::ModelData model;
    virtual void sendSysex(const unsigned char*, size_t numBytes) = 0;
};