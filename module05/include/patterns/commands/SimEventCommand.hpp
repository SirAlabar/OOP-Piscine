#ifndef SIMEVENTCOMMAND_HPP
#define SIMEVENTCOMMAND_HPP

#include "patterns/commands/ICommand.hpp"
#include <string>

// Records a simulation event activation (weather, signal failure, etc.).
// Replay: logs the event; rail/node effects are not replayed.
class SimEventCommand : public ICommand
{
public:
    SimEventCommand(double             timestamp,
                    const std::string& eventType,
                    const std::string& description);

    void        execute()                              override;
    void        undo()                                 override;
    std::string serialize()                      const override;
    std::string getType()                        const override;
    double      getTimestamp()                   const override;
    void        applyReplay(SimulationManager* sim)    override;

private:
    double      _timestamp;
    std::string _eventType;
    std::string _description;
};

#endif