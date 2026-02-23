#include "app/RunSession.hpp"
#include "io/CLI.hpp"
#include "io/IOutputWriter.hpp"
#include "io/FileOutputWriter.hpp"
#include "simulation/SimulationConfig.hpp"
#include "patterns/commands/CommandManager.hpp"
#include "utils/FileSystemUtils.hpp"
#include "core/Graph.hpp"
#include "core/Train.hpp"
#include <ctime>

RunSession::RunSession(const CLI& cli,
                       IOutputWriter& consoleWriter,
                       SimulationManager& sim,
                       SimulationBuilder& builder)
    : _cli(cli),
      _consoleWriter(consoleWriter),
      _sim(sim),
      _builder(builder)
{
}

std::unique_ptr<CommandManager> RunSession::setupCommandManager() const
{
    if (!_cli.hasRecord())
    {
        return nullptr;
    }

    std::unique_ptr<CommandManager> cmdMgr(new CommandManager());
    cmdMgr->startRecording();
    return cmdMgr;
}

void RunSession::flushFinalSnapshots(const std::vector<FileOutputWriter*>& writers,
                                     double currentTime) const
{
    _consoleWriter.writeProgress("Writing final snapshots...");
    for (FileOutputWriter* w : writers)
    {
        w->writeSnapshot(currentTime);
    }
}

void RunSession::saveRecording(CommandManager*    cmdMgr,
                               const std::string& netFile,
                               const std::string& trainFile,
                               unsigned int       seed,
                               double             stopTime) const
{
    if (!cmdMgr)
    {
        return;
    }

    FileSystemUtils::ensureOutputDirectoryExists();

    RecordingMetadata meta;
    meta.networkFile = netFile;
    meta.trainFile   = trainFile;
    meta.seed        = seed;
    meta.stopTime    = stopTime;

    cmdMgr->saveToFile("output/replay.json", meta);
    _consoleWriter.writeProgress(
        "Recording saved: output/replay.json (" +
        std::to_string(cmdMgr->commandCount()) + " commands)");
}

unsigned int RunSession::resolveSeed(int seedOverride) const
{
    unsigned int seed;

    if (seedOverride >= 0)
    {
        seed = static_cast<unsigned int>(seedOverride);
        _consoleWriter.writeConfiguration("Seed", std::to_string(seed) + " (from replay recording)");
    }
    else if (_cli.hasSeed())
    {
        seed = _cli.getSeed();
        _consoleWriter.writeConfiguration("Seed", std::to_string(seed) + " (deterministic)");
    }
    else
    {
        seed = static_cast<unsigned int>(std::time(nullptr));
        _consoleWriter.writeConfiguration(
            "Seed",
            std::to_string(seed) + " (random - use --seed=" + std::to_string(seed) + " to reproduce)");
    }

    return seed;
}

bool RunSession::shouldEnableRoundTrip() const
{
    return _cli.hasRoundTrip() || _cli.hasRender() || _cli.hasReplay();
}

void RunSession::configureSimulation(SimulationBundle& bundle, int seedOverride)
{
    _consoleWriter.writeProgress("Initializing simulation...");

    _sim.reset();

    SimulationConfig config;
    config.network   = bundle.graph;
    config.seed      = resolveSeed(seedOverride);
    config.roundTrip = shouldEnableRoundTrip();
    config.writer    = &_consoleWriter;

    _sim.configure(config);

    for (std::size_t i = 0; i < bundle.trains.size(); ++i)
    {
        _sim.registerOutputWriter(bundle.trains[i], bundle.writers[i]);
    }

    for (Train* train : bundle.trains)
    {
        _sim.addTrain(train);
    }

    if (config.roundTrip)
    {
        _consoleWriter.writeConfiguration("Round-trip mode", "enabled");
    }

    _consoleWriter.writeSimulationStart();
    for (Train* train : bundle.trains)
    {
        _consoleWriter.writeTrainSchedule(train->getName(), train->getDepartureTime());
    }
}

bool RunSession::buildSimulation(const std::string& netFile,
                                 const std::string& trainFile,
                                 SimulationBundle&  outBundle,
                                 int                seedOverride)
{
    try
    {
        outBundle = _builder.build(netFile, trainFile);

        if (outBundle.trains.empty())
        {
            _consoleWriter.writeError("No valid trains created.");
            delete outBundle.graph;
            outBundle.graph = nullptr;
            return false;
        }

        configureSimulation(outBundle, seedOverride);
        return true;
    }
    catch (const std::exception& e)
    {
        _consoleWriter.writeError(e.what());
        for (FileOutputWriter* w : outBundle.writers) { delete w; }
        outBundle.writers.clear();
        for (Train* t : outBundle.trains)             { delete t; }
        outBundle.trains.clear();
        delete outBundle.graph;
        outBundle.graph = nullptr;
        return false;
    }
}

void RunSession::teardownSimulation(SimulationBundle& bundle)
{
    _sim.reset();

    for (FileOutputWriter* w : bundle.writers)
    {
        w->close();
        delete w;
    }
    bundle.writers.clear();

    for (Train* t : bundle.trains)
    {
        delete t;
    }
    bundle.trains.clear();

    delete bundle.graph;
    bundle.graph = nullptr;
}

void RunSession::finishRun(CommandManager*    cmdMgr,
                           SimulationBundle&  bundle,
                           const std::string& netFile,
                           const std::string& trainFile)
{
    flushFinalSnapshots(bundle.writers, _sim.getCurrentTime());
    saveRecording(cmdMgr, netFile, trainFile, _sim.getSeed(), _sim.getCurrentTime());
    teardownSimulation(bundle);
    _consoleWriter.writeSimulationComplete();
}

SimulationManager& RunSession::simulation()
{
    return _sim;
}

IOutputWriter& RunSession::output()
{
    return _consoleWriter;
}

const CLI& RunSession::cli() const
{
    return _cli;
}
