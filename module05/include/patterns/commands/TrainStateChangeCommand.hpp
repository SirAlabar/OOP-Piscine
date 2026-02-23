#ifndef TRAINSTATECHANGECOMMAND_HPP
#define TRAINSTATECHANGECOMMAND_HPP

#include "patterns/commands/ICommand.hpp"

class IReplayTarget;
#include <string>

// Records a train state transition.
// Replay: forces the named train into the recorded target state.
class TrainStateChangeCommand : public ICommand
{
public:
    TrainStateChangeCommand(double             timestamp,
                            const std::string& trainName,
                            const std::string& fromState,
                            const std::string& toState);

    void        execute()                              override;
    std::string serialize()                      const override;
    std::string getType()                        const override;
    double      getTimestamp()                   const override;
    void        applyReplay(IReplayTarget* target)    override;

private:
    double      _timestamp;
    std::string _trainName;
    std::string _fromState;
    std::string _toState;
};

#endif