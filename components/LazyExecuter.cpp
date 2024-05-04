#include "LazyExecuter.h"


void LazyExecuter::handleAsyncUpdate()
{
    const std::lock_guard<Mutex> lock(todoMutex);
    for(const auto& wf : todo)
    {
        wf();
    }
    todo.clear();
}

void LazyExecuter::doLater(const WorkFunction& wf)
{
    const std::lock_guard<Mutex> lock(todoMutex);
    todo.push_back(wf);
    triggerAsyncUpdate();
}