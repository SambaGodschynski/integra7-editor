#pragma once

#include <integra7/Model.h>
#include <integra7/Expansion.h>
#include <integra7/PartType.h>
#include <future>
class I7Host
{
public:
    typedef std::promise<i7::RequestResponse> RequestResponsePromise;
    typedef std::future<i7::RequestResponse> RequestResponseFuture;
    enum { NumExpansions = 4 };
    enum { NumParts = 16 };
    virtual i7::ModelData* getModel() = 0;
    virtual const i7::ModelData* getModel() const = 0;
    virtual void sendSysex(const unsigned char*, size_t numBytes) = 0;
    virtual void requestExpansion() = 0;
    virtual void requestToneType(int partNumber) = 0;
    virtual void requestPartSetup(int partNumber) = 0;
    virtual void requestPart(int partNumber) = 0;
    virtual RequestResponseFuture request(const i7::RequestInfo &requestInfo) = 0;
    virtual ~I7Host() = default;
};