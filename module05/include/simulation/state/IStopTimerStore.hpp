#ifndef ISTOPTIMERSTORE_HPP
#define ISTOPTIMERSTORE_HPP

class Train;

// Per-train stop-duration bookkeeping.
// Implemented by SimulationContext; injectable for unit testing.
class IStopTimerStore
{
public:
    virtual ~IStopTimerStore() = default;

    // Set or overwrite the stop duration for a train, in seconds.
    virtual void setStopDuration(Train* train, double durationSeconds) = 0;

    // Current remaining stop duration in seconds; 0.0 if not set.
    virtual double getStopDuration(const Train* train) const = 0;

    // Subtract dt from the stored duration.
    // Returns true when the duration has just reached or passed zero.
    virtual bool decrementStopDuration(Train* train, double dt) = 0;

    // Remove the stop-duration entry for a train.
    virtual void clearStopDuration(Train* train) = 0;
};

#endif