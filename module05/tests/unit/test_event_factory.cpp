#include <gtest/gtest.h>
#include <vector>
#include <string>

#include "patterns/creational/factories/EventFactory.hpp"
#include "simulation/interfaces/IEventScheduler.hpp"
#include "core/INetworkQuery.hpp"
#include "utils/IRng.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include "events/StationDelayEvent.hpp"
#include "events/TrackMaintenanceEvent.hpp"
#include "events/SignalFailureEvent.hpp"
#include "events/WeatherEvent.hpp"

class FakeRng : public IRng
{
public:
	std::vector<bool> boolValues;
	std::vector<int> intValues;
	std::vector<double> doubleValues;
	unsigned int seed = 1337;

	int getInt(int min, int max) override
	{
		if (!intValues.empty())
		{
			int value = intValues.front();
			intValues.erase(intValues.begin());
			if (value < min)
			{
				return min;
			}
			if (value > max)
			{
				return max;
			}
			return value;
		}
		return min;
	}

	double getDouble(double min, double max) override
	{
		if (!doubleValues.empty())
		{
			double value = doubleValues.front();
			doubleValues.erase(doubleValues.begin());
			if (value < min)
			{
				return min;
			}
			if (value > max)
			{
				return max;
			}
			return value;
		}
		return min;
	}

	bool getBool(double probability) override
	{
		(void)probability;
		if (!boolValues.empty())
		{
			bool value = boolValues.front();
			boolValues.erase(boolValues.begin());
			return value;
		}
		return false;
	}

	unsigned int getSeed() const override
	{
		return seed;
	}
};

class FakeNetwork : public INetworkQuery
{
public:
	std::vector<Node*> nodes;
	std::vector<Rail*> rails;

	std::vector<Node*> getNodes() const override
	{
		return nodes;
	}

	std::vector<Rail*> getRails() const override
	{
		return rails;
	}
};

class FakeEventScheduler : public IEventScheduler
{
public:
	std::vector<Event*> active;
	std::vector<Event*> scheduled;

	void scheduleEvent(Event* event) override
	{
		scheduled.push_back(event);
	}

	void update(const Time& currentTime) override
	{
		(void)currentTime;
	}

	const std::vector<Event*>& getActiveEvents() const override
	{
		return active;
	}

	const std::vector<Event*>& getScheduledEvents() const override
	{
		return scheduled;
	}

	int countActiveEventsByType(EventType type) const override
	{
		int count = 0;
		for (Event* ev : active)
		{
			if (ev && ev->getType() == type)
			{
				++count;
			}
		}
		return count;
	}

	bool hasActiveEventAt(Node* node) const override
	{
		for (Event* ev : active)
		{
			if (ev && ev->affectsNode(node))
			{
				return true;
			}
		}
		return false;
	}

	bool hasActiveEventAt(Rail* rail) const override
	{
		for (Event* ev : active)
		{
			if (ev && ev->affectsRail(rail))
			{
				return true;
			}
		}
		return false;
	}

	int getTotalEventsGenerated() const override
	{
		return static_cast<int>(scheduled.size() + active.size());
	}

	void clear() override
	{
		scheduled.clear();
		active.clear();
	}
};

class EventFactoryTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		nodeA = new Node("CityA", NodeType::CITY);
		nodeB = new Node("CityB", NodeType::CITY);
		nodeJ = new Node("RailNodeJ", NodeType::JUNCTION);

		railAB = new Rail(nodeA, nodeB, 10.0, 150.0);
		railAJ = new Rail(nodeA, nodeJ, 8.0, 120.0);

		network.nodes = {nodeA, nodeB, nodeJ};
		network.rails = {railAB, railAJ};
	}

	void TearDown() override
	{
		for (Event* ev : scheduler.active)
		{
			delete ev;
		}
		scheduler.active.clear();

		delete railAB;
		delete railAJ;
		delete nodeA;
		delete nodeB;
		delete nodeJ;
	}

	FakeRng rng;
	FakeNetwork network;
	FakeEventScheduler scheduler;

	Node* nodeA;
	Node* nodeB;
	Node* nodeJ;
	Rail* railAB;
	Rail* railAJ;
};

TEST_F(EventFactoryTest, CreatesAllEventTypesWhenAllowed)
{
	rng.boolValues = {true, true, true, true};
	rng.intValues = {
		0, 20, 30,
		0, 90,
		1, 10, 12,
		0, 180, 2
	};
	rng.doubleValues = {0.6, 25.0, 0.7, 0.02};

	EventFactory factory(rng, &network, &scheduler);
	std::vector<Event*> events = factory.tryGenerateEvents(Time("08h00"));

	ASSERT_EQ(events.size(), 4);
	EXPECT_NE(dynamic_cast<StationDelayEvent*>(events[0]), nullptr);
	EXPECT_NE(dynamic_cast<TrackMaintenanceEvent*>(events[1]), nullptr);
	EXPECT_NE(dynamic_cast<SignalFailureEvent*>(events[2]), nullptr);
	EXPECT_NE(dynamic_cast<WeatherEvent*>(events[3]), nullptr);

	for (Event* ev : events)
	{
		delete ev;
	}
}

TEST_F(EventFactoryTest, BlocksStationDelayWhenConflictExists)
{
	scheduler.active.push_back(new SignalFailureEvent(nodeA, Time("09h00"), Time("00h10"), Time("00h03")));

	rng.boolValues = {true, false, false, false};
	rng.intValues = {0};

	EventFactory factory(rng, &network, &scheduler);
	std::vector<Event*> events = factory.tryGenerateEvents(Time("09h05"));
	EXPECT_TRUE(events.empty());
}

TEST_F(EventFactoryTest, BlocksWeatherWhenAnotherWeatherIsActive)
{
	WeatherEvent* activeWeather = new WeatherEvent("Storm", nodeA, Time("10h00"), Time("02h00"), 30.0, 0.7, 0.02);
	activeWeather->setAffectedRails({railAB});
	scheduler.active.push_back(activeWeather);

	rng.boolValues = {false, false, false, true};
	EventFactory factory(rng, &network, &scheduler);
	std::vector<Event*> events = factory.tryGenerateEvents(Time("10h10"));

	EXPECT_TRUE(events.empty());
}
