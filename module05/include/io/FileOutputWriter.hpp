#ifndef FILEOUTPUTWRITER_HPP
#define FILEOUTPUTWRITER_HPP

#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "simulation/state/OccupancyMap.hpp"
#include "utils/Time.hpp"
#include <string>
#include <fstream>
#include <vector>

// Generates output files for train journeys
class FileOutputWriter
{
public:
	FileOutputWriter(Train* train);
	~FileOutputWriter();

	// Open output file
	void open();

	// Write header with estimated time
	void writeHeader(double estimatedTimeMinutes);
	
	// Write path information (segments, distances, speeds)
	void writePathInfo();

	// Write snapshot at current simulation time
	void writeSnapshot(double currentTimeSeconds);
	
	// Write event notification
	void writeEventNotification(double currentTimeSeconds, const std::string& eventType, 
	                            const std::string& eventDetails, const std::string& action);

	// Close file
	void close();

    // Inject a read-only view of the current rail occupancy.
    // Called by SimulationManager after registerOutputWriter().
    // Pass nullptr to disable multi-train visualization (default).
    void setOccupancyMap(const OccupancyMap* occupancy);

private:
	Train*              _train;
	std::ofstream       _file;
	std::string         _filename;
	double              _totalPathDistance;
	bool                _finalSnapshotWritten;
    const OccupancyMap* _occupancy;  // Non-owning; may be nullptr.

	std::string generateFilename() const;
	double      calculateTotalPathDistance() const;
	std::string getStatusString() const;
	double      calculateRemainingDistance() const;
	std::string generateRailVisualization() const;
	std::string formatTime(double seconds) const;
};

#endif