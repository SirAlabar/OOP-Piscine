#ifndef RISKDATA_HPP
#define RISKDATA_HPP

class Train;

struct RiskData
{
	// Spatial relationships
	double gap;              // meters, -1 if no leader
	double closingSpeed;     // m/s (positive = approaching)
	
	// Physical constraints
	double brakingDistance;  // meters
	double safeDistance;     // meters (nominal)
	
	// Rail constraints
	double currentSpeedLimit; // m/s
	double nextSpeedLimit;    // m/s, -1 if none
	
	// Leader reference
	Train* leader;           // nullptr if none
	
	RiskData()
		: gap(-1.0),
		  closingSpeed(0.0),
		  brakingDistance(0.0),
		  safeDistance(100.0),
		  currentSpeedLimit(0.0),
		  nextSpeedLimit(-1.0),
		  leader(nullptr)
	{
	}
	
	// --- Pure factual queries ---
	bool hasLeader() const
	{
		return leader != nullptr && gap >= 0.0;
	}
	
	bool isClosingIn() const
	{
		return hasLeader() && closingSpeed > 0.1;
	}
	
	bool isWithinSafeDistance() const
	{
		return hasLeader() && gap < safeDistance;
	}
	
	bool isWithinBrakingDistance() const
	{
		return hasLeader() && gap < brakingDistance;
	}
	
	double timeToCollision() const
	{
		if (!hasLeader() || closingSpeed <= 0.0)
		{
			return -1.0;
		}
		return gap / closingSpeed;
	}
};

#endif