#pragma once
#include <sstream>
#include <string>
#include <iomanip>

template<class TBytesIt>
 std::string bytesToString(TBytesIt begin, TBytesIt end)
 {
    TBytesIt it = begin;
    std::stringstream ss;
    ss << std::hex;
    for(; it != end; ++it)
    {
        ss << std::setfill('0') << std::setw(2) << ((unsigned int)(*it));
    }
    return ss.str();
 }
