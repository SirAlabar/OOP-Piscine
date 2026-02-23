#ifndef SAFETYCONSTANTS_HPP
#define SAFETYCONSTANTS_HPP

namespace SafetyConstants
{
	constexpr double SAFE_TIME_HEADWAY = 2.0;
	constexpr double MINIMUM_CLEARANCE = 50.0;
	constexpr double BRAKING_MARGIN = 1.1;
	constexpr double EMERGENCY_TTC_THRESHOLD = 3.0;
	constexpr double EMERGENCY_EXIT_HYSTERESIS = 1.2;
	constexpr double CAR_FOLLOWING_KP = 0.3;
	constexpr double CAR_FOLLOWING_KD = 1.0;
	constexpr double CAR_FOLLOWING_MAX_ACCEL = 1.5;

	// Zone detection with sentinel value guard
	inline bool isEmergencyZone(double gap, double brakingDistance)
	{
		return gap >= 0.0 && gap <= brakingDistance;
	}

	inline bool isFollowingZone(double gap, double brakingDistance, double safeDistance)
	{
		return gap >= 0.0 && gap > brakingDistance && gap <= safeDistance;
	}

	inline bool isFreeZone(double gap, double safeDistance)
	{
		return gap >= 0.0 && gap > safeDistance;
	}
}

#endif