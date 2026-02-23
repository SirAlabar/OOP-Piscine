#include "app/ModeHandlers.hpp"
#include "app/HotReloadSupport.hpp"
#include "app/RunSession.hpp"
#include "analysis/MonteCarloRunner.hpp"
#include "core/Train.hpp"
#include "io/CLI.hpp"
#include "io/IOutputWriter.hpp"
#include "patterns/commands/CommandManager.hpp"
#include "rendering/SFMLRenderer.hpp"
#include "utils/FileSystemUtils.hpp"
#include "utils/FileWatcher.hpp"

namespace
{
int runPreparedSimulation(const std::string& netFile,
                         const std::string& trainFile,
                         RunSession& session,
                         bool listOutputs,
                         const std::function<int(SimulationBundle&, CommandManager*)>& runLoop)
{
    SimulationBundle                bundle;
    std::unique_ptr<CommandManager> cmdMgr = session.setupCommandManager();

    if (!session.buildSimulation(netFile, trainFile, bundle))
    {
        return 1;
    }

    if (cmdMgr)
    {
        session.simulation().setCommandManager(cmdMgr.get());
    }

    int status = runLoop(bundle, cmdMgr.get());

    if (listOutputs)
    {
        for (const Train* train : bundle.trains)
        {
            session.output().writeOutputFileListing("output/" + train->getName() + "_" +
                                                    train->getDepartureTime().toString() + ".result");
        }
    }

    session.finishRun(cmdMgr.get(), bundle, netFile, trainFile);
    return status;
}
} // namespace

bool MonteCarloModeHandler::matches(const CLI& cli) const
{
    return cli.hasMonteCarloRuns();
}

int MonteCarloModeHandler::run(const std::string& netFile,
                               const std::string& trainFile,
                               RunSession& session)
{
    try
    {
        FileSystemUtils::ensureOutputDirectoryExists();

        MonteCarloRunner runner(netFile,
                                trainFile,
                                session.cli().getSeed(),
                                session.cli().getMonteCarloRuns(),
                                session.cli().getPathfinding(),
                                &session.output());

        runner.runAll();
        runner.writeCSV("output/monte_carlo_results.csv");
        return 0;
    }
    catch (const std::exception& e)
    {
        session.output().writeError(e.what());
        return 1;
    }
}

bool ReplayModeHandler::matches(const CLI& cli) const
{
    return cli.hasReplay();
}

int ReplayModeHandler::run(const std::string& /*netFile*/,
                           const std::string& /*trainFile*/,
                           RunSession& session)
{
    CommandManager    cmdMgr;
    RecordingMetadata meta;

    if (!cmdMgr.loadFromFile(session.cli().getReplayFile(), meta))
    {
        session.output().writeError("Failed to load replay file: " + session.cli().getReplayFile());
        return 1;
    }

    cmdMgr.startReplay();
    session.output().writeProgress("Loaded " + std::to_string(cmdMgr.commandCount()) +
                                   " commands from " + session.cli().getReplayFile());

    SimulationBundle bundle;
    if (!session.buildSimulation(meta.networkFile, meta.trainFile, bundle, static_cast<int>(meta.seed)))
    {
        return 1;
    }

    session.simulation().setCommandManager(&cmdMgr);

    const double maxTime = (meta.stopTime > 0.0) ? meta.stopTime : 1e9;
    if (session.cli().hasRender())
    {
        SFMLRenderer renderer;
        session.simulation().run(maxTime, true, true, &renderer);
    }
    else
    {
        session.simulation().run(maxTime, false, true);
    }

    session.finishRun(nullptr, bundle, meta.networkFile, meta.trainFile);
    return 0;
}

bool HotReloadModeHandler::matches(const CLI& cli) const
{
    return cli.hasRender() && cli.hasHotReload();
}

int HotReloadModeHandler::run(const std::string& netFile,
                              const std::string& trainFile,
                              RunSession& session)
{
    return runPreparedSimulation(
        netFile,
        trainFile,
        session,
        false,
        [&](SimulationBundle& bundle, CommandManager* cmdMgr) -> int
        {
            SFMLRenderer     renderer;
            HotReloadSupport support(session.output());
            const int        hotReloadSeed = static_cast<int>(session.simulation().getSeed());

            std::function<bool(const std::string&, const std::string&)> rebuildCallback =
                [&](const std::string& net, const std::string& train) -> bool
                {
                    session.teardownSimulation(bundle);
                    if (!session.buildSimulation(net, train, bundle, hotReloadSeed))
                    {
                        return false;
                    }

                    if (cmdMgr)
                    {
                        session.simulation().setCommandManager(cmdMgr);
                    }

                    renderer.shutdown();
                    renderer.initialize(session.simulation());
                    session.simulation().start();
                    return true;
                };

            FileWatcher watcher(
                {netFile, trainFile},
                [&](const std::string& changedFile)
                {
                    session.output().writeProgress("Hot-reload: change detected in " + changedFile);

                    const std::string oldNetContent = HotReloadSupport::readFile(netFile);
                    const std::string oldTrainContent = HotReloadSupport::readFile(trainFile);

                    if (!support.validateFilesForReload(netFile, trainFile))
                    {
                        return;
                    }

                    session.output().writeProgress("Hot-reload: files valid, restarting simulation...");

                    const double reloadTime = session.simulation().getCurrentTime();
                    if (!rebuildCallback(netFile, trainFile))
                    {
                        session.output().writeError("Hot-reload: failed to rebuild simulation.");
                        return;
                    }

                    support.recordReloadCommand(cmdMgr,
                                                reloadTime,
                                                oldNetContent,
                                                oldTrainContent,
                                                netFile,
                                                trainFile,
                                                rebuildCallback);

                    session.output().writeProgress("Hot-reload: simulation restarted successfully.");
                });

            watcher.start();
            session.simulation().run(1e9, true, false, &renderer, [&watcher]() { watcher.poll(); });
            watcher.stop();
            return 0;
        });
}

bool RenderModeHandler::matches(const CLI& cli) const
{
    return cli.hasRender();
}

int RenderModeHandler::run(const std::string& netFile,
                           const std::string& trainFile,
                           RunSession& session)
{
    return runPreparedSimulation(
        netFile,
        trainFile,
        session,
        false,
        [&](SimulationBundle& /*bundle*/, CommandManager* /*cmdMgr*/) -> int
        {
            SFMLRenderer renderer;
            session.simulation().run(1e9, true, false, &renderer);
            return 0;
        });
}

bool ConsoleModeHandler::matches(const CLI& /*cli*/) const
{
    return true;
}

int ConsoleModeHandler::run(const std::string& netFile,
                            const std::string& trainFile,
                            RunSession& session)
{
    return runPreparedSimulation(
        netFile,
        trainFile,
        session,
        true,
        [&](SimulationBundle& /*bundle*/, CommandManager* /*cmdMgr*/) -> int
        {
            const double maxTime = session.cli().hasRoundTrip() ? 172800.0 : 106400.0;
            session.simulation().run(maxTime);
            return 0;
        });
}
