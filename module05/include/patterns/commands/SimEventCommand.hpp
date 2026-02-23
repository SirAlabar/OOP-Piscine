#ifndef SIMEVENTCOMMAND_HPP
#define SIMEVENTCOMMAND_HPP

#include "patterns/commands/ICommand.hpp"

class IReplayTarget;
#include <string>

// Records a simulation event activation (weather, signal failure, etc.).
// Replay: logs the event; rail/node effects are not re-applied.
class SimEventCommand : public ICommand
{
public:
    SimEventCommand(double             timestamp,
                    const std::string& eventType,
                    const std::string& description);

    void        execute()                              override;
    std::string serialize()                      const override;
    std::string getType()                        const override;
    double      getTimestamp()                   const override;
    void        applyReplay(IReplayTarget* target)    override;

private:
    double      _timestamp;
    std::string _eventType;
    std::string _description;
};

#endif