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

TEST(HotReloadSupportTest, ReadFileReturnsEmptyForMissingFile)
{
    const std::string content = HotReloadSupport::readFile("/tmp/nonexistent_hot_reload_support_file.txt");
    EXPECT_TRUE(content.empty());
}

TEST(HotReloadSupportTest, ReadFileReturnsExactFileContent)
{
    const std::string path = "/tmp/hot_reload_support_read_ok.txt";
    writeTempFile(path, "abc\n123\n");

    const std::string content = HotReloadSupport::readFile(path);

    EXPECT_EQ(content, "abc\n123\n");
    std::remove(path.c_str());
}

TEST(HotReloadSupportTest, RecordReloadCommandStoresCommandWhenRecording)
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
                                42.0,
                                "old_net",
                                "old_train",
                                "network.txt",
                                "trains.txt",
                                rebuildFn);

    EXPECT_EQ(manager.commandCount(), 1u);
}

TEST(HotReloadSupportTest, ValidateFilesForReloadAcceptsValidTempFiles)
{
    ConsoleOutputWriter output;
    HotReloadSupport    support(output);

    const std::string netPath = "/tmp/hot_reload_network_ok.txt";
    const std::string trainPath = "/tmp/hot_reload_train_ok.txt";

    writeTempFile(netPath,
                  "Node CityA\n"
                  "Node CityB\n"
                  "Rail CityA CityB 10 100\n");
    writeTempFile(trainPath,
                  "TrainAB 80 0.005 356 500 CityA CityB 14h10 00h05\n");

    const bool ok = support.validateFilesForReload(netPath, trainPath);

    EXPECT_TRUE(ok);

    std::remove(netPath.c_str());
    std::remove(trainPath.c_str());
}

TEST(HotReloadSupportTest, ValidateFilesForReloadRejectsEmptyTrainFile)
{
    ConsoleOutputWriter output;
    HotReloadSupport    support(output);

    const std::string netPath = "/tmp/hot_reload_network_valid.txt";
    const std::string trainPath = "/tmp/hot_reload_train_empty.txt";

    writeTempFile(netPath,
                  "Node CityA\n"
                  "Node CityB\n"
                  "Rail CityA CityB 10 100\n");
    writeTempFile(trainPath, "\n");

    const bool ok = support.validateFilesForReload(netPath, trainPath);

    EXPECT_FALSE(ok);

    std::remove(netPath.c_str());
    std::remove(trainPath.c_str());
}
