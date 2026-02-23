#include "Application.hpp"
#include "io/FileParser.hpp"
#include "io/RailNetworkParser.hpp"
#include "io/TrainConfigParser.hpp"
#include "io/IOutputWriter.hpp"
#include "io/ConsoleOutputWriter.hpp"
#include "io/FileOutputWriter.hpp"
#include "io/ISimulationOutput.hpp"
#include "simulation/SimulationConfig.hpp"
#include "analysis/MonteCarloRunner.hpp"
#include "core/Train.hpp"
#include "core/Graph.hpp"
#include "utils/FileWatcher.hpp"
#include "utils/FileSystemUtils.hpp"
#include "patterns/commands/CommandManager.hpp"
#include "patterns/commands/ReloadCommand.hpp"
#include "patterns/strategies/DijkstraStrategy.hpp"
#include "rendering/SFMLRenderer.hpp"
#include <ctime>
#include <fstream>
#include <iterator>
#include <memory>

Application::Application(int argc, char* argv[])
	: _cli(argc, argv),
	  _consoleWriter(new ConsoleOutputWriter()),
	  _collision(),
	  _sim(&_collision),
	  _builder(_consoleWriter, _cli.getPathfinding())
{
}

Application::~Application()
{
	delete _consoleWriter;
}

// ---------------------------------------------------------------------------
// Configuration printing
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------------

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

std::string Application::_readFile(const std::string& path)
{
	std::ifstream f(path, std::ios::in | std::ios::binary);
	if (!f.is_open())
	{
		return "";
	}
	return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

// ---------------------------------------------------------------------------
// Simulation lifecycle
// ---------------------------------------------------------------------------

void Application::_configureSimulation(SimulationBundle& bundle, int seedOverride)
{
	_consoleWriter->writeProgress("Initializing simulation...");

	_sim.reset();

	SimulationConfig config;
	config.network   = bundle.graph;
	config.seed      = _resolveSeed(seedOverride);
	config.roundTrip = _shouldEnableRoundTrip();
	config.writer    = _consoleWriter;

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
		_consoleWriter->writeConfiguration("Round-trip mode", "enabled");
	}

	_consoleWriter->writeSimulationStart();
	for (Train* train : bundle.trains)
	{
		_consoleWriter->writeTrainSchedule(train->getName(), train->getDepartureTime());
	}
}

bool Application::_buildSimulation(
	const std::string& netFile,
	const std::string& trainFile,
	SimulationBundle&  outBundle,
	int                seedOverride)
{
	try
	{
		outBundle = _builder.build(netFile, trainFile);

		if (outBundle.trains.empty())
		{
			_consoleWriter->writeError("No valid trains created.");
			delete outBundle.graph;
			outBundle.graph = nullptr;
			return false;
		}

		_configureSimulation(outBundle, seedOverride);
		return true;
	}
	catch (const std::exception& e)
	{
		_consoleWriter->writeError(e.what());
		for (FileOutputWriter* w : outBundle.writers) { delete w; }
		outBundle.writers.clear();
		for (Train* t : outBundle.trains)             { delete t; }
		outBundle.trains.clear();
		delete outBundle.graph;
		outBundle.graph = nullptr;
		return false;
	}
}

void Application::_teardownSimulation(SimulationBundle& bundle)
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

void Application::_finishRun(
	CommandManager*    cmdMgr,
	SimulationBundle&  bundle,
	const std::string& netFile,
	const std::string& trainFile)
{
	_flushFinalSnapshots(bundle.writers, _sim.getCurrentTime());
	_saveRecording(cmdMgr, netFile, trainFile, _sim.getSeed(), _sim.getCurrentTime());
	_teardownSimulation(bundle);
	_consoleWriter->writeSimulationComplete();
}

bool Application::_shouldEnableRoundTrip() const
{
	return _cli.hasRoundTrip() || _cli.hasRender() || _cli.hasReplay();
}

// ---------------------------------------------------------------------------
// Hot-reload helpers
// ---------------------------------------------------------------------------

bool Application::_validateFilesForReload(
	const std::string& netFile,
	const std::string& trainFile) const
{
	try
	{
		FileParser::validateFile(netFile);
		FileParser::validateFile(trainFile);

		RailNetworkParser testNet(netFile);
		Graph*            testGraph = testNet.parse();

		if (!testGraph)
		{
			_consoleWriter->writeError(
				"Hot-reload: invalid network file — keeping current simulation.");
			return false;
		}

		TrainConfigParser        testTrain(trainFile);
		std::vector<TrainConfig> testConfigs = testTrain.parse();

		if (testConfigs.empty())
		{
			delete testGraph;
			_consoleWriter->writeError(
				"Hot-reload: train file is empty — keeping current simulation.");
			return false;
		}

		DijkstraStrategy dryStrategy;
		std::vector<TrainValidationResult> results =
			SimulationBuilder::validateTrainConfigs(testConfigs, testGraph, &dryStrategy);

		delete testGraph;

		bool anyRoutable = false;
		for (const TrainValidationResult& r : results)
		{
			if (r.status == TrainValidationResult::Status::InvalidConfig)
			{
				_consoleWriter->writeError(
					"Hot-reload: train '" + r.config.name + "' skipped: " + r.error);
			}
			else if (r.status == TrainValidationResult::Status::NoPath)
			{
				_consoleWriter->writeError(
					"Hot-reload: no path for train '" + r.config.name +
					"' from " + r.config.departureStation +
					" to "    + r.config.arrivalStation + " — skipped.");
			}
			else
			{
				anyRoutable = true;
			}
		}

		if (!anyRoutable)
		{
			_consoleWriter->writeError(
				"Hot-reload: no train can reach its destination in the new network"
				" — keeping current simulation.");
			return false;
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
		oldNet,  oldTrain,
		netFile, trainFile,
		rebuildCallback));
}

