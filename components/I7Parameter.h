#pragma once

#include <integra7/Model.h>
#include <stdexcept>
#include "ISysexSender.h"

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
    I7Parameter(const char *modelId, ISysexSender* _sysexSender) :  sysexSender(_sysexSender) 
    {
        model = i7::getNode(modelId);
        if (model.node == nullptr)
        {
            throw std::runtime_error(std::string("missing model for id: ") + modelId);
        }
        if (sysexSender == nullptr)
        {
            throw std::runtime_error("missing sender");
        }
        ControllerBase::i7setControlLimits(model.node->min, model.node->max);
        i7::put(model, ControllerBase::i7GetDefaultValue(model.node->init));
        ControllerBase::i7setValue(ControllerBase::i7GetDefaultValue(model.node->init));
    }
protected:
    virtual void i7onValueChanged(T v) override;
private:
    i7::NodeInfo model;
    ISysexSender *sysexSender;
};

template<class TControlComponent>
void I7Parameter<TControlComponent>::i7onValueChanged(T v)
{
    i7::put(model, v);
    i7::Bytes sysexMsg = i7::createSysexData(model);
    sysexSender->sendSysex(sysexMsg.data(), sysexMsg.size());
}