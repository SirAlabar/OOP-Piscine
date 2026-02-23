#include "app/HotReloadSupport.hpp"
#include "io/FileParser.hpp"
#include "io/IOutputWriter.hpp"
#include "io/RailNetworkParser.hpp"
#include "io/TrainConfigParser.hpp"
#include "patterns/behavioral/command/CommandManager.hpp"
#include "patterns/behavioral/command/ReloadCommand.hpp"
#include "patterns/behavioral/strategies/DijkstraStrategy.hpp"
#include "simulation/core/SimulationBuilder.hpp"
#include "core/Rail.hpp"
#include "core/Graph.hpp"
#include <fstream>
#include <iterator>

HotReloadSupport::HotReloadSupport(IOutputWriter& output)
    : _output(output)
{
}

bool HotReloadSupport::validateFilesForReload(
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
            _output.writeError("Hot-reload: invalid network file — keeping current simulation.");
            return false;
        }

        TrainConfigParser        testTrain(trainFile);
        std::vector<TrainConfig> testConfigs = testTrain.parse();

        if (testConfigs.empty())
        {
            delete testGraph;
            _output.writeError("Hot-reload: train file is empty — keeping current simulation.");
            return false;
        }

        DijkstraStrategy dryStrategy;
        std::vector<TrainValidationResult> results =
            SimulationBuilder::validateTrainConfigs(testConfigs, testGraph, &dryStrategy);

        delete testGraph;

        bool anyRoutable = false;
        for (const TrainValidationResult& result : results)
        {
            if (result.status == TrainValidationResult::Status::InvalidConfig)
            {
                _output.writeError(
                    "Hot-reload: train '" + result.config.name + "' skipped: " + result.error);
            }
            else if (result.status == TrainValidationResult::Status::NoPath)
            {
                _output.writeError(
                    "Hot-reload: no path for train '" + result.config.name +
                    "' from " + result.config.departureStation +
                    " to " + result.config.arrivalStation + " — skipped.");
            }
            else
            {
                anyRoutable = true;
            }
        }

        if (!anyRoutable)
        {
            _output.writeError("Hot-reload: no train can reach its destination in the new network"
                               " — keeping current simulation.");
            return false;
        }
    }
    catch (const std::exception& e)
    {
        _output.writeError(
            std::string("Hot-reload: validation failed (") + e.what() + ") — keeping current simulation.");
        return false;
    }

    return true;
}

void HotReloadSupport::recordReloadCommand(
    CommandManager* cmdMgr,
    double reloadTime,
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

    cmdMgr->record(new ReloadCommand(reloadTime, oldNet, oldTrain, netFile, trainFile, rebuildCallback));
}

std::string HotReloadSupport::readFile(const std::string& path)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        return "";
    }

    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}
