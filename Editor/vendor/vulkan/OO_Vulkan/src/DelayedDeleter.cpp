#include "DelayedDeleter.h"
#include <cassert>
#include <algorithm>

DelayedDeleter* DelayedDeleter::get()
{
    if (s_deleter == nullptr)
    {
        s_deleter = new DelayedDeleter();
    }
    return s_deleter;
}

void DelayedDeleter::Shutdown()
{
    auto* p = get();
    p->Clear();
    delete p;
}

void DelayedDeleter::DeleteAfterFrames(std::function<void()> fn, size_t frames)
{
    assert(frames);
    frames = std::min(frames, 3ull);

    auto currFrame = m_frame + frames;

    m_funcs[currFrame%MAX_FRAMES].emplace_back(fn);
}

void DelayedDeleter::DeleteAfterSeconds(std::function<void()> fn, float seconds)
{
}

void DelayedDeleter::Update(float deltaTime)
{
    ++m_frame;

    auto currFrame = m_frame % MAX_FRAMES;

    for (auto& f : m_funcs[currFrame])
    {
        // invoker deleter
        f();
    }
    m_funcs[currFrame].clear();

}

void DelayedDeleter::Clear()
{
    for (auto& batch : m_funcs)
    {
        for (auto& f : batch)
        {
            // invoker deleter
            f();
        }
        batch.clear();
    }
}


