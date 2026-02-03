#ifndef OUTPUTWRITER_HPP
#define OUTPUTWRITER_HPP

#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "utils/Time.hpp"
#include <string>
#include <fstream>
#include <vector>

// Generates output files for train journeys
class OutputWriter
{
public:
	OutputWriter(Train* train);
	~OutputWriter();

	// Open output file
	void open();

	// Write header with estimated time
	void writeHeader(double estimatedTimeMinutes);

	// Write snapshot at current simulation time
	void writeSnapshot(double currentTimeSeconds);

	// Close file
	void close();

private:
	Train* _train;
	std::ofstream _file;
	std::string _filename;

	// Generate filename: TrainName_DepartureTime.result
	std::string generateFilename() const;

	// Get status string from current state
	std::string getStatusString() const;

	// Calculate remaining distance to final destination
	double calculateRemainingDistance() const;

	// Generate ASCII rail visualization
	std::string generateRailVisualization() const;

	// Format time as HHhMM
	std::string formatTime(double seconds) const;
};

#endif