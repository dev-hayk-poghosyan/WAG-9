#include "EventExecutor.h"
#include "ExecEvent.h"

EventExecutor::EventExecutor(QObject* parent)
    : QObject(parent)
{
}

void
EventExecutor::customEvent(QEvent* evt)
{
    if (evt->type() == ExecEvent::registeredType())
    {
        evt->setAccepted(true);
        static_cast<ExecEvent*>(evt)->exec();
    }
}
