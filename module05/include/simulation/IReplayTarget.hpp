#ifndef IREPLAYTARGET_HPP
#define IREPLAYTARGET_HPP

#include <string>

class Train;
class SimulationContext;

// Narrow interface that ICommand::applyReplay() requires from the simulation.
// SimulationManager implements this; commands depend only on this interface.
class IReplayTarget
{
public:
    virtual ~IReplayTarget() = default;

    virtual Train*             findTrain(const std::string& name) const = 0;
    virtual SimulationContext* getContext()                        const = 0;
};

#endif