#pragma once

#include <string>

class I7ParameterBase
{
public:
    virtual ~I7ParameterBase() {}
    virtual void modelValueChanged() = 0;
    virtual std::string getNodeId() const = 0;
};