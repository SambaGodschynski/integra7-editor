#pragma once

#include <integra7/Model.h>
#include <integra7/Expansion.h>
#include <future>
class I7Host
{
public:
    typedef std::promise<i7::RequestResponse> RequestResponsePromise;
    typedef std::future<i7::RequestResponse> RequestResponseFuture;
    i7::ModelData model;
    enum { NumExpansions = 4 };
    i7::Expansion expansions[NumExpansions] = { i7::Expansion::NoAssing };
    virtual void sendSysex(const unsigned char*, size_t numBytes) = 0;
    virtual void requestExpansion() = 0;
    virtual RequestResponseFuture request(const i7::RequestInfo &requestInfo) = 0;
    virtual ~I7Host() = default;
};