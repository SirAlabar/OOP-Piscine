#include <gtest/gtest.h>

#include "patterns/events/Event.hpp"
#include "patterns/events/TrackMaintenanceEvent.hpp"
#include "patterns/events/WeatherEvent.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"

TEST(EventTest, TypeToStringCoversAllKnownTypes)
{
	EXPECT_EQ(Event::typeToString(EventType::STATION_DELAY), "STATION DELAY");
	EXPECT_EQ(Event::typeToString(EventType::TRACK_MAINTENANCE), "TRACK MAINTENANCE");
	EXPECT_EQ(Event::typeToString(EventType::SIGNAL_FAILURE), "SIGNAL FAILURE");
	EXPECT_EQ(Event::typeToString(EventType::WEATHER), "WEATHER EVENT");
}

TEST(EventTest, TrackMaintenanceLifecycleModifiesAndRestoresSpeedLimit)
{
	Node a("CityA");
	Node b("CityB");
	Rail rail(&a, &b, 10.0, 200.0);

	TrackMaintenanceEvent maintenance(&rail, Time("08h00"), Time("00h30"), 0.5);

	EXPECT_FALSE(maintenance.isActive());
	EXPECT_DOUBLE_EQ(rail.getSpeedLimit(), 200.0);

	maintenance.update(Time("08h10"));
	EXPECT_TRUE(maintenance.isActive());
	EXPECT_DOUBLE_EQ(rail.getSpeedLimit(), 100.0);

	maintenance.update(Time("08h31"));
	EXPECT_FALSE(maintenance.isActive());
	EXPECT_DOUBLE_EQ(rail.getSpeedLimit(), 200.0);
}

TEST(EventTest, WeatherLifecycleAffectsOnlyConfiguredRails)
{
	Node a("CityA");
	Node b("CityB");
	Node c("CityC");

	Rail railAB(&a, &b, 12.0, 160.0);
	Rail railBC(&b, &c, 8.0, 120.0);

	WeatherEvent weather("Storm", &b, Time("10h00"), Time("01h00"), 20.0, 0.75, 0.02);
	weather.setAffectedRails({&railAB});

	weather.update(Time("10h05"));
	EXPECT_TRUE(weather.isActive());
	EXPECT_DOUBLE_EQ(railAB.getSpeedLimit(), 120.0);
	EXPECT_DOUBLE_EQ(railBC.getSpeedLimit(), 120.0);

	weather.update(Time("11h05"));
	EXPECT_FALSE(weather.isActive());
	EXPECT_DOUBLE_EQ(railAB.getSpeedLimit(), 160.0);
}
