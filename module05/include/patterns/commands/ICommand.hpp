#ifndef ICOMMAND_HPP
#define ICOMMAND_HPP

#include <string>

class SimulationManager;

// Abstract base for all recordable simulation actions.
// execute() / undo() are used for hot-reload reversal.
// applyReplay() is used during replay mode to reapply the action.
class ICommand
{
public:
    virtual ~ICommand() = default;

    // Apply the action (used for undo/redo with ReloadCommand)
    virtual void execute() = 0;

    // Reverse the action (hot-reload undo)
    virtual void undo() = 0;

    // Serialize to a single-line JSON object string (no trailing comma)
    virtual std::string serialize() const = 0;

    // Short type tag written to JSON (e.g. "STATE_CHANGE", "DEPARTURE")
    virtual std::string getType() const = 0;

    // Simulation time in seconds when this command was recorded
    virtual double getTimestamp() const = 0;

    // Apply this command during replay (default: no-op)
    virtual void applyReplay(SimulationManager*) {};
};

#endif