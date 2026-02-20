#ifndef TRAINDEPARTURECOMMAND_HPP
#define TRAINDEPARTURECOMMAND_HPP

#include "patterns/commands/ICommand.hpp"
#include <string>

// Records the moment a train departs (Idle -> Accelerating).
class TrainDepartureCommand : public ICommand
{
public:
    TrainDepartureCommand(double timestamp, const std::string& trainName);

    void        execute()                              override;
    std::string serialize()                      const override;
    std::string getType()                        const override;
    double      getTimestamp()                   const override;
    void        applyReplay(SimulationManager* sim)    override;

private:
    double      _timestamp;
    std::string _trainName;
};

#endif