#include "Application.hpp"
#include "io/FileParser.hpp"
#include "io/RailNetworkParser.hpp"
#include "io/TrainConfigParser.hpp"
#include "io/FileOutputWriter.hpp"
#include "io/IOutputWriter.hpp"
#include "io/ConsoleOutputWriter.hpp"
#include "patterns/factories/TrainFactory.hpp"
#include "patterns/factories/TrainValidator.hpp"
#include "patterns/strategies/IPathfindingStrategy.hpp"
#include "patterns/strategies/DijkstraStrategy.hpp"
#include "patterns/strategies/AStarStrategy.hpp"
#include "simulation/SimulationManager.hpp"
#include "analysis/MonteCarloRunner.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "utils/FileWatcher.hpp"
#include "utils/FileSystemUtils.hpp"
#include "patterns/commands/CommandManager.hpp"
#include "patterns/commands/ReloadCommand.hpp"
#include "rendering/SFMLRenderer.hpp"
#include <ctime>
#include <fstream>
#include <iterator>
#include <memory>

// =============================================================================
// Construction / destruction
// =============================================================================

Application::Application(int argc, char* argv[])
    : _cli(argc, argv),
      _consoleWriter(new ConsoleOutputWriter())
{
}

Application::~Application()
{
    delete _consoleWriter;
}

// =============================================================================
// Phase 1 helpers
// =============================================================================

void Application::_printConfiguration(
    const std::string& netFile,
    const std::string& trainFile) const
{
    _consoleWriter->writeStartupHeader();
    _consoleWriter->writeConfiguration("Network file",     netFile);
    _consoleWriter->writeConfiguration("Train file",       trainFile);
    _consoleWriter->writeConfiguration("Output directory", "output/");
    _consoleWriter->writeConfiguration("Pathfinding",      _cli.getPathfinding());

    if (_cli.hasRender())
    {
        _consoleWriter->writeConfiguration("Rendering", "enabled (SFML)");
    }
    if (_cli.hasHotReload())
    {
        _consoleWriter->writeConfiguration("Hot-reload", "enabled");
    }
    if (_cli.hasRecord())
    {
        _consoleWriter->writeConfiguration("Recording", "enabled -> output/replay.json");
    }
    if (_cli.hasReplay())
    {
        _consoleWriter->writeConfiguration("Replay file", _cli.getReplayFile());
    }
    if (_cli.hasMonteCarloRuns())
    {
        _consoleWriter->writeConfiguration("Monte Carlo", std::to_string(_cli.getMonteCarloRuns()) + " runs");
    }
}

CommandManager* Application::_setupCommandManager() const
{
    if (!_cli.hasRecord())
    {
        return nullptr;
    }

    CommandManager* cmdMgr = new CommandManager();
    cmdMgr->startRecording();
    return cmdMgr;
}

void Application::_flushFinalSnapshots(
    const std::vector<FileOutputWriter*>& writers,
    double                                currentTime) const
{
    _consoleWriter->writeProgress("Writing final snapshots...");
    for (FileOutputWriter* w : writers)
    {
        w->writeSnapshot(currentTime);
    }
}

void Application::_saveRecording(
    CommandManager*    cmdMgr,
    const std::string& netFile,
    const std::string& trainFile,
    unsigned int       seed) const
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

    cmdMgr->saveToFile("output/replay.json", meta);
    _consoleWriter->writeProgress(
        "Recording saved: output/replay.json (" +
        std::to_string(cmdMgr->commandCount()) + " commands)");
}

unsigned int Application::_resolveSeed(int seedOverride) const
{
    unsigned int seed;

    if (seedOverride >= 0)
    {
        seed = static_cast<unsigned int>(seedOverride);
        _consoleWriter->writeConfiguration("Seed", std::to_string(seed) + " (from replay recording)");
    }
    else if (_cli.hasSeed())
    {
        seed = _cli.getSeed();
        _consoleWriter->writeConfiguration("Seed", std::to_string(seed) + " (deterministic)");
    }
    else
    {
        seed = static_cast<unsigned int>(std::time(nullptr));
        _consoleWriter->writeConfiguration(
            "Seed",
            std::to_string(seed) + " (random - use --seed=" + std::to_string(seed) + " to reproduce)");
    }

    return seed;
}

double Application::_estimateJourneyMinutes(const Train* train)
{
    double estMinutes = 0.0;
    for (const PathSegment& seg : train->getPath())
    {
        estMinutes += (seg.rail->getLength() / seg.rail->getSpeedLimit()) * 60.0;
    }
    return estMinutes;
}

std::string Application::_readFile(const std::string& path)
{
    std::ifstream f(path, std::ios::in | std::ios::binary);
    if (!f.is_open())
    {
        return "";
    }
    return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

// =============================================================================
// Phase 2 build sub-steps
// =============================================================================

Graph* Application::_parseNetwork(const std::string& netFile)
{
    _consoleWriter->writeProgress("Parsing network file...");
    RailNetworkParser parser(netFile);
    Graph*            graph = parser.parse();
    _consoleWriter->writeGraphDetails(graph->getNodes(), graph->getRails());
    _consoleWriter->writeNetworkSummary(graph->getNodeCount(), graph->getRailCount());
    return graph;
}

std::vector<TrainConfig> Application::_parseTrains(const std::string& trainFile)
{
    _consoleWriter->writeProgress("Parsing train file...");
    TrainConfigParser        parser(trainFile);
    std::vector<TrainConfig> configs = parser.parse();
    _consoleWriter->writeProgress(std::to_string(configs.size()) + " trains parsed");
    return configs;
}

std::unique_ptr<IPathfindingStrategy> Application::_createStrategy() const
{
    if (_cli.getPathfinding() == "astar")
    {
        _consoleWriter->writeProgress("Using A* pathfinding");
        return std::make_unique<AStarStrategy>();
    }
    _consoleWriter->writeProgress("Using Dijkstra pathfinding");
    return std::make_unique<DijkstraStrategy>();
}

std::vector<Train*> Application::_buildTrains(
    const std::vector<TrainConfig>& configs,
    Graph*                          graph,
    IPathfindingStrategy*           strategy)
{
    _consoleWriter->writeProgress("Creating trains and finding paths...");
    std::vector<Train*> trains;

    for (const auto& config : configs)
    {
        ValidationResult vr = TrainValidator::validate(config, graph);
        if (!vr.valid)
        {
            _consoleWriter->writeError(vr.error);
            continue;
        }

        Train* train = TrainFactory::create(config, graph);
        if (!train)
        {
            _consoleWriter->writeError("Failed to create train: " + config.name);
            continue;
        }

        Node* startNode = graph->getNode(config.departureStation);
        Node* endNode   = graph->getNode(config.arrivalStation);

        auto path = strategy->findPath(graph, startNode, endNode);
        if (path.empty())
        {
            _consoleWriter->writeError(
                "No path found for train " + config.name +
                " from " + config.departureStation + " to " + config.arrivalStation);
            delete train;
            continue;
        }

        train->setPath(path);
        _consoleWriter->writePathDebug(train);
        trains.push_back(train);
        _consoleWriter->writeTrainCreated(
            train->getName(), train->getID(),
            config.departureStation, config.arrivalStation,
            static_cast<int>(path.size()));
    }

    return trains;
}

std::vector<FileOutputWriter*> Application::_createOutputWriters(
    const std::vector<Train*>& trains)
{
    _consoleWriter->writeProgress("Creating output files...");
    std::vector<FileOutputWriter*> writers;

    for (Train* train : trains)
    {
        double estMinutes = _estimateJourneyMinutes(train);

        // unique_ptr guards the allocation until all fallible operations succeed.
        // release() transfers ownership to the writers vector only on full success.
        std::unique_ptr<FileOutputWriter> writer(new FileOutputWriter(train));
        writer->open();
        writer->writeHeader(estMinutes);
        writer->writePathInfo();
        writers.push_back(writer.release());

        _consoleWriter->writeProgress(
            "Created: output/" + train->getName() + "_" +
            train->getDepartureTime().toString() + ".result" +
            " (estimated: " + std::to_string(static_cast<int>(estMinutes)) + " min)");
    }

    return writers;
}

// Render and replay modes require the simulation to loop back to origin so
// the visual/playback lifecycle completes cleanly — it is not purely a
// journey-domain decision. This method makes that policy explicit.
bool Application::_shouldEnableRoundTrip() const
{
    return _cli.hasRoundTrip() || _cli.hasRender() || _cli.hasReplay();
}

void Application::_configureSimulation(SimulationBundle& bundle, int seedOverride)
{
    _consoleWriter->writeProgress("Initializing simulation...");

    SimulationManager& sim = SimulationManager::getInstance();
    sim.reset();
    sim.setSimulationWriter(_consoleWriter);
    sim.setEventSeed(_resolveSeed(seedOverride));
    sim.setNetwork(bundle.graph);

    for (std::size_t i = 0; i < bundle.trains.size(); ++i)
    {
        sim.registerOutputWriter(bundle.trains[i], bundle.writers[i]);
    }

    for (Train* train : bundle.trains)
    {
        sim.addTrain(train);
    }

    if (_shouldEnableRoundTrip())
    {
        sim.setRoundTripMode(true);
        _consoleWriter->writeConfiguration("Round-trip mode", "enabled");
    }

    _consoleWriter->writeSimulationStart();
    for (Train* train : bundle.trains)
    {
        _consoleWriter->writeTrainSchedule(train->getName(), train->getDepartureTime());
    }
}

// =============================================================================
// _buildSimulation — thin orchestrator
// =============================================================================

bool Application::_buildSimulation(
    const std::string& netFile,
    const std::string& trainFile,
    SimulationBundle&  outBundle,
    int                seedOverride)
{
    try
    {
        outBundle.graph = _parseNetwork(netFile);

        std::vector<TrainConfig>              configs  = _parseTrains(trainFile);
        std::unique_ptr<IPathfindingStrategy> strategy = _createStrategy();

        outBundle.trains = _buildTrains(configs, outBundle.graph, strategy.get());

        if (outBundle.trains.empty())
        {
            _consoleWriter->writeError("No valid trains created.");
            delete outBundle.graph;
            outBundle.graph = nullptr;
            return false;
        }

        outBundle.writers = _createOutputWriters(outBundle.trains);
        _configureSimulation(outBundle, seedOverride);
        return true;
    }
    catch (const std::exception& e)
    {
        _consoleWriter->writeError(e.what());
        for (FileOutputWriter* w : outBundle.writers)
        {
            delete w;
        }
        outBundle.writers.clear();
        for (Train* t : outBundle.trains)
        {
            delete t;
        }
        outBundle.trains.clear();
        delete outBundle.graph;
        outBundle.graph = nullptr;
        return false;
    }
}

// =============================================================================
// _teardownSimulation
// =============================================================================

void Application::_teardownSimulation(SimulationBundle& bundle)
{
    SimulationManager::getInstance().reset();

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

// =============================================================================
// Hot-reload helpers
// =============================================================================

bool Application::_validateFilesForReload(
    const std::string& netFile,
    const std::string& trainFile) const
{
    try
    {
        FileParser::validateFile(netFile);
        FileParser::validateFile(trainFile);

        {
            RailNetworkParser testNet(netFile);
            Graph*            testGraph = testNet.parse();
            if (!testGraph)
            {
                _consoleWriter->writeError(
                    "Hot-reload: invalid network file — keeping current simulation.");
                return false;
            }
            delete testGraph;
        }

        {
            TrainConfigParser        testTrain(trainFile);
            std::vector<TrainConfig> testConfigs = testTrain.parse();
            if (testConfigs.empty())
            {
                _consoleWriter->writeError(
                    "Hot-reload: train file produced no valid trains — keeping current simulation.");
                return false;
            }
        }
    }
    catch (const std::exception& e)
    {
        _consoleWriter->writeError(
            std::string("Hot-reload: validation failed (") + e.what() +
            ") — keeping current simulation.");
        return false;
    }

    return true;
}

void Application::_recordReloadCommand(
    CommandManager*    cmdMgr,
    double             reloadTime,
    const std::string& oldNet,
    const std::string& oldTrain,
    const std::string& netFile,
    const std::string& trainFile,
    const std::function<bool(const std::string&, const std::string&)>& rebuildCallback) const
{
    if (!cmdMgr || !cmdMgr->isRecording())
    {
        return;
    }

    cmdMgr->record(new ReloadCommand(
        reloadTime,
        oldNet,   oldTrain,
        netFile,  trainFile,
        rebuildCallback));
}

// =============================================================================
// Execution modes
// =============================================================================

int Application::_runMonteCarlo(
    const std::string& netFile,
    const std::string& trainFile)
{
    try
    {
        FileSystemUtils::ensureOutputDirectoryExists();
        MonteCarloRunner runner(
            netFile, trainFile,
            _cli.getSeed(), _cli.getMonteCarloRuns(), _cli.getPathfinding());
        runner.runAll();
        runner.writeCSV("output/monte_carlo_results.csv");
        return 0;
    }
    catch (const std::exception& e)
    {
        _consoleWriter->writeError(e.what());
        return 1;
    }
}

int Application::_runReplay(
    const std::string& /*netFile*/,
    const std::string& /*trainFile*/)
{
    CommandManager    cmdMgr;
    RecordingMetadata meta;

    if (!cmdMgr.loadFromFile(_cli.getReplayFile(), meta))
    {
        _consoleWriter->writeError("Failed to load replay file: " + _cli.getReplayFile());
        return 1;
    }

    cmdMgr.startReplay();
    _consoleWriter->writeProgress(
        "Loaded " + std::to_string(cmdMgr.commandCount()) +
        " commands from " + _cli.getReplayFile());

    SimulationBundle bundle;
    if (!_buildSimulation(meta.networkFile, meta.trainFile, bundle, static_cast<int>(meta.seed)))
    {
        return 1;
    }

    SimulationManager& sim = SimulationManager::getInstance();
    sim.setCommandManager(&cmdMgr);

    if (_cli.hasRender())
    {
        SFMLRenderer renderer;
        sim.run(1e9, true, true, &renderer);
    }
    else
    {
        sim.run(1e9, false, true);
    }

    _flushFinalSnapshots(bundle.writers, sim.getCurrentTime());
    _teardownSimulation(bundle);
    _consoleWriter->writeSimulationComplete();
    return 0;
}

int Application::_runHotReload(
    const std::string& netFile,
    const std::string& trainFile)
{
    SimulationBundle   bundle;
    SimulationManager& sim    = SimulationManager::getInstance();
    SFMLRenderer       renderer;
    CommandManager*    cmdMgr = _setupCommandManager();

    if (!_buildSimulation(netFile, trainFile, bundle))
    {
        delete cmdMgr;
        return 1;
    }

    if (cmdMgr)
    {
        sim.setCommandManager(cmdMgr);
    }

    const int hotReloadSeed = static_cast<int>(sim.getSeed());

    auto rebuildCallback = [&](const std::string& net, const std::string& train) -> bool
    {
        _teardownSimulation(bundle);
        if (_buildSimulation(net, train, bundle, hotReloadSeed))
        {
            if (cmdMgr)
            {
                sim.setCommandManager(cmdMgr);
            }
            renderer.shutdown();
            renderer.initialize(sim);
            sim.start();
            return true;
        }
        return false;
    };

    FileWatcher watcher(
        {netFile, trainFile},
        [&](const std::string& changedFile)
        {
            _consoleWriter->writeProgress("Hot-reload: change detected in " + changedFile);

            const std::string oldNetContent   = _readFile(netFile);
            const std::string oldTrainContent = _readFile(trainFile);

            if (!_validateFilesForReload(netFile, trainFile))
            {
                return;
            }

            _consoleWriter->writeProgress("Hot-reload: files valid, restarting simulation...");
            const double reloadTime = sim.getCurrentTime();

            if (!rebuildCallback(netFile, trainFile))
            {
                _consoleWriter->writeError("Hot-reload: failed to rebuild simulation.");
                return;
            }

            _recordReloadCommand(cmdMgr, reloadTime, oldNetContent, oldTrainContent, netFile, trainFile, rebuildCallback);
            _consoleWriter->writeProgress("Hot-reload: simulation restarted successfully.");
        }
    );

    watcher.start();
    sim.run(1e9, true, false, &renderer, [&watcher]() { watcher.poll(); });
    watcher.stop();

    _flushFinalSnapshots(bundle.writers, sim.getCurrentTime());
    _saveRecording(cmdMgr, netFile, trainFile, sim.getSeed());
    _teardownSimulation(bundle);
    _consoleWriter->writeSimulationComplete();
    delete cmdMgr;
    return 0;
}

int Application::_runRender(
    const std::string& netFile,
    const std::string& trainFile)
{
    SimulationBundle   bundle;
    SimulationManager& sim    = SimulationManager::getInstance();
    SFMLRenderer       renderer;
    CommandManager*    cmdMgr = _setupCommandManager();

    if (!_buildSimulation(netFile, trainFile, bundle))
    {
        delete cmdMgr;
        return 1;
    }

    if (cmdMgr)
    {
        sim.setCommandManager(cmdMgr);
    }

    sim.run(1e9, true, false, &renderer);

    _flushFinalSnapshots(bundle.writers, sim.getCurrentTime());
    _saveRecording(cmdMgr, netFile, trainFile, sim.getSeed());
    _teardownSimulation(bundle);
    _consoleWriter->writeSimulationComplete();
    delete cmdMgr;
    return 0;
}

int Application::_runConsole(
    const std::string& netFile,
    const std::string& trainFile)
{
    SimulationBundle   bundle;
    SimulationManager& sim    = SimulationManager::getInstance();
    CommandManager*    cmdMgr = _setupCommandManager();

    if (!_buildSimulation(netFile, trainFile, bundle))
    {
        delete cmdMgr;
        return 1;
    }

    if (cmdMgr)
    {
        sim.setCommandManager(cmdMgr);
    }

    const double maxTime = _cli.hasRoundTrip() ? 172800.0 : 106400.0;
    sim.run(maxTime);

    _flushFinalSnapshots(bundle.writers, sim.getCurrentTime());
    _consoleWriter->writeSimulationComplete();

    for (const Train* train : bundle.trains)
    {
        _consoleWriter->writeOutputFileListing(
            "output/" + train->getName() + "_" +
            train->getDepartureTime().toString() + ".result");
    }

    _saveRecording(cmdMgr, netFile, trainFile, sim.getSeed());
    _teardownSimulation(bundle);
    delete cmdMgr;
    return 0;
}

// =============================================================================
// run — dispatcher
// =============================================================================

int Application::run()
{
    if (_cli.shouldShowHelp())
    {
        _cli.printHelp();
        return 0;
    }

    if (!_cli.hasValidArguments())
    {
        _consoleWriter->writeError("Invalid number of arguments");
        _cli.printUsage("railway_sim");
        return 1;
    }

    std::string flagError;
    if (!_cli.validateFlags(flagError))
    {
        _consoleWriter->writeError(flagError);
        _consoleWriter->writeError("Use --help for valid options");
        return 1;
    }

    const std::string netFile   = _cli.getNetworkFile();
    const std::string trainFile = _cli.getTrainFile();

    try
    {
        FileParser::validateFile(netFile);
        FileParser::validateFile(trainFile);
    }
    catch (const std::exception& e)
    {
        _consoleWriter->writeError(e.what());
        return 1;
    }

    _printConfiguration(netFile, trainFile);

    if (_cli.hasMonteCarloRuns())
    {
        return _runMonteCarlo(netFile, trainFile);
    }
    if (_cli.hasReplay())
    {
        return _runReplay(netFile, trainFile);
    }
    if (_cli.hasRender() && _cli.hasHotReload())
    {
        return _runHotReload(netFile, trainFile);
    }
    if (_cli.hasRender())
    {
        return _runRender(netFile, trainFile);
    }
    return _runConsole(netFile, trainFile);
}