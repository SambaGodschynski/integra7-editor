#pragma once

#include <integra7/Model.h>
#include <stdexcept>
#include "I7Host.h"
#include <string>
#include <iostream>
#include "I7ParameterBase.h"
#include <components/Common.h>

namespace juce
{
    class Component;
}

template<class TControlComponent>
class I7Parameter : public TControlComponent, public I7ParameterBase
{
public:
    typedef TControlComponent ControllerBase;
    typedef typename ControllerBase::ControlerValueType T;
    I7Parameter(const char *_nodeId, I7Host* _i7Host) :  i7Host(nullptr), nodeId(_nodeId), omitSendSysex(true)
    {
        _i7Host->registerParameter(this);
        nodeInfo = i7::getNode(nodeId.c_str());
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
        i7::put(i7Host->getModel(), nodeInfo, ControllerBase::i7GetDefaultValue(nodeInfo.node->init));
        omitSendSysex = false;
    }
    virtual ~I7Parameter() 
    {
        i7Host->unregisterParameter(this);
    }
    std::string i7getDescription() const { return nodeInfo.node->desc; }
    virtual void modelValueChanged() override;
protected:
    virtual void i7onValueChanged(T v) override;
    virtual void i7putValue(const char *nodeId, i7::UInt v);
    virtual I7Host* getHost() { return i7Host; }
private:
    i7::NodeInfo nodeInfo;
    I7Host *i7Host;
    std::string nodeId;
    bool omitSendSysex;
};

template<class TControlComponent>
void I7Parameter<TControlComponent>::i7onValueChanged(T v)
{
    if (!i7Host || omitSendSysex)
    {
        return;
    }
    i7::put(i7Host->getModel(), nodeInfo, v);
    i7::Bytes sysexMsg = i7::createSysexData(i7Host->getModel(), nodeInfo);
    i7Host->sendSysex(sysexMsg.data(), sysexMsg.size());
    DEBUGONLY(std::cout << nodeId << std::endl);
}

template<class TControlComponent>
void I7Parameter<TControlComponent>::i7putValue(const char* otherNodeId, i7::UInt v)
{
    if (!i7Host)
    {
        return;
    }
    auto otherNodeInfo = i7::getNode(otherNodeId);
    if (otherNodeInfo.node == nullptr)
    {
        throw std::runtime_error(std::string("missing model for id: ") + otherNodeId);
    }
    i7::put(i7Host->getModel(), otherNodeInfo, v);
    i7::Bytes sysexMsg = i7::createSysexData(i7Host->getModel(), otherNodeInfo);
    i7Host->sendSysex(sysexMsg.data(), sysexMsg.size());
    DEBUGONLY(std::cout << otherNodeId << std::endl);
}

template<class TControlComponent>
void I7Parameter<TControlComponent>::modelValueChanged()
{
    omitSendSysex = true;
    auto v = i7::get(i7Host->getModel(), nodeInfo);
    ControllerBase::i7setValue(v);
    omitSendSysex = false;
}
