#ifndef RELOADCOMMAND_HPP
#define RELOADCOMMAND_HPP

#include "patterns/commands/ICommand.hpp"
#include <functional>
#include <string>

// Records a hot-reload event.
// undo() restores the simulation to the state before the reload by
// writing the previous file contents to temp files and invoking the rebuild callback.
class ReloadCommand : public ICommand
{
public:
    // rebuildFn: called with (netFilePath, trainFilePath); returns true on success.
    // The caller (Application) is responsible for stopping the sim thread before rebuild.
    using RebuildFn = std::function<bool(const std::string& netFile,
                                         const std::string& trainFile)>;

    // oldNetContent / oldTrainContent: raw file contents BEFORE this reload.
    // newNetFile / newTrainFile: paths used for the new (current) simulation.
    ReloadCommand(double      timestamp,
                  std::string oldNetContent,
                  std::string oldTrainContent,
                  std::string newNetFile,
                  std::string newTrainFile,
                  RebuildFn   rebuildFn);

    // No-op: the reload was already carried out by Application.
    void execute() override;

    // Write old contents to temp files and trigger a rebuild with them.
    void undo() override;

    std::string serialize()                      const override;
    std::string getType()                        const override;
    double      getTimestamp()                   const override;
    void        applyReplay(SimulationManager* sim)    override;

private:
    double      _timestamp;
    std::string _oldNetContent;
    std::string _oldTrainContent;
    std::string _newNetFile;
    std::string _newTrainFile;
    RebuildFn   _rebuildFn;

    static std::string writeTempFile(const std::string& path, const std::string& content);
};

#endif