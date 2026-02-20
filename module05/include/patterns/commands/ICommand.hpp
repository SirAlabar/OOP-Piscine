#ifndef ICOMMAND_HPP
#define ICOMMAND_HPP

#include <string>

class SimulationManager;

// Abstract base for all recordable simulation actions.
class ICommand
{
public:
    virtual ~ICommand() = default;

    // Apply the action (used when command is first executed).
    virtual void execute() = 0;

    // Serialize to a single-line JSON object string (no trailing comma).
    virtual std::string serialize() const = 0;

    // Short type tag written to JSON (e.g. "STATE_CHANGE", "DEPARTURE").
    virtual std::string getType() const = 0;

    // Simulation time in seconds when this command was recorded.
    virtual double getTimestamp() const = 0;

    // Re-apply this command during replay mode (default: no-op).
    virtual void applyReplay(SimulationManager*) {}
};

#endif