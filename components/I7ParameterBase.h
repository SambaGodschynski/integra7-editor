#pragma once

class I7ParameterBase
{
public:
    virtual ~I7ParameterBase() {}
    virtual void modelValueChanged() = 0;
};