#pragma once

#include <integra7/Model.h>
#include <vector>

class I7Host
{
public:
    i7::ModelData model;
    virtual void sendSysex(const unsigned char*, size_t numBytes) = 0;
    typedef std::vector<i7::ExtensionId> ExtensionIds;
    virtual ExtensionIds getCurrentExtensions() const = 0;
};