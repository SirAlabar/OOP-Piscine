#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <unistd.h>

#include "io/TrainConfigParser.hpp"

namespace
{
	std::string writeTempFile(const std::string& contents)
	{
		const auto path = std::filesystem::temp_directory_path()
			/ std::filesystem::path("train_config_parser_test_" + std::to_string(::getpid()) + "_" + std::to_string(std::rand()) + ".txt");
		std::ofstream out(path);
		out << contents;
		out.close();
		return path.string();
	}
}

TEST(TrainConfigParserTest, ParsesMultipleValidTrainConfigs)
{
	const std::string filepath = writeTempFile(
		"Express 80 0.005 356 500 CityA CityB 14h10 00h05\n"
		"Regional 65 0.007 300 450 CityB CityC 08h30 00h02\n");

	TrainConfigParser parser(filepath);
	const std::vector<TrainConfig> configs = parser.parse();

	ASSERT_EQ(configs.size(), 2);
	EXPECT_EQ(configs[0].name, "Express");
	EXPECT_EQ(configs[0].departureStation, "CityA");
	EXPECT_EQ(configs[0].arrivalStation, "CityB");
	EXPECT_EQ(configs[0].departureTime.toString(), "14h10");

	EXPECT_EQ(configs[1].name, "Regional");
	EXPECT_EQ(configs[1].stopDuration.toString(), "00h02");

	std::filesystem::remove(filepath);
}

TEST(TrainConfigParserTest, ThrowsOnDuplicateTrainNames)
{
	const std::string filepath = writeTempFile(
		"Express 80 0.005 356 500 CityA CityB 14h10 00h05\n"
		"Express 70 0.005 320 480 CityB CityC 15h10 00h04\n");

	TrainConfigParser parser(filepath);
	EXPECT_THROW(parser.parse(), std::runtime_error);

	std::filesystem::remove(filepath);
}

TEST(TrainConfigParserTest, ThrowsOnInvalidFieldCount)
{
	const std::string filepath = writeTempFile(
		"Express 80 0.005 356 500 CityA CityB 14h10\n");

	TrainConfigParser parser(filepath);
	EXPECT_THROW(parser.parse(), std::runtime_error);

	std::filesystem::remove(filepath);
}

TEST(TrainConfigParserTest, ThrowsOnNegativeMass)
{
	const std::string filepath = writeTempFile(
		"Express -1 0.005 356 500 CityA CityB 14h10 00h05\n");

	TrainConfigParser parser(filepath);
	EXPECT_THROW(parser.parse(), std::runtime_error);

	std::filesystem::remove(filepath);
}

TEST(TrainConfigParserTest, ThrowsOnInvalidTimeFormat)
{
	const std::string filepath = writeTempFile(
		"Express 80 0.005 356 500 CityA CityB 99h99 00h05\n");

	TrainConfigParser parser(filepath);
	EXPECT_THROW(parser.parse(), std::runtime_error);

	std::filesystem::remove(filepath);
}
