#pragma once

class SignalStateAspectObstacle
{
public:
    virtual ~SignalStateAspectObstacle() = default;

    /**
     * In some signalling systems (e.g. Russian), a faulty signal may be treated
     * as an obstacle by the auto-blocking system.
     */
    virtual bool countsAsObstacle() const = 0;
};
