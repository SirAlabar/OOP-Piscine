#ifndef TRAINADVANCERAILCOMMAND_HPP
#define TRAINADVANCERAILCOMMAND_HPP

#include "patterns/commands/ICommand.hpp"
#include <cstddef>
#include <string>

// Records when a train advances to a new rail segment.
// Replay: seeks the train's rail index to the recorded value.
class TrainAdvanceRailCommand : public ICommand
{
public:
    TrainAdvanceRailCommand(double             timestamp,
                            const std::string& trainName,
                            std::size_t        railIndex);

    void        execute()                              override;
    std::string serialize()                      const override;
    std::string getType()                        const override;
    double      getTimestamp()                   const override;
    void        applyReplay(SimulationManager* sim)    override;

private:
    double      _timestamp;
    std::string _trainName;
    std::size_t _railIndex;
};

#endif