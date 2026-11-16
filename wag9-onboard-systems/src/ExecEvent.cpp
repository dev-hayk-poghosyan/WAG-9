#include "ExecEvent.h"

ExecEvent::ExecEvent()
    : QEvent(registeredType())
{
}
    
QEvent::Type
ExecEvent::registeredType()
{
    static int const type = registerEventType();
    return static_cast<Type>(type);
}