// ---------------------------------------------------------------------------
// Run modes
// ---------------------------------------------------------------------------

int Application::_runMonteCarlo(
	const std::string& netFile,
	const std::string& trainFile)
{
	try
	{
		FileSystemUtils::ensureOutputDirectoryExists();
		MonteCarloRunner runner(
			netFile, trainFile,
			_cli.getSeed(), _cli.getMonteCarloRuns(), _cli.getPathfinding(),
			_consoleWriter);
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

	_sim.setCommandManager(&cmdMgr);

	const double maxTime = (meta.stopTime > 0.0) ? meta.stopTime : 1e9;

	if (_cli.hasRender())
	{
		SFMLRenderer renderer;
		_sim.run(maxTime, true, true, &renderer);
	}
	else
	{
		_sim.run(maxTime, false, true);
	}

	// nullptr cmdMgr: _saveRecording is a no-op when no recording active.
	_finishRun(nullptr, bundle, meta.networkFile, meta.trainFile);
	return 0;
}

int Application::_runHotReload(
	const std::string& netFile,
	const std::string& trainFile)
{
	SimulationBundle                bundle;
	SFMLRenderer                    renderer;
	std::unique_ptr<CommandManager> cmdMgr(_setupCommandManager());

	if (!_buildSimulation(netFile, trainFile, bundle))
	{
		return 1;
	}

	if (cmdMgr)
	{
		_sim.setCommandManager(cmdMgr.get());
	}

	const int hotReloadSeed = static_cast<int>(_sim.getSeed());

	auto rebuildCallback = [&](const std::string& net, const std::string& train) -> bool
	{
		_teardownSimulation(bundle);
		if (_buildSimulation(net, train, bundle, hotReloadSeed))
		{
			if (cmdMgr)
			{
				_sim.setCommandManager(cmdMgr.get());
			}
			renderer.shutdown();
			renderer.initialize(_sim);
			_sim.start();
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
			const double reloadTime = _sim.getCurrentTime();

			if (!rebuildCallback(netFile, trainFile))
			{
				_consoleWriter->writeError("Hot-reload: failed to rebuild simulation.");
				return;
			}

			_recordReloadCommand(
				cmdMgr.get(), reloadTime,
				oldNetContent, oldTrainContent,
				netFile, trainFile,
				rebuildCallback);
			_consoleWriter->writeProgress("Hot-reload: simulation restarted successfully.");
		}
	);

	watcher.start();
	_sim.run(1e9, true, false, &renderer, [&watcher]() { watcher.poll(); });
	watcher.stop();

	_flushFinalSnapshots(bundle.writers, _sim.getCurrentTime());
	_saveRecording(cmdMgr.get(), netFile, trainFile, _sim.getSeed(), _sim.getCurrentTime());
	_teardownSimulation(bundle);
	_consoleWriter->writeSimulationComplete();
	return 0;
}

int Application::_runRender(
	const std::string& netFile,
	const std::string& trainFile)
{
	SimulationBundle                bundle;
	SFMLRenderer                    renderer;
	std::unique_ptr<CommandManager> cmdMgr(_setupCommandManager());

	if (!_buildSimulation(netFile, trainFile, bundle))
	{
		return 1;
	}

	if (cmdMgr)
	{
		_sim.setCommandManager(cmdMgr.get());
	}

	_sim.run(1e9, true, false, &renderer);
	_finishRun(cmdMgr.get(), bundle, netFile, trainFile);
	return 0;
}

int Application::_runConsole(
	const std::string& netFile,
	const std::string& trainFile)
{
	SimulationBundle                bundle;
	std::unique_ptr<CommandManager> cmdMgr(_setupCommandManager());

	if (!_buildSimulation(netFile, trainFile, bundle))
	{
		return 1;
	}

	if (cmdMgr)
	{
		_sim.setCommandManager(cmdMgr.get());
	}

	const double maxTime = _cli.hasRoundTrip() ? 172800.0 : 106400.0;
	_sim.run(maxTime);

	for (const Train* train : bundle.trains)
	{
		_consoleWriter->writeOutputFileListing(
			"output/" + train->getName() + "_" +
			train->getDepartureTime().toString() + ".result");
	}

	_finishRun(cmdMgr.get(), bundle, netFile, trainFile);
	return 0;
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

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