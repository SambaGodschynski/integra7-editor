#pragma once

#include <stddef.h>

namespace i7
{
    typedef unsigned char Byte;
    typedef size_t CategoryId;
    struct Preset 
    {
        size_t index;
        Byte msb;
        Byte lsb;
        Byte pc;
        CategoryId category;
        const char * fullName;
    };
    struct SnaInstr
    {
        const char* bank;
        const char* desc;
        const char* cate;
        Byte lsb;
        Byte pc;
        enum { NumMods = 22 };
        const char* mods[NumMods];
    };
}