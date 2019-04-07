#pragma once

#include <Fabula/types.h>
#include <Fabula/Layer/Event/Event.h>

struct ClickEvent : public fbl::IEvent
{
    EmitEventRTTI(ClickEvent)

public:
    float NDCXPos;
    float NDCYPos;
};

struct WindowFocusEvent : public fbl::IEvent
{
    EmitEventRTTI(WindowFocusEvent)

public:
    bool Focused;
};

struct WindowResizedEvent : public fbl::IEvent
{
    EmitEventRTTI(WindowResizedEvent)

public:
    i32 Width;
    i32 Height;
};
