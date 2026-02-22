#ifndef ICOMMANDRECORDER_HPP
#define ICOMMANDRECORDER_HPP

class ICommand;

// Narrow interface that services use to record replay commands.
// SimulationManager implements this and injects itself into each service
// that needs to record.  Services never depend on CommandManager directly.
//
// record() takes ownership of cmd.  If recording is not active, the
// implementation must delete cmd to avoid leaking.
class ICommandRecorder
{
public:
    virtual ~ICommandRecorder() = default;

    virtual void record(ICommand* cmd) = 0;
};

#endif