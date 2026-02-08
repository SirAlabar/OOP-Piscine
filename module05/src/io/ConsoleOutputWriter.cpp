#include "io/ConsoleOutputWriter.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "core/Train.hpp"
#include <iostream>

void ConsoleOutputWriter::writeStartupHeader()
{
	std::cout << "\n" << Color::BOLD_CYAN;
	std::cout << Box::drawHeader("RAILWAY SIMULATION ENGINE", 80);
	std::cout << Color::RESET;
}

void ConsoleOutputWriter::writeConfiguration(const std::string& key, const std::string& value)
{
	std::cout << Color::CYAN << key << ": " << Color::RESET 
	          << Color::BOLD_GREEN << value << Color::RESET << std::endl;
}

void ConsoleOutputWriter::writeNetworkSummary(int nodeCount, int railCount)
{
	std::cout << Color::GREEN << "  ✓ Nodes: " << Color::RESET << nodeCount << std::endl;
	std::cout << Color::GREEN << "  ✓ Rails: " << Color::RESET << railCount << std::endl;
	std::cout << std::endl;
}

void ConsoleOutputWriter::writeGraphDetails(const std::vector<Node*>& nodes,
                                             const std::vector<Rail*>& rails)
{
	std::cout << Color::DIM << "\n=== GRAPH CONTENT ===" << Color::RESET << std::endl;
	
	for (const Node* n : nodes)
	{
		std::cout << "Node: " << n->getName() << " | type=" << n->getTypeString()
		          << std::endl;
	}
	
	for (const Rail* r : rails)
	{
		std::cout << "Rail: "
		          << r->getNodeA()->getName()
		          << " <-> "
		          << r->getNodeB()->getName()
		          << std::endl;
	}
	
	std::cout << Color::DIM << "=====================\n" << Color::RESET << std::endl;
}

void ConsoleOutputWriter::writeTrainCreated(const std::string& trainName, int trainId,
                                             const std::string& from, const std::string& to,
                                             int segments)
{
	std::cout << Color::GREEN << "  ✓ " << Color::RESET 
	          << Color::BOLD_WHITE << trainName << Color::RESET
	          << Color::DIM << " (ID: " << trainId << ")" << Color::RESET
	          << ": " 
	          << Color::CYAN << from << Color::RESET
	          << " → " 
	          << Color::CYAN << to << Color::RESET
	          << Color::DIM << " (" << segments << " segments)" << Color::RESET
	          << std::endl;
}

void ConsoleOutputWriter::writePathDebug(const Train* train)
{
	if (!train) return;
	
	std::cout << "\n[DEBUG] PATH FOR TRAIN: " << train->getName() << "\n";
	
	const auto& path = train->getPath();
	
	if (path.empty())
	{
		std::cout << "  -> NO PATH FOUND!\n";
		return;
	}
	
	for (size_t i = 0; i < path.size(); ++i)
	{
		const PathSegment& segment = path[i];
		Rail* rail = segment.rail;
		
		std::cout << "  Segment " << i
		          << ": "
		          << rail->getNodeA()->getName()
		          << " <-> "
		          << rail->getNodeB()->getName()
		          << " | length="
		          << rail->getLength()
		          << " | speed="
		          << rail->getSpeedLimit()
		          << "\n";
	}
	
	std::cout << std::endl;
}

void ConsoleOutputWriter::writeSimulationStart()
{
	std::cout << "\n" << Color::BOLD_GREEN;
	std::cout << Box::drawHeader("SIMULATION START", 80);
	std::cout << Color::RESET;
	
	std::cout << Color::BOLD_CYAN << "Train Schedule:" << Color::RESET << std::endl;
}

void ConsoleOutputWriter::writeTrainSchedule(const std::string& trainName, const Time& departureTime)
{
	std::cout << Color::DIM << "  • " << Color::RESET
	          << Color::BOLD_WHITE << trainName << Color::RESET
	          << Color::DIM << " scheduled for " << Color::RESET
	          << Color::YELLOW << departureTime.toString() << Color::RESET
	          << std::endl;
}

void ConsoleOutputWriter::writeEventActivated(const Time& currentTime, 
                                               const std::string& eventType,
                                               const std::string& description)
{
	std::cout << Color::BOLD_YELLOW << "\n[" << currentTime.toString() << "] "
	          << Color::RESET << getEventIcon(eventType)
	          << Color::BOLD_WHITE << " EVENT: " << Color::RESET
	          << Color::CYAN << eventType << Color::RESET << std::endl;
	std::cout << Color::DIM << "        └─ " << description 
	          << Color::RESET << "\n" << std::endl;
}

void ConsoleOutputWriter::writeEventEnded(const Time& currentTime, const std::string& eventType)
{
	std::cout << Color::DIM << "[" << currentTime.toString() << "] "
	          << "✓ EVENT ENDED: " << eventType
	          << Color::RESET << std::endl;
}

void ConsoleOutputWriter::writeDashboard(const Time& currentTime, int activeTrains, 
                                          int totalTrains, int completedTrains, int activeEvents)
{
	std::cout << "\n" << Color::BOLD_CYAN;
	std::cout << "╔════════════════════════ [" << currentTime.toString() 
	          << "] SIMULATION STATUS ════════════════════════╗\n";
	std::cout << Color::RESET;
	
	std::cout << Color::CYAN << "║ " << Color::RESET;
	std::cout << Color::GREEN << "Active: " << activeTrains << "/" << totalTrains << Color::RESET;
	std::cout << Color::DIM << "  |  " << Color::RESET;
	std::cout << Color::BOLD_GREEN << "Completed: " << completedTrains << Color::RESET;
	std::cout << Color::DIM << "  |  " << Color::RESET;
	std::cout << Color::YELLOW << "Events: " << activeEvents << Color::RESET;
	
	std::string padding(48, ' ');
	std::cout << padding << Color::CYAN << "║\n" << Color::RESET;
	
	std::cout << Color::CYAN;
	std::cout << "╚═══════════════════════════════════════════════════════════════════════════════╝\n";
	std::cout << Color::RESET << std::endl;
}

void ConsoleOutputWriter::writeSimulationComplete()
{
	std::cout << "\n" << Color::BOLD_GREEN;
	std::cout << Box::drawHeader("SIMULATION COMPLETE", 80);
	std::cout << Color::RESET;
	
	std::cout << Color::BOLD_CYAN << "Output files generated:" << Color::RESET << std::endl;
}

void ConsoleOutputWriter::writeOutputFileListing(const std::string& filename)
{
	std::cout << Color::GREEN << "  ✓ " << Color::RESET
	          << Color::BOLD_WHITE << filename << Color::RESET << std::endl;
}

void ConsoleOutputWriter::writeProgress(const std::string& message)
{
	std::cout << Color::BOLD_YELLOW << "► " << message << Color::RESET << std::endl;
}

void ConsoleOutputWriter::writeError(const std::string& message)
{
	std::cerr << Color::BOLD_RED << "Error: " << Color::RESET << message << std::endl;
}

std::string ConsoleOutputWriter::getEventIcon(const std::string& eventType) const
{
	if (eventType.find("StationDelay") != std::string::npos) return "⏱️ ";
	if (eventType.find("TrackMaintenance") != std::string::npos) return "🚧";
	if (eventType.find("SignalFailure") != std::string::npos) return "🚦";
	if (eventType.find("Weather") != std::string::npos) return "⚠️ ";
	return "📢";
}