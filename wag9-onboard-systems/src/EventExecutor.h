#pragma once

#include <QObject>

/**
 * Catches instances of ExecEvent delivered to this object and executes them.
 *
 * This can be used as a general "execute this on that thread" system.
 *
 * @see ExecEvent
 */
class EventExecutor : public QObject
{
public:
    EventExecutor(QObject* parent = nullptr);

private:
    virtual void customEvent(QEvent* evt) override;
};
