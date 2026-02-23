#include "Application.hpp"
#include "app/RunSession.hpp"
#include "app/ModeHandlers.hpp"
#include "io/FileParser.hpp"
#include "io/IOutputWriter.hpp"
#include "io/ConsoleOutputWriter.hpp"
#include <memory>

Application::Application(int argc, char* argv[])
    : _cli(argc, argv),
      _consoleWriter(new ConsoleOutputWriter()),
      _collision(),
      _sim(&_collision),
      _builder(_consoleWriter, _cli.getPathfinding())
{
    registerModeHandlers();
}

Application::~Application()
{
    delete _consoleWriter;
}

void Application::registerModeHandlers()
{
    _modeHandlers.push_back(std::unique_ptr<IRunModeHandler>(new MonteCarloModeHandler()));
    _modeHandlers.push_back(std::unique_ptr<IRunModeHandler>(new ReplayModeHandler()));
    _modeHandlers.push_back(std::unique_ptr<IRunModeHandler>(new HotReloadModeHandler()));
    _modeHandlers.push_back(std::unique_ptr<IRunModeHandler>(new RenderModeHandler()));
    _modeHandlers.push_back(std::unique_ptr<IRunModeHandler>(new ConsoleModeHandler()));
}

void Application::printConfiguration(const std::string& netFile,
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

    printConfiguration(netFile, trainFile);

    RunSession session(_cli, *_consoleWriter, _sim, _builder);

    for (std::size_t i = 0; i < _modeHandlers.size(); ++i)
    {
        if (_modeHandlers[i]->matches(_cli))
        {
            return _modeHandlers[i]->run(netFile, trainFile, session);
        }
    }

    _consoleWriter->writeError("No run mode matched.");
    return 1;
}
