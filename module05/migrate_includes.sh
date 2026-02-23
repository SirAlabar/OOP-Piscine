#!/usr/bin/env bash

echo "Fixing remaining adapter and event includes..."

FILES=$(find include src tests -type f \( -name "*.hpp" -o -name "*.cpp" \))

fix() {
    OLD=$1
    NEW=$2
    echo "$FILES" | xargs sed -i "s|$OLD|$NEW|g"
}

#
# ADAPTERS
#
fix "patterns/observers/TrainEventAdapter.hpp" "patterns/structural/adapter/TrainEventAdapter.hpp"
fix "patterns/observers/RailEventAdapter.hpp" "patterns/structural/adapter/RailEventAdapter.hpp"

fix "patterns/events/TrainEventAdapter.hpp" "patterns/structural/adapter/TrainEventAdapter.hpp"
fix "patterns/events/RailEventAdapter.hpp" "patterns/structural/adapter/RailEventAdapter.hpp"

#
# EVENT HEADERS
#
fix "patterns/events/Event.hpp" "events/Event.hpp"
fix "patterns/events/SignalFailureEvent.hpp" "events/SignalFailureEvent.hpp"
fix "patterns/events/StationDelayEvent.hpp" "events/StationDelayEvent.hpp"
fix "patterns/events/TrackMaintenanceEvent.hpp" "events/TrackMaintenanceEvent.hpp"
fix "patterns/events/WeatherEvent.hpp" "events/WeatherEvent.hpp"

#
# EVENT SYSTEM HEADERS
#
fix "patterns/observers/EventDispatcher.hpp" "event_system/EventDispatcher.hpp"
fix "patterns/observers/EventScheduler.hpp" "event_system/EventScheduler.hpp"
fix "patterns/observers/ObserverManager.hpp" "event_system/ObserverManager.hpp"

echo "Remaining includes fixed."