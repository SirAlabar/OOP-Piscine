#include "io/OutputWriter.hpp"
#include "core/Node.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "patterns/states/ITrainState.hpp"
#include <iomanip>
#include <sstream>
#include <cmath>

OutputWriter::OutputWriter(Train* train) : _train(train)
{
	_filename = generateFilename();
}

OutputWriter::~OutputWriter()
{
	if (_file.is_open())
	{
		_file.close();
	}
}

std::string OutputWriter::generateFilename() const
{
	return _train->getName() + "_" + 
	       _train->getDepartureTime().toString() + ".result";
}

void OutputWriter::open()
{
	_file.open(_filename);
	if (!_file.is_open())
	{
		throw std::runtime_error("Failed to open output file: " + _filename);
	}
}

void OutputWriter::writeHeader(double estimatedTimeMinutes)
{
	int hours = static_cast<int>(estimatedTimeMinutes) / 60;
	int minutes = static_cast<int>(estimatedTimeMinutes) % 60;

	_file << "Train : " << _train->getName() << std::endl;
	_file << "Final travel time : " 
	      << std::setfill('0') << std::setw(2) << hours << "h"
	      << std::setfill('0') << std::setw(2) << minutes << "m" << std::endl;
	_file << std::endl;
}

void OutputWriter::writeSnapshot(double currentTimeSeconds)
{
	Rail* currentRail = _train->getCurrentRail();
	if (!currentRail)
	{
		return;
	}

	std::string timeStr = formatTime(currentTimeSeconds);
	Node* nodeA = currentRail->getNodeA();
	Node* nodeB = currentRail->getNodeB();
	double remainingDist = calculateRemainingDistance();
	std::string status = getStatusString();
	std::string visualization = generateRailVisualization();

	// Format node names with padding (10 chars each, left-aligned)
	std::ostringstream nodeAStr, nodeBStr, statusStr;
	nodeAStr << std::setw(10) << std::left << nodeA->getName();
	nodeBStr << std::setw(10) << std::left << nodeB->getName();
	statusStr << std::setw(9) << std::left << status;

	_file << "[" << timeStr << "] - "
	      << "[" << nodeAStr.str() << "]"
	      << "[" << nodeBStr.str() << "] - "
	      << "[" << std::fixed << std::setprecision(2) << remainingDist << "km] - "
	      << "[" << statusStr.str() << "] - "
	      << visualization << std::endl;
}

void OutputWriter::close()
{
	if (_file.is_open())
	{
		_file.close();
	}
}

std::string OutputWriter::getStatusString() const
{
	std::string stateName = _train->getCurrentState()->getName();

	if (stateName == "Idle")
	{
		return "Idle";
	}
	else if (stateName == "Accelerating")
	{
		return "Accelerating";
	}
	else if (stateName == "Cruising")
	{
		return "Cruising";
	}
	else if (stateName == "Braking")
	{
		return "Braking";
	}
	else if (stateName == "Stopped")
	{
		return "Stopped";
	}
	else if (stateName == "Waiting")
	{
		return "Waiting";
	}
	else if (stateName == "Emergency")
	{
		return "Emergency";
	}

	return "Unknown";
}

double OutputWriter::calculateRemainingDistance() const
{
	double totalRemaining = 0.0;
	const auto& path = _train->getPath();
	size_t currentRailIndex = _train->getCurrentRailIndex();

	// Distance remaining on current rail
	Rail* currentRail = _train->getCurrentRail();
	if (currentRail)
	{
		double currentRailLengthM = PhysicsSystem::kmToM(currentRail->getLength());
		double remainingOnCurrentRail = currentRailLengthM - _train->getPosition();
		totalRemaining += PhysicsSystem::mToKm(remainingOnCurrentRail);
	}

	// Add length of all remaining rails
	for (size_t i = currentRailIndex + 1; i < path.size(); ++i)
	{
		totalRemaining += path[i]->getLength();
	}

	return totalRemaining;
}

std::string OutputWriter::generateRailVisualization() const
{
	Rail* currentRail = _train->getCurrentRail();
	if (!currentRail)
	{
		return "[ ]";
	}

	// One cell per kilometer
	int cellCount = static_cast<int>(std::ceil(currentRail->getLength()));
	if (cellCount < 1)
	{
		cellCount = 1;
	}

	// Calculate train position cell
	double railLengthM = PhysicsSystem::kmToM(currentRail->getLength());
	double progressRatio = _train->getPosition() / railLengthM;
	int trainCell = static_cast<int>(progressRatio * cellCount);

	// Clamp to valid range
	if (trainCell < 0)
	{
		trainCell = 0;
	}
	if (trainCell >= cellCount)
	{
		trainCell = cellCount - 1;
	}

	// Build visualization string
	std::ostringstream oss;
	for (int i = 0; i < cellCount; ++i)
	{
		if (i == trainCell)
		{
			oss << "[x]";
		}
		else
		{
			oss << "[ ]";
		}
	}

	// Check for blocking train
	Train* blockingTrain = currentRail->getOccupiedBy();
	if (blockingTrain && blockingTrain != _train)
	{
		// Calculate blocking train position
		double blockingProgressRatio = blockingTrain->getPosition() / railLengthM;
		int blockingCell = static_cast<int>(blockingProgressRatio * cellCount);

		if (blockingCell >= 0 && blockingCell < cellCount && blockingCell != trainCell)
		{
			// Insert [O] for blocking train
			std::string vizStr = oss.str();
			int insertPos = blockingCell * 3; // Each cell is 3 chars: "[ ]"

			if (insertPos < static_cast<int>(vizStr.length()))
			{
				vizStr[insertPos + 1] = 'O';
				return vizStr;
			}
		}
	}

	return oss.str();
}

std::string OutputWriter::formatTime(double seconds) const
{
	int totalMinutes = static_cast<int>(seconds) / 60;
	int hours = totalMinutes / 60;
	int minutes = totalMinutes % 60;

	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(2) << hours << "h"
	    << std::setfill('0') << std::setw(2) << minutes;

	return oss.str();
}