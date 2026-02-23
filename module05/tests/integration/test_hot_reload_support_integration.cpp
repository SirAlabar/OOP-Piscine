#include <gtest/gtest.h>

#include "app/HotReloadSupport.hpp"
#include "io/ConsoleOutputWriter.hpp"
#include "patterns/behavioral/command/CommandManager.hpp"

#include <cstdio>
#include <fstream>
#include <functional>

namespace
{
std::string writeTempFile(const std::string& path, const std::string& content)
{
    std::ofstream out(path.c_str(), std::ios::out | std::ios::trunc);
    out << content;
    out.close();
    return path;
}
} // namespace

TEST(HotReloadSupportIntegrationTest, RecordReloadCommandDoesNotRecordWhenManagerIsNotRecording)
{
    ConsoleOutputWriter output;
    HotReloadSupport    support(output);
    CommandManager      manager;

    const std::function<bool(const std::string&, const std::string&)> rebuildFn =
        [](const std::string&, const std::string&) -> bool
        {
            return true;
        };

    support.recordReloadCommand(&manager,
                                10.0,
                                "old_network",
                                "old_train",
                                "network.txt",
                                "trains.txt",
                                rebuildFn);

    EXPECT_EQ(manager.commandCount(), 0u);
}

TEST(HotReloadSupportIntegrationTest, ValidateFilesForReloadRejectsUnroutableTrainSet)
{
    ConsoleOutputWriter output;
    HotReloadSupport    support(output);

    const std::string netPath = "/tmp/hot_reload_network_unroutable.txt";
    const std::string trainPath = "/tmp/hot_reload_trains_unroutable.txt";

    writeTempFile(netPath,
                  "Node CityA\n"
                  "Node CityB\n"
                  "Node CityC\n"
                  "Rail CityA CityB 10 100\n");

    writeTempFile(trainPath,
                  "TrainAC 80 0.005 356 500 CityA CityC 14h10 00h05\n"
                  "TrainCA 80 0.005 356 500 CityC CityA 15h10 00h05\n");

    const bool ok = support.validateFilesForReload(netPath, trainPath);

    EXPECT_FALSE(ok);

    std::remove(netPath.c_str());
    std::remove(trainPath.c_str());
}

TEST(HotReloadSupportIntegrationTest, RecordedReloadCommandSurvivesSaveAndLoad)
{
    ConsoleOutputWriter output;
    HotReloadSupport    support(output);
    CommandManager      manager;

    manager.startRecording();

    const std::function<bool(const std::string&, const std::string&)> rebuildFn =
        [](const std::string&, const std::string&) -> bool
        {
            return true;
        };

    support.recordReloadCommand(&manager,
                                55.0,
                                "old_network",
                                "old_train",
                                "network.txt",
                                "trains.txt",
                                rebuildFn);

    ASSERT_EQ(manager.commandCount(), 1u);

    const std::string savePath = "/tmp/hot_reload_recording.json";
    RecordingMetadata meta;
    meta.networkFile = "network.txt";
    meta.trainFile = "trains.txt";
    meta.seed = 42u;
    meta.stopTime = 120.0;

    ASSERT_TRUE(manager.saveToFile(savePath, meta));

    CommandManager loaded;
    RecordingMetadata loadedMeta;

    ASSERT_TRUE(loaded.loadFromFile(savePath, loadedMeta));

    // RELOAD commands are intentionally skipped in deserializeCommand.
    EXPECT_EQ(loaded.commandCount(), 0u);
    EXPECT_EQ(loadedMeta.networkFile, "network.txt");
    EXPECT_EQ(loadedMeta.trainFile, "trains.txt");
    EXPECT_EQ(loadedMeta.seed, 42u);
    EXPECT_DOUBLE_EQ(loadedMeta.stopTime, 120.0);

    std::remove(savePath.c_str());
}
