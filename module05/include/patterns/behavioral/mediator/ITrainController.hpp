#ifndef ITRAINCONTROLLER_HPP
#define ITRAINCONTROLLER_HPP

class Train;
class Rail;

// Narrow interface for rail access arbitration.
// SimulationContext depends on this interface, not the concrete
// TrafficController, enabling testing and DIP compliance.
class ITrainController
{
public:
    virtual ~ITrainController() = default;

    enum AccessDecision
    {
        GRANT,
        DENY
    };

    // Request permission for train to occupy targetRail.
    // Returns GRANT if the controller allows access, DENY otherwise.
    virtual AccessDecision requestRailAccess(Train* train, Rail* targetRail) = 0;
};

#endif