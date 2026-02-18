#include "Application.hpp"
#include "io/FileParser.hpp"
#include "io/RailNetworkParser.hpp"
#include "io/TrainConfigParser.hpp"
#include "io/FileOutputWriter.hpp"
#include "io/IOutputWriter.hpp"
#include "io/ConsoleOutputWriter.hpp"
#include "patterns/factories/TrainFactory.hpp"
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
#include <mutex>
#include <thread>

Application::Application(int argc, char* argv[])
    : _cli(argc, argv),
      _consoleWriter(new ConsoleOutputWriter())
{
}

Application::~Application()
{
    delete _consoleWriter;
}

std::string Application::_readFile(const std::string& path)
{
    std::ifstream f(path, std::ios::in | std::ios::binary);
    if (!f.is_open()) { return ""; }
    return std::string(std::istreambuf_iterator<char>(f),
                       std::istreambuf_iterator<char>());
}

bool Application::_buildSimulation(
    const std::string&              netFile,
    const std::string&              trainFile,
    Graph*&                         outGraph,
    std::vector<Train*>&            outTrains,
    std::vector<FileOutputWriter*>& outWriters,
    int                             seedOverride)
{
    try
    {
        // Parse network
        _consoleWriter->writeProgress("Parsing network file...");
        RailNetworkParser networkParser(netFile);
        outGraph = networkParser.parse();

        _consoleWriter->writeGraphDetails(outGraph->getNodes(), outGraph->getRails());
        _consoleWriter->writeNetworkSummary(outGraph->getNodeCount(), outGraph->getRailCount());

        // Parse train configurations
        _consoleWriter->writeProgress("Parsing train file...");
        TrainConfigParser        trainParser(trainFile);
        std::vector<TrainConfig> trainConfigs = trainParser.parse();

        _consoleWriter->writeProgress(std::to_string(trainConfigs.size()) + " trains parsed");

        // Select pathfinding strategy
        _consoleWriter->writeProgress("Creating trains and finding paths...");

        DijkstraStrategy      dijkstra;
        AStarStrategy         astar;
        IPathfindingStrategy* strategy = nullptr;

        if (_cli.getPathfinding() == "astar")
        {
            strategy = &astar;
            _consoleWriter->writeProgress("Using A* pathfinding");
        }
        else
        {
            strategy = &dijkstra;
            _consoleWriter->writeProgress("Using Dijkstra pathfinding");
        }

        for (const auto& config : trainConfigs)
        {
            Train* train = TrainFactory::create(config, outGraph);
            if (!train)
            {
                _consoleWriter->writeError("Failed to create train: " + config.name);
                continue;
            }

            Node* startNode = outGraph->getNode(config.departureStation);
            Node* endNode   = outGraph->getNode(config.arrivalStation);

            if (!startNode || !endNode)
            {
                _consoleWriter->writeError("Invalid stations for train " + config.name);
                delete train;
                continue;
            }

            auto path = strategy->findPath(outGraph, startNode, endNode);
            if (path.empty())
            {
                _consoleWriter->writeError(
                    "No path found for train " + config.name +
                    " from " + config.departureStation +
                    " to "   + config.arrivalStation);
                delete train;
                continue;
            }

            train->setPath(path);
            _consoleWriter->writePathDebug(train);
            // train->setState(&idleState);
            outTrains.push_back(train);

            _consoleWriter->writeTrainCreated(
                train->getName(), train->getID(),
                config.departureStation, config.arrivalStation,
                static_cast<int>(path.size()));
        }

        if (outTrains.empty())
        {
            _consoleWriter->writeError("No valid trains created.");
            delete outGraph;
            outGraph = nullptr;
            return false;
        }

        // Create output writers
        _consoleWriter->writeProgress("Creating output files...");

        for (Train* train : outTrains)
        {
            FileOutputWriter* writer = new FileOutputWriter(train);
            writer->open();

            double estMinutes = 0.0;
            for (const PathSegment& seg : train->getPath())
            {
                estMinutes += (seg.rail->getLength() / seg.rail->getSpeedLimit()) * 60.0;
            }

            writer->writeHeader(estMinutes);
            writer->writePathInfo();
            outWriters.push_back(writer);

            _consoleWriter->writeProgress(
                "Created: output/" + train->getName() + "_" +
                train->getDepartureTime().toString() + ".result" +
                " (estimated: " + std::to_string(static_cast<int>(estMinutes)) + " min)");
        }

        // Configure SimulationManager
        _consoleWriter->writeProgress("Initializing simulation...");

        SimulationManager& sim = SimulationManager::getInstance();
        sim.reset();
        sim.setSimulationWriter(_consoleWriter);

        unsigned int seed;
        if (seedOverride >= 0)
        {
            seed = static_cast<unsigned int>(seedOverride);
            sim.setEventSeed(seed);
            _consoleWriter->writeConfiguration("Seed",
                std::to_string(seed) + " (from replay recording)");
        }
        else if (_cli.hasSeed())
        {
            seed = _cli.getSeed();
            sim.setEventSeed(seed);
            _consoleWriter->writeConfiguration("Seed",
                std::to_string(seed) + " (deterministic)");
        }
        else
        {
            seed = static_cast<unsigned int>(std::time(nullptr));
            sim.setEventSeed(seed);
            _consoleWriter->writeConfiguration("Seed",
                std::to_string(seed) +
                " (random - use --seed=" + std::to_string(seed) + " to reproduce)");
        }

        sim.setNetwork(outGraph);

        for (std::size_t i = 0; i < outTrains.size(); ++i)
        {
            sim.registerOutputWriter(outTrains[i], outWriters[i]);
        }

        for (Train* train : outTrains)
        {
            sim.addTrain(train);
        }

        if (_cli.hasRoundTrip() || _cli.hasRender() || _cli.hasReplay())
        {
            sim.setRoundTripMode(true);
            _consoleWriter->writeConfiguration("Round-trip mode", "enabled");
        }

        _consoleWriter->writeSimulationStart();
        for (Train* train : outTrains)
        {
            _consoleWriter->writeTrainSchedule(train->getName(), train->getDepartureTime());
        }

        return true;
    }
    catch (const std::exception& e)
    {
        _consoleWriter->writeError(e.what());

        for (FileOutputWriter* w : outWriters) 
		{
			delete w;
		}
        outWriters.clear();
        for (Train* t : outTrains)
		{
			delete t;
		}
        outTrains.clear();
        delete outGraph;
        outGraph = nullptr;
        return false;
    }
}

void Application::_teardownSimulation(
    Graph*&                         graph,
    std::vector<Train*>&            trains,
    std::vector<FileOutputWriter*>& writers)
{
    SimulationManager::getInstance().reset();

    for (FileOutputWriter* w : writers) 
	{
		w->close(); delete w;
	}
    writers.clear();
    for (Train* t : trains)
	{
		delete t;
	}
    trains.clear();
    delete graph;
    graph = nullptr;
}

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

    std::string networkFile = _cli.getNetworkFile();
    std::string trainFile   = _cli.getTrainFile();

    try
    {
        FileParser::validateFile(networkFile);
        FileParser::validateFile(trainFile);
    }
    catch (const std::exception& e)
    {
        _consoleWriter->writeError(e.what());
        return 1;
    }

    _consoleWriter->writeStartupHeader();
    _consoleWriter->writeConfiguration("Network file",     networkFile);
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
		_consoleWriter->writeConfiguration("Recording", "enabled → output/replay.json");
	}

	if (_cli.hasReplay())
	{
		_consoleWriter->writeConfiguration("Replay file", _cli.getReplayFile());
	}

	if (_cli.hasMonteCarloRuns())
	{
		_consoleWriter->writeConfiguration(
			"Monte Carlo",
			std::to_string(_cli.getMonteCarloRuns()) + " runs"
		);
	}

    // Monte Carlo mode
    if (_cli.hasMonteCarloRuns())
    {
        try
        {
            FileSystemUtils::ensureOutputDirectoryExists();

            MonteCarloRunner runner(
                networkFile, trainFile,
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

    // Replay mode
    if (_cli.hasReplay())
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

        Graph*                         graph = nullptr;
        std::vector<Train*>            trains;
        std::vector<FileOutputWriter*> writers;

        if (!_buildSimulation(meta.networkFile, meta.trainFile, graph, trains, writers,
                              static_cast<int>(meta.seed)))
        {
            return 1;
        }

        SimulationManager& sim = SimulationManager::getInstance();
        sim.setCommandManager(&cmdMgr);

        if (_cli.hasRender())
        {
            SFMLRenderer renderer;
            std::thread  simThread([&sim]() { sim.run(1e9, true, true); });

            renderer.run(sim);

            sim.stop();
            simThread.join();
        }
        else
        {
            sim.run(1e9, false, true);
        }

        _consoleWriter->writeProgress("Writing final snapshots...");
        for (FileOutputWriter* w : writers) { w->writeSnapshot(sim.getCurrentTime()); }

        _teardownSimulation(graph, trains, writers);
        _consoleWriter->writeSimulationComplete();
        return 0;
    }

    // Render + Hot-reload + (optional) Record
    if (_cli.hasRender() && _cli.hasHotReload())
    {
        Graph*                         graph = nullptr;
        std::vector<Train*>            trains;
        std::vector<FileOutputWriter*> writers;
        SimulationManager&             sim   = SimulationManager::getInstance();
        SFMLRenderer                   renderer;

        CommandManager* cmdMgr = nullptr;
        if (_cli.hasRecord())
        {
            cmdMgr = new CommandManager();
            cmdMgr->startRecording();
        }

        if (!_buildSimulation(networkFile, trainFile, graph, trains, writers))
        {
            delete cmdMgr;
            return 1;
        }
        if (cmdMgr)
		{
			sim.setCommandManager(cmdMgr);
		}

        std::thread simThread([&sim]() { sim.run(1e9, true); });
        std::mutex  reloadMutex;

        auto rebuildCallback = [&](const std::string& net, const std::string& train) -> bool
        {
            sim.stop();
            if (simThread.joinable())
			{
				simThread.join();
			}
            _teardownSimulation(graph, trains, writers);

            if (_buildSimulation(net, train, graph, trains, writers))
            {
                if (cmdMgr)
				{
					sim.setCommandManager(cmdMgr);
				}
                simThread = std::thread([&sim](){sim.run(1e9, true);});
                return true;
            }
            return false;
        };

        FileWatcher watcher(
            {networkFile, trainFile},
            [&](const std::string& changedFile)
            {
                std::lock_guard<std::mutex> lock(reloadMutex);

                _consoleWriter->writeProgress("Hot-reload: change detected in " + changedFile);

                std::string oldNetContent   = _readFile(networkFile);
                std::string oldTrainContent = _readFile(trainFile);

                try
                {
                    FileParser::validateFile(networkFile);
                    FileParser::validateFile(trainFile);

                    {
                        RailNetworkParser testNet(networkFile);
                        Graph*            testGraph = testNet.parse();
                        if (!testGraph)
                        {
                            _consoleWriter->writeError(
                                "Hot-reload: invalid network file — keeping current simulation.");
                            return;
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
                            return;
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    _consoleWriter->writeError(
                        std::string("Hot-reload: validation failed (") + e.what() +
                        ") — keeping current simulation.");
                    return;
                }

                _consoleWriter->writeProgress("Hot-reload: files valid, restarting simulation...");

                double reloadTime = sim.getCurrentTime();

                if (!rebuildCallback(networkFile, trainFile))
                {
                    _consoleWriter->writeError("Hot-reload: failed to rebuild simulation.");
                    return;
                }

                if (cmdMgr && cmdMgr->isRecording())
                {
                    cmdMgr->record(new ReloadCommand(
                        reloadTime,
                        oldNetContent, oldTrainContent,
                        networkFile,   trainFile,
                        rebuildCallback));
                }

                _consoleWriter->writeProgress("Hot-reload: simulation restarted successfully.");
            }
        );

        watcher.start();
        renderer.run(sim);
        watcher.stop();

        sim.stop();
        if (simThread.joinable())
		{
			simThread.join();
		}

        _consoleWriter->writeProgress("Writing final snapshots...");
        for (FileOutputWriter* w : writers)
		{
			w->writeSnapshot(sim.getCurrentTime());
		}

        if (cmdMgr)
        {
            FileSystemUtils::ensureOutputDirectoryExists();

            RecordingMetadata meta;
            meta.networkFile = networkFile;
            meta.trainFile   = trainFile;
            meta.seed        = sim.getSeed();

            cmdMgr->saveToFile("output/replay.json", meta);
            _consoleWriter->writeProgress(
                "Recording saved: output/replay.json (" +
                std::to_string(cmdMgr->commandCount()) + " commands)");
        }

        _teardownSimulation(graph, trains, writers);
        _consoleWriter->writeSimulationComplete();
        delete cmdMgr;
        return 0;
    }

    // ── Render mode (no hot-reload) ────────────────────────────────────────
    if (_cli.hasRender())
    {
        Graph*                         graph = nullptr;
        std::vector<Train*>            trains;
        std::vector<FileOutputWriter*> writers;
        SimulationManager&             sim   = SimulationManager::getInstance();
        SFMLRenderer                   renderer;

        CommandManager* cmdMgr = nullptr;
        if (_cli.hasRecord())
        {
            cmdMgr = new CommandManager();
            cmdMgr->startRecording();
        }

        if (!_buildSimulation(networkFile, trainFile, graph, trains, writers))
        {
            delete cmdMgr;
            return 1;
        }

        if (cmdMgr) { sim.setCommandManager(cmdMgr); }

        std::thread simThread([&sim]() { sim.run(1e9, true); });

        renderer.run(sim);

        sim.stop();
        simThread.join();

        _consoleWriter->writeProgress("Writing final snapshots...");
        for (FileOutputWriter* w : writers) { w->writeSnapshot(sim.getCurrentTime()); }

        if (cmdMgr)
        {
            FileSystemUtils::ensureOutputDirectoryExists();

            RecordingMetadata meta;
            meta.networkFile = networkFile;
            meta.trainFile   = trainFile;
            meta.seed        = sim.getSeed();

            cmdMgr->saveToFile("output/replay.json", meta);
            _consoleWriter->writeProgress(
                "Recording saved: output/replay.json (" +
                std::to_string(cmdMgr->commandCount()) + " commands)");
        }

        _teardownSimulation(graph, trains, writers);
        _consoleWriter->writeSimulationComplete();
        delete cmdMgr;
        return 0;
    }

    // ── Console mode ───────────────────────────────────────────────────────
    {
        Graph*                         graph = nullptr;
        std::vector<Train*>            trains;
        std::vector<FileOutputWriter*> writers;
        SimulationManager&             sim   = SimulationManager::getInstance();

        CommandManager* cmdMgr = nullptr;
        if (_cli.hasRecord())
        {
            cmdMgr = new CommandManager();
            cmdMgr->startRecording();
        }

        if (!_buildSimulation(networkFile, trainFile, graph, trains, writers))
        {
            delete cmdMgr;
            return 1;
        }

        if (cmdMgr)
		{
			sim.setCommandManager(cmdMgr);
		}

        double maxTime = _cli.hasRoundTrip() ? 172800.0 : 106400.0;
        sim.run(maxTime);

        _consoleWriter->writeProgress("Writing final snapshots...");
        for (FileOutputWriter* w : writers)
		{
			w->writeSnapshot(sim.getCurrentTime());
		}

        _consoleWriter->writeSimulationComplete();
        for (const Train* train : trains)
        {
            _consoleWriter->writeOutputFileListing(
                "output/" + train->getName() + "_" +
                train->getDepartureTime().toString() + ".result");
        }

        if (cmdMgr)
        {
            FileSystemUtils::ensureOutputDirectoryExists();

            RecordingMetadata meta;
            meta.networkFile = networkFile;
            meta.trainFile   = trainFile;
            meta.seed        = sim.getSeed();

            cmdMgr->saveToFile("output/replay.json", meta);
            _consoleWriter->writeProgress(
                "Recording saved: output/replay.json (" +
                std::to_string(cmdMgr->commandCount()) + " commands)");
        }

        _teardownSimulation(graph, trains, writers);
        delete cmdMgr;
    }

    return 0;
}