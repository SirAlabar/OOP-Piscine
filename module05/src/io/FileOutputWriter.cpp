#include "io/FileOutputWriter.hpp"
#include "simulation/state/OccupancyMap.hpp"
#include "core/Node.hpp"
#include "simulation/systems/PhysicsSystem.hpp"
#include "patterns/behavioral/states/ITrainState.hpp"
#include "utils/FileSystemUtils.hpp"
#include "utils/Time.hpp"
#include <iomanip>
#include <sstream>
#include <cmath>

FileOutputWriter::FileOutputWriter(Train* train)
    : _train(train),
      _totalPathDistance(0.0),
      _finalSnapshotWritten(false),
      _occupancy(nullptr)
{
    FileSystemUtils::ensureOutputDirectoryExists();
    _filename          = generateFilename();
    _totalPathDistance = calculateTotalPathDistance();
}

FileOutputWriter::~FileOutputWriter()
{
    if (_file.is_open())
    {
        _file.close();
    }
}

std::string FileOutputWriter::generateFilename() const
{
    return "output/" + _train->getName() + "_" +
           _train->getDepartureTime().toString() + ".result";
}

void FileOutputWriter::open()
{
    _file.open(_filename);
    if (!_file.is_open())
    {
        throw std::runtime_error("Failed to open output file: " + _filename);
    }
}

void FileOutputWriter::writeHeader(double estimatedTimeMinutes)
{
    int hours   = static_cast<int>(estimatedTimeMinutes) / 60;
    int minutes = static_cast<int>(estimatedTimeMinutes) % 60;

    _file << "Train : " << _train->getName() << "\n";
    _file << "Final travel time : "
          << std::setfill('0') << std::setw(2) << hours   << "h"
          << std::setfill('0') << std::setw(2) << minutes << "m" << "\n";
    _file << "\n";
}

void FileOutputWriter::writePathInfo()
{
    const auto& path = _train->getPath();

    _file << "PATH:" << "\n";

    for (std::size_t i = 0; i < path.size(); ++i)
    {
        const PathSegment& segment = path[i];
        if (!segment.rail)
		{
			continue;
		}
        _file << "  Segment " << i << ": "
              << segment.from->getName() << " <-> " << segment.to->getName()
              << " | length=" << std::fixed << std::setprecision(2) << segment.rail->getLength() << "km"
              << " | speed="  << static_cast<int>(segment.rail->getSpeedLimit()) << "km/h"
              << "\n";
    }

    _file << "  Total distance: " << std::fixed << std::setprecision(2)
          << _totalPathDistance << "km" << "\n";
    _file << "\n";
}

void FileOutputWriter::writeSnapshot(double currentTimeSeconds)
{
    Rail* currentRail = _train->getCurrentRail();

    if (!currentRail)
    {
        if (_finalSnapshotWritten)
		{
			return;
		}

        std::string timeStr   = formatTime(currentTimeSeconds);
        std::string realState = getStatusString();
        double      velocityKmh = PhysicsSystem::msToKmh(_train->getVelocity());

        std::ostringstream nodeStr, statusStr;
        nodeStr   << std::setw(10) << std::left << _train->getArrivalStation();
        statusStr << std::setw(9)  << std::left << realState;

        _file << "[" << timeStr << "] - "
              << "[" << nodeStr.str() << "]"
              << "[" << std::setw(10) << std::left << "          " << "] - "
              << "[" << std::fixed << std::setprecision(2) << 0.00 << "km] - "
              << "[" << statusStr.str() << "] - "
              << "[" << std::setw(6) << std::right << std::fixed
                     << std::setprecision(0) << velocityKmh << "km/h] - "
              << "[ ]" << "\n";

        _finalSnapshotWritten = true;
        return;
    }

    std::string timeStr       = formatTime(currentTimeSeconds);
    Node*       nodeA         = currentRail->getNodeA();
    Node*       nodeB         = currentRail->getNodeB();
    double      remainingDist = calculateRemainingDistance();
    std::string status        = getStatusString();
    std::string visualization = generateRailVisualization();
    double      velocityKmh   = PhysicsSystem::msToKmh(_train->getVelocity());

    std::ostringstream nodeAStr, nodeBStr, statusStr;
    nodeAStr  << std::setw(10) << std::left << nodeA->getName();
    nodeBStr  << std::setw(10) << std::left << nodeB->getName();
    statusStr << std::setw(9)  << std::left << status;

    _file << "[" << timeStr << "] - "
          << "[" << nodeAStr.str() << "]"
          << "[" << nodeBStr.str() << "] - "
          << "[" << std::fixed << std::setprecision(2) << remainingDist << "km] - "
          << "[" << statusStr.str() << "] - "
          << "[" << std::setw(6) << std::right << std::fixed
                 << std::setprecision(0) << velocityKmh << "km/h] - "
          << visualization << "\n";
}

void FileOutputWriter::writeEventNotification(double             currentTimeSeconds,
                                              const std::string& eventType,
                                              const std::string& eventDetails,
                                              const std::string& action)
{
    std::string timeStr = formatTime(currentTimeSeconds);

    _file << "\n";
    _file << "*** EVENT " << action << " ***" << "\n";
    _file << "[" << timeStr << "] - " << eventType << ": " << eventDetails << "\n";
    _file << "\n";
}

void FileOutputWriter::close()
{
    if (_file.is_open())
    {
        _file.close();
    }
}

void FileOutputWriter::setOccupancyMap(const OccupancyMap* occupancy)
{
    _occupancy = occupancy;
}

std::string FileOutputWriter::getStatusString() const
{
    if (_train->getCurrentState())
    {
        return _train->getCurrentState()->getName();
    }
    return "Unknown";
}

double FileOutputWriter::calculateTotalPathDistance() const
{
    const auto& path   = _train->getPath();
    double      totalKm = 0.0;

    for (const auto& segment : path)
    {
        if (segment.rail)
		{
			totalKm += segment.rail->getLength();
		}
    }

    return totalKm;
}

double FileOutputWriter::calculateRemainingDistance() const
{
    Rail* currentRail = _train->getCurrentRail();
    if (!currentRail)
	{
		return 0.0;
	}

    const auto& path         = _train->getPath();
    std::size_t currentIndex = _train->getCurrentRailIndex();

    double remainingOnCurrentRail =
        PhysicsSystem::kmToM(currentRail->getLength()) - _train->getPosition();

    double totalRemaining = remainingOnCurrentRail;
    for (std::size_t i = currentIndex + 1; i < path.size(); ++i)
    {
        if (path[i].rail)
        {
            totalRemaining += PhysicsSystem::kmToM(path[i].rail->getLength());
        }
    }

    return PhysicsSystem::mToKm(totalRemaining);
}

std::string FileOutputWriter::generateRailVisualization() const
{
    Rail* currentRail = _train->getCurrentRail();
    if (!currentRail)
	{
		return "[ ]";
	}

    int cellCount = static_cast<int>(std::ceil(currentRail->getLength()));
    if (cellCount < 1)
	{
		cellCount = 1;
	}

    double railLengthM   = PhysicsSystem::kmToM(currentRail->getLength());
    double progressRatio = _train->getPosition() / railLengthM;
    int    trainCell     = static_cast<int>(progressRatio * cellCount);

    if (trainCell < 0)
	{
		trainCell = 0;
	}
    if (trainCell >= cellCount)
	{
		trainCell = cellCount - 1;
	}

    std::ostringstream oss;
    for (int i = 0; i < cellCount; ++i)
    {
        oss << (i == trainCell ? "[x]" : "[ ]");
    }

    std::string vizStr = oss.str();

    const std::vector<Train*>& others =
        _occupancy ? _occupancy->get(currentRail) : std::vector<Train*>{};

    for (Train* otherTrain : others)
    {
        if (!otherTrain || otherTrain == _train)
		{
			continue;
		}
        double otherProgress = otherTrain->getPosition() / railLengthM;
        int    otherCell     = static_cast<int>(otherProgress * cellCount);

        if (otherCell >= 0 && otherCell < cellCount && otherCell != trainCell)
        {
            int insertPos = otherCell * 3;
            if (insertPos < static_cast<int>(vizStr.length()))
            {
                vizStr[insertPos + 1] = 'O';
            }
        }
    }

    return vizStr;
}

std::string FileOutputWriter::formatTime(double seconds) const
{
    return Time::fromSeconds(seconds).toString();
}