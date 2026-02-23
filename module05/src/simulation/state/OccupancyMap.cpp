#include "simulation/state/OccupancyMap.hpp"
#include "core/Rail.hpp"
#include <algorithm>

const std::vector<Train*> OccupancyMap::_empty;

void OccupancyMap::add(Rail* rail, Train* train)
{
    if (!rail || !train)
    {
        return;
    }

    std::vector<Train*>& occupants = _map[rail];

    for (Train* t : occupants)
    {
        if (t == train)
        {
            return;  // Already present — no duplicates.
        }
    }

    occupants.push_back(train);
}

void OccupancyMap::remove(Rail* rail, Train* train)
{
    if (!rail || !train)
    {
        return;
    }

    auto it = _map.find(rail);

    if (it == _map.end())
    {
        return;
    }

    std::vector<Train*>& occupants = it->second;

    auto pos = std::find(occupants.begin(), occupants.end(), train);

    if (pos != occupants.end())
    {
        occupants.erase(pos);
    }
}

bool OccupancyMap::hasTrains(Rail* rail) const
{
    if (!rail)
    {
        return false;
    }

    auto it = _map.find(rail);

    return it != _map.end() && !it->second.empty();
}

const std::vector<Train*>& OccupancyMap::get(Rail* rail) const
{
    if (!rail)
    {
        return _empty;
    }

    auto it = _map.find(rail);

    if (it == _map.end())
    {
        return _empty;
    }

    return it->second;
}

void OccupancyMap::clearAll(const std::vector<Rail*>& rails)
{
    for (Rail* rail : rails)
    {
        if (rail)
        {
            auto it = _map.find(rail);

            if (it != _map.end())
            {
                it->second.clear();
            }
        }
    }
}