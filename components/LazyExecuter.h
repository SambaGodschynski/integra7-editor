#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <list>
#include <thread>

class LazyExecuter : public juce::AsyncUpdater
{
public:
    virtual void handleAsyncUpdate() override;
private:
    typedef std::function<void()> WorkFunction;
    typedef std::list<WorkFunction> Todo;
    typedef std::recursive_mutex Mutex;
    Mutex todoMutex;
    Todo todo;
public:
    void doLater(const WorkFunction&);

};
