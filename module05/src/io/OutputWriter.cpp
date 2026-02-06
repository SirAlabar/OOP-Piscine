#include "io/OutputWriter.hpp"
#include "core/Node.hpp"
#include "simulation/PhysicsSystem.hpp"
#include "patterns/states/ITrainState.hpp"
#include <iomanip>
#include <sstream>
#include <cmath>

OutputWriter::OutputWriter(Train* train) : _train(train), _totalPathDistance(0.0), _finalSnapshotWritten(false)
{
	_filename = generateFilename();
	_totalPathDistance = calculateTotalPathDistance();
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

void OutputWriter::writePathInfo()
{
	const auto& path = _train->getPath();
	
	_file << "PATH:" << std::endl;
	
	for (size_t i = 0; i < path.size(); ++i)
	{
		const PathSegment& segment = path[i];
		if (!segment.rail)
		{
			continue;
		}
		
		_file << "  Segment " << i << ": "
		      << segment.from->getName() << " <-> " << segment.to->getName()
		      << " | length=" << std::fixed << std::setprecision(2) << segment.rail->getLength() << "km"
		      << " | speed=" << static_cast<int>(segment.rail->getSpeedLimit()) << "km/h"
		      << std::endl;
	}
	
	_file << "  Total distance: " << std::fixed << std::setprecision(2) 
	      << _totalPathDistance << "km" << std::endl;
	_file << std::endl;
}

void OutputWriter::writeSnapshot(double currentTimeSeconds)
{
	Rail* currentRail = _train->getCurrentRail();
	
	// Handle finished train - write final snapshot at destination (ONCE ONLY)
	if (!currentRail)
	{
		// Skip if we already wrote the final snapshot
		if (_finalSnapshotWritten)
		{
			return;
		}
		
		std::string timeStr = formatTime(currentTimeSeconds);
		std::string arrivalStation = _train->getArrivalStation();
		
		// Format final snapshot: train at destination with 0.00km remaining
		std::ostringstream nodeStr, statusStr;
		nodeStr << std::setw(10) << std::left << arrivalStation;
		statusStr << std::setw(9) << std::left << "Stopped";
		
		_file << "[" << timeStr << "] - "
		      << "[" << nodeStr.str() << "]"
		      << "[" << std::setw(10) << std::left << "          " << "] - "
		      << "[" << std::fixed << std::setprecision(2) << 0.00 << "km] - "
		      << "[" << statusStr.str() << "] - "
		      << "[ ]" << std::endl;
		
		_finalSnapshotWritten = true;
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

double OutputWriter::calculateTotalPathDistance() const
{
	const auto& path = _train->getPath();
	double totalKm = 0.0;
	
	for (const auto& segment : path)
	{
		if (segment.rail)
		{
			totalKm += segment.rail->getLength();
		}
	}
	
	return totalKm;
}

double OutputWriter::calculateRemainingDistance() const
{
    Rail* currentRail = _train->getCurrentRail();
    if (!currentRail)
	{
        return 0.0;
	}

    const auto& path = _train->getPath();
    size_t currentIndex = _train->getCurrentRailIndex();
    
    // Calculate remaining distance on current rail
    double currentRailLengthM = PhysicsSystem::kmToM(currentRail->getLength());
    double remainingOnCurrentRail = currentRailLengthM - _train->getPosition();
    
    // Add all subsequent rails
    double totalRemaining = remainingOnCurrentRail;
    for (size_t i = currentIndex + 1; i < path.size(); ++i)
    {
        if (path[i].rail)
        {
            totalRemaining += PhysicsSystem::kmToM(path[i].rail->getLength());
        }
    }
    
    return PhysicsSystem::mToKm(totalRemaining);
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

	// Check for other trains on the same rail
	const auto& trainsOnRail = currentRail->getTrainsOnRail();
	std::string vizStr = oss.str();
	
	for (Train* otherTrain : trainsOnRail)
	{
		if (otherTrain && otherTrain != _train)
		{
			// Calculate other train position
			double otherProgressRatio = otherTrain->getPosition() / railLengthM;
			int otherCell = static_cast<int>(otherProgressRatio * cellCount);

			if (otherCell >= 0 && otherCell < cellCount && otherCell != trainCell)
			{
				// Insert [O] for other train
				int insertPos = otherCell * 3; // Each cell is 3 chars: "[ ]"

				if (insertPos < static_cast<int>(vizStr.length()))
				{
					vizStr[insertPos + 1] = 'O';
				}
			}
		}
	}

	return vizStr;
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