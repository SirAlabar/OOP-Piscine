#ifndef HOTRELOADSUPPORT_HPP
#define HOTRELOADSUPPORT_HPP

#include <functional>
#include <string>

class IOutputWriter;
class CommandManager;

class HotReloadSupport
{
public:
    explicit HotReloadSupport(IOutputWriter& output);

    bool validateFilesForReload(const std::string& netFile,
                                const std::string& trainFile) const;

    void recordReloadCommand(
        CommandManager* cmdMgr,
        double reloadTime,
        const std::string& oldNet,
        const std::string& oldTrain,
        const std::string& netFile,
        const std::string& trainFile,
        const std::function<bool(const std::string&, const std::string&)>& rebuildCallback) const;

    static std::string readFile(const std::string& path);

private:
    IOutputWriter& _output;
};

#endif
