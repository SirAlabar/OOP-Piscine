#ifndef OCCUPANCYMAP_HPP
#define OCCUPANCYMAP_HPP

#include <unordered_map>
#include <vector>

class Rail;
class Train;

// Tracks which trains are currently on each rail segment.
// Owned by CollisionAvoidance; rebuilt every tick via refreshRailOccupancy().
// Rail itself is now a pure geometric data class with no runtime train state.
class OccupancyMap
{
public:
    OccupancyMap()  = default;
    ~OccupancyMap() = default;

    OccupancyMap(const OccupancyMap&)            = default;
    OccupancyMap& operator=(const OccupancyMap&) = default;

    // Add train to rail's occupant list.  Ignores duplicates and nulls.
    void add(Rail* rail, Train* train);

    // Remove train from rail's occupant list.  Ignores unknown pairs.
    void remove(Rail* rail, Train* train);

    // Returns true when at least one train occupies rail.
    bool hasTrains(Rail* rail) const;

    // Returns the (possibly empty) list of trains on rail.
    // The returned reference is stable until the next add/remove/clearAll.
    const std::vector<Train*>& get(Rail* rail) const;

    // Remove all trains from every rail in the provided list.
    void clearAll(const std::vector<Rail*>& rails);

private:
    std::unordered_map<Rail*, std::vector<Train*>> _map;

    // Returned by get() when no entry exists for the requested rail.
    static const std::vector<Train*> _empty;
};

#endif