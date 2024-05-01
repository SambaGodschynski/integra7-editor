#pragma once

#include<string>

struct PartInfo
{
    int partId = 1;
    std::string createId(const char *id) const
    {
        return std::string("PRM-_FPART") + std::to_string(partId) + "-" + id;
    }
};