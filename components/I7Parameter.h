#pragma once

#include <integra7/Model.h>
#include <stdexcept>
#include "I7Host.h"

namespace juce
{
    class Component;
}

template<class TControlComponent>
class I7Parameter : public TControlComponent
{
public:
    typedef TControlComponent ControllerBase;
    typedef typename ControllerBase::ControlerValueType T;
    I7Parameter(const char *nodeId, I7Host* _i7Host) :  i7Host(_i7Host) 
    {
        nodeInfo = i7::getNode(nodeId);
        if (nodeInfo.node == nullptr)
        {
            throw std::runtime_error(std::string("missing model for id: ") + nodeId);
        }
        if (i7Host == nullptr)
        {
            throw std::runtime_error("missing i7Host");
        }
        ControllerBase::i7setControlLimits(nodeInfo.node->min, nodeInfo.node->max);
        i7::put(&i7Host->model, nodeInfo, ControllerBase::i7GetDefaultValue(nodeInfo.node->init));
        ControllerBase::i7setValue(ControllerBase::i7GetDefaultValue(nodeInfo.node->init));
    }
protected:
    virtual void i7onValueChanged(T v) override;
private:
    i7::NodeInfo nodeInfo;
    I7Host *i7Host;
};

template<class TControlComponent>
void I7Parameter<TControlComponent>::i7onValueChanged(T v)
{
    i7::put(&i7Host->model, nodeInfo, v);
    i7::Bytes sysexMsg = i7::createSysexData(&i7Host->model, nodeInfo);
    i7Host->sendSysex(sysexMsg.data(), sysexMsg.size());
}