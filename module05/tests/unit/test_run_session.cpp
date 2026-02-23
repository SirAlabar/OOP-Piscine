#include <gtest/gtest.h>

#include "app/RunSession.hpp"
#include "io/CLI.hpp"
#include "io/ConsoleOutputWriter.hpp"
#include "simulation/systems/CollisionAvoidance.hpp"
#include "simulation/core/SimulationBuilder.hpp"
#include "simulation/core/SimulationManager.hpp"
#include "patterns/behavioral/command/CommandManager.hpp"

#include <memory>
#include <string>
#include <vector>

namespace
{
struct CliHarness
{
    std::vector<std::string> storage;
    std::vector<char*>       argv;

    explicit CliHarness(const std::vector<std::string>& args)
        : storage(args)
    {
        for (std::size_t i = 0; i < storage.size(); ++i)
        {
            argv.push_back(&storage[i][0]);
        }
    }

    int argc() const
    {
        return static_cast<int>(argv.size());
    }

    char** data()
    {
        return argv.data();
    }
};

struct SessionFixture
{
    explicit SessionFixture(const std::vector<std::string>& args)
        : cliHarness(args),
          cli(cliHarness.argc(), cliHarness.data()),
          writer(),
          collision(),
          sim(&collision),
          builder(&writer, cli.getPathfinding()),
          session(cli, writer, sim, builder)
    {
    }

    CliHarness          cliHarness;
    CLI                 cli;
    ConsoleOutputWriter writer;
    CollisionAvoidance  collision;
    SimulationManager   sim;
    SimulationBuilder   builder;
    RunSession          session;
};
} // namespace

TEST(RunSessionTest, SetupCommandManagerReturnsNullWhenRecordFlagAbsent)
{
    SessionFixture fixture({"railway_sim", "network.txt", "trains.txt"});

    std::unique_ptr<CommandManager> manager = fixture.session.setupCommandManager();

    EXPECT_EQ(manager.get(), nullptr);
}

TEST(RunSessionTest, SetupCommandManagerReturnsRecordingManagerWhenRecordFlagPresent)
{
    SessionFixture fixture({"railway_sim", "network.txt", "trains.txt", "--record"});

    std::unique_ptr<CommandManager> manager = fixture.session.setupCommandManager();

    ASSERT_NE(manager.get(), nullptr);
    EXPECT_TRUE(manager->isRecording());
}

TEST(RunSessionTest, ShouldEnableRoundTripReturnsFalseWithoutRoundTripRenderOrReplay)
{
    SessionFixture fixture({"railway_sim", "network.txt", "trains.txt"});

    EXPECT_FALSE(fixture.session.shouldEnableRoundTrip());
}

TEST(RunSessionTest, ShouldEnableRoundTripReturnsTrueWithRoundTripFlag)
{
    SessionFixture fixture({"railway_sim", "network.txt", "trains.txt", "--round-trip"});

    EXPECT_TRUE(fixture.session.shouldEnableRoundTrip());
}

TEST(RunSessionTest, ShouldEnableRoundTripReturnsTrueWithRenderFlag)
{
    SessionFixture fixture({"railway_sim", "network.txt", "trains.txt", "--render"});

    EXPECT_TRUE(fixture.session.shouldEnableRoundTrip());
}

TEST(RunSessionTest, ShouldEnableRoundTripReturnsTrueWithReplayFlag)
{
    SessionFixture fixture({"railway_sim", "network.txt", "trains.txt", "--replay=output/replay.json"});

    EXPECT_TRUE(fixture.session.shouldEnableRoundTrip());
}
