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
        const char* mod1;
        const char* mod2;
        const char* mod3;
        const char* mod4;
        const char* mod5;
        const char* mod6;
        const char* mod7;
        const char* mod8;
        const char* mod9;
        const char* mod10;
        const char* mod11;
        const char* mod12;
        const char* mod13;
        const char* mod14;
        const char* mod15;
        const char* mod16;
        const char* mod17;
        const char* mod18;
        const char* mod19;
        const char* mod20;
        const char* mod21;
        const char* mod22;
    };
}