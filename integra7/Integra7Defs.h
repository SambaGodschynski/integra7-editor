#pragma once

#include <vector>

namespace i7
{
    typedef unsigned char Byte;
    typedef std::vector<Byte> Bytes;
    typedef unsigned int UInt;
    typedef UInt CategoryId;
    struct Preset 
    {
        UInt index;
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
    constexpr UInt nibble(UInt x) {
        return (
            ((x & 0x7f000000) >> 3) |
            ((x & 0x007f0000) >> 2) |
            ((x & 0x00007f00) >> 1) |
            ((x & 0x0000007f))
        );
    }
    constexpr UInt expand(UInt x) {
        return (
            ((x & 0x0fe00000) << 3) |
            ((x & 0x001fc000) << 2) |
            ((x & 0x00003f80) << 1) |
            ((x & 0x0000007f))
        );
    }
}