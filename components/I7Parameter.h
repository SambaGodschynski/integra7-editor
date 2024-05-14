#pragma once

#include <integra7/Model.h>
#include <stdexcept>
#include "I7Host.h"
#include <string>
namespace juce
{
    class Component;
}

class I7ParameterBase
{
public:
    virtual ~I7ParameterBase() {}
};

template<class TControlComponent>
class I7Parameter : public TControlComponent, public I7ParameterBase
{
public:
    typedef TControlComponent ControllerBase;
    typedef typename ControllerBase::ControlerValueType T;
    I7Parameter(const char *nodeId, I7Host* _i7Host) :  i7Host(nullptr) 
    {
        nodeInfo = i7::getNode(nodeId);
        if (nodeInfo.node == nullptr)
        {
            throw std::runtime_error(std::string("missing model for id: ") + nodeId);
        }
        ControllerBase::i7setControlLimits(nodeInfo.node->min, nodeInfo.node->max);
        ControllerBase::i7setValue(ControllerBase::i7GetDefaultValue(nodeInfo.node->init));
        i7Host = _i7Host; // arm onValueChanged
        if (i7Host == nullptr)
        {
            throw std::runtime_error("missing i7Host");
        }
        i7::put(&i7Host->model, nodeInfo, ControllerBase::i7GetDefaultValue(nodeInfo.node->init));
    }
    virtual ~I7Parameter() {}
    std::string i7getDescription() const { return nodeInfo.node->desc; }
protected:
    virtual void i7onValueChanged(T v) override;
    virtual void i7putValue(const char *nodeId, i7::UInt v);
    virtual I7Host* getHost() { return i7Host; }
private:
    i7::NodeInfo nodeInfo;
    I7Host *i7Host;
};

template<class TControlComponent>
void I7Parameter<TControlComponent>::i7onValueChanged(T v)
{
    if (!i7Host)
    {
        return;
    }
    i7::put(&i7Host->model, nodeInfo, v);
    i7::Bytes sysexMsg = i7::createSysexData(&i7Host->model, nodeInfo);
    i7Host->sendSysex(sysexMsg.data(), sysexMsg.size());
}

template<class TControlComponent>
void I7Parameter<TControlComponent>::i7putValue(const char* nodeId, i7::UInt v)
{
    if (!i7Host)
    {
        return;
    }
    auto otherNodeInfo = i7::getNode(nodeId);
    if (otherNodeInfo.node == nullptr)
    {
        throw std::runtime_error(std::string("missing model for id: ") + nodeId);
    }
    i7::put(&i7Host->model, otherNodeInfo, v);
    i7::Bytes sysexMsg = i7::createSysexData(&i7Host->model, otherNodeInfo);
    i7Host->sendSysex(sysexMsg.data(), sysexMsg.size());
}
