#include <gtest/gtest.h>

#include "patterns/observers/EventDispatcher.hpp"
#include "patterns/observers/EventScheduler.hpp"
#include "patterns/observers/IObserver.hpp"
#include "patterns/events/StationDelayEvent.hpp"
#include "core/Node.hpp"

class CountingObserver : public IObserver
{
public:
	int notifications = 0;
	void onNotify(Event* event) override
	{
		if (event)
		{
			++notifications;
		}
	}
};

TEST(EventDispatcherTest, AttachNotifyDetachWorks)
{
	EventDispatcher dispatcher;
	CountingObserver obs;

	dispatcher.attach(&obs);
	Node station("CityA");
	Event* event = new StationDelayEvent(&station, Time("09h00"), Time("00h10"), Time("00h02"));

	dispatcher.notify(event);
	EXPECT_EQ(obs.notifications, 1);

	dispatcher.detach(&obs);
	dispatcher.notify(event);
	EXPECT_EQ(obs.notifications, 1);

	delete event;
}

TEST(EventSchedulerTest, ScheduledEventActivatesAndExpires)
{
	EventDispatcher dispatcher;
	CountingObserver obs;
	dispatcher.attach(&obs);

	EventScheduler scheduler(dispatcher);
	Node station("CityA");

	scheduler.scheduleEvent(new StationDelayEvent(&station, Time("10h00"), Time("00h05"), Time("00h01")));
	ASSERT_EQ(scheduler.getScheduledEvents().size(), 1);
	ASSERT_EQ(scheduler.getActiveEvents().size(), 0);

	scheduler.update(Time("10h02"));
	EXPECT_EQ(scheduler.getScheduledEvents().size(), 0);
	EXPECT_EQ(scheduler.getActiveEvents().size(), 1);
	EXPECT_EQ(obs.notifications, 1);

	scheduler.update(Time("10h06"));
	EXPECT_EQ(scheduler.getActiveEvents().size(), 0);
	EXPECT_EQ(obs.notifications, 2);
}

TEST(EventSchedulerTest, ClearRemovesAllEventsAndResetsCounter)
{
	EventDispatcher dispatcher;
	EventScheduler scheduler(dispatcher);
	Node station("CityA");

	scheduler.scheduleEvent(new StationDelayEvent(&station, Time("11h00"), Time("00h05"), Time("00h01")));
	scheduler.scheduleEvent(new StationDelayEvent(&station, Time("12h00"), Time("00h05"), Time("00h01")));
	EXPECT_EQ(scheduler.getTotalEventsGenerated(), 2);

	scheduler.clear();
	EXPECT_TRUE(scheduler.getScheduledEvents().empty());
	EXPECT_TRUE(scheduler.getActiveEvents().empty());
	EXPECT_EQ(scheduler.getTotalEventsGenerated(), 0);
}
