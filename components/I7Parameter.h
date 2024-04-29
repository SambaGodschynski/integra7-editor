#pragma once

#include <integra7/Model.h>
#include <stdexcept>
#include "ISysexSender.h"

namespace juce
{
    class Component;
}
extern ISysexSender* getSysexSender(const juce::Component*); 

template<class TControlComponent>
class I7Parameter : public TControlComponent
{
public:
    typedef TControlComponent ControlerBase;
    typedef typename ControlerBase::ControlerValueType T;
    void setModel(const i7::NodeInfo& n) { model = n; }
    const i7::NodeInfo& getModel() const { return model; }
    void setSysexSender(ISysexSender *snd) { sysexSender = snd; }
    ISysexSender* getSysexSender() const { return sysexSender; }
protected:
    virtual void onValueChanged(T v) override;
private:
    i7::NodeInfo model;
    ISysexSender *sysexSender = nullptr;
};

template<class TControlComponent>
void I7Parameter<TControlComponent>::onValueChanged(T v)
{
    if (model.node == nullptr)
    {
        throw std::runtime_error("missing model");
    }
    if (sysexSender == nullptr)
    {
        throw std::runtime_error("missing sender");
    }
    i7::put(model, v);
    i7::Bytes sysexMsg = i7::createSysexData(model);
    sysexSender->sendSysex(sysexMsg.data(), sysexMsg.size());
}