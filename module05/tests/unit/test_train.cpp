#include <gtest/gtest.h>
#include "core/Train.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "utils/Time.hpp"

class TrainTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		Train::resetIDCounter();  // Reset ID counter before each test
	}
};

TEST_F(TrainTest, DefaultConstructor)
{
	Train t;
	EXPECT_EQ(t.getName(), "");
	EXPECT_EQ(t.getID(), 1);
	EXPECT_EQ(t.getMass(), 0.0);
	EXPECT_EQ(t.getVelocity(), 0.0);
	EXPECT_EQ(t.getPosition(), 0.0);
	EXPECT_FALSE(t.isValid());
}

TEST_F(TrainTest, ParameterizedConstructor)
{
	Time depTime("14h10");
	Time stopDur("00h05");
	Train t("Express1", 80.0, 0.005, 356.0, 500.0, "CityA", "CityB", depTime, stopDur);
	
	EXPECT_EQ(t.getName(), "Express1");
	EXPECT_EQ(t.getID(), 1);
	EXPECT_EQ(t.getMass(), 80.0);
	EXPECT_EQ(t.getFrictionCoef(), 0.005);
	EXPECT_EQ(t.getMaxAccelForce(), 356.0);
	EXPECT_EQ(t.getMaxBrakeForce(), 500.0);
	EXPECT_EQ(t.getDepartureStation(), "CityA");
	EXPECT_EQ(t.getArrivalStation(), "CityB");
	EXPECT_EQ(t.getDepartureTime().toString(), "14h10");
	EXPECT_EQ(t.getStopDuration().toString(), "00h05");
	EXPECT_TRUE(t.isValid());
}

TEST_F(TrainTest, SequentialIDGeneration)
{
	Time depTime("10h00");
	Time stopDur("00h10");
	
	Train t1("Train1", 80.0, 0.005, 356.0, 500.0, "A", "B", depTime, stopDur);
	Train t2("Train2", 60.0, 0.005, 400.0, 450.0, "A", "C", depTime, stopDur);
	Train t3("Train3", 70.0, 0.005, 380.0, 480.0, "B", "C", depTime, stopDur);
	
	EXPECT_EQ(t1.getID(), 1);
	EXPECT_EQ(t2.getID(), 2);
	EXPECT_EQ(t3.getID(), 3);
}

TEST_F(TrainTest, CopyConstructor)
{
	Time depTime("14h10");
	Time stopDur("00h05");
	Train original("Express1", 80.0, 0.005, 356.0, 500.0, "CityA", "CityB", depTime, stopDur);
	Train copy(original);
	
	EXPECT_EQ(copy.getName(), "Express1");
	EXPECT_EQ(copy.getID(), 1);
	EXPECT_EQ(copy.getMass(), 80.0);
	EXPECT_EQ(copy.getDepartureStation(), "CityA");
	EXPECT_EQ(copy.getDepartureTime().toString(), "14h10");
}

TEST_F(TrainTest, AssignmentOperator)
{
	Time depTime1("14h10");
	Time depTime2("15h20");
	Time stopDur("00h05");
	
	Train t1("Train1", 80.0, 0.005, 356.0, 500.0, "A", "B", depTime1, stopDur);
	Train t2("Train2", 60.0, 0.004, 400.0, 450.0, "C", "D", depTime2, stopDur);
	
	t2 = t1;
	EXPECT_EQ(t2.getName(), "Train1");
	EXPECT_EQ(t2.getMass(), 80.0);
	EXPECT_EQ(t2.getDepartureStation(), "A");
	EXPECT_EQ(t2.getDepartureTime().toString(), "14h10");
}

TEST_F(TrainTest, VelocityGetterSetter)
{
	Time depTime("10h00");
	Time stopDur("00h05");
	Train t("Express", 80.0, 0.005, 356.0, 500.0, "A", "B", depTime, stopDur);
	
	EXPECT_EQ(t.getVelocity(), 0.0);
	t.setVelocity(25.5);
	EXPECT_EQ(t.getVelocity(), 25.5);
}

TEST_F(TrainTest, PositionGetterSetter)
{
	Time depTime("10h00");
	Time stopDur("00h05");
	Train t("Express", 80.0, 0.005, 356.0, 500.0, "A", "B", depTime, stopDur);
	
	EXPECT_EQ(t.getPosition(), 0.0);
	t.setPosition(1500.0);
	EXPECT_EQ(t.getPosition(), 1500.0);
}

TEST_F(TrainTest, PathManagement)
{
	Time depTime("10h00");
	Time stopDur("00h05");
	Train t("Express", 80.0, 0.005, 356.0, 500.0, "A", "B", depTime, stopDur);
	
	Node a("A"), b("B"), c("C");
	Rail r1(&a, &b, 10.0, 100.0);
	Rail r2(&b, &c, 15.0, 120.0);
	
	std::vector<Rail*> path = {&r1, &r2};
	t.setPath(path);
	
	EXPECT_EQ(t.getPath().size(), 2);
	EXPECT_EQ(t.getCurrentRail(), &r1);
	EXPECT_EQ(t.getCurrentRailIndex(), 0);
}

TEST_F(TrainTest, AdvanceToNextRail)
{
	Time depTime("10h00");
	Time stopDur("00h05");
	Train t("Express", 80.0, 0.005, 356.0, 500.0, "A", "B", depTime, stopDur);
	
	Node a("A"), b("B"), c("C");
	Rail r1(&a, &b, 10.0, 100.0);
	Rail r2(&b, &c, 15.0, 120.0);
	
	std::vector<Rail*> path = {&r1, &r2};
	t.setPath(path);
	
	t.advanceToNextRail();
	EXPECT_EQ(t.getCurrentRail(), &r2);
	EXPECT_EQ(t.getCurrentRailIndex(), 1);
	
	t.advanceToNextRail();
	EXPECT_EQ(t.getCurrentRail(), nullptr);
	EXPECT_EQ(t.getCurrentRailIndex(), 2);
}

TEST_F(TrainTest, ValidationEmptyName)
{
	Time depTime("10h00");
	Time stopDur("00h05");
	Train t("", 80.0, 0.005, 356.0, 500.0, "A", "B", depTime, stopDur);
	EXPECT_FALSE(t.isValid());
}

TEST_F(TrainTest, ValidationZeroMass)
{
	Time depTime("10h00");
	Time stopDur("00h05");
	Train t("Train", 0.0, 0.005, 356.0, 500.0, "A", "B", depTime, stopDur);
	EXPECT_FALSE(t.isValid());
}

TEST_F(TrainTest, ValidationNegativeMass)
{
	Time depTime("10h00");
	Time stopDur("00h05");
	Train t("Train", -80.0, 0.005, 356.0, 500.0, "A", "B", depTime, stopDur);
	EXPECT_FALSE(t.isValid());
}

TEST_F(TrainTest, ValidationZeroAccelForce)
{
	Time depTime("10h00");
	Time stopDur("00h05");
	Train t("Train", 80.0, 0.005, 0.0, 500.0, "A", "B", depTime, stopDur);
	EXPECT_FALSE(t.isValid());
}

TEST_F(TrainTest, ValidationZeroBrakeForce)
{
	Time depTime("10h00");
	Time stopDur("00h05");
	Train t("Train", 80.0, 0.005, 356.0, 0.0, "A", "B", depTime, stopDur);
	EXPECT_FALSE(t.isValid());
}

TEST_F(TrainTest, ValidationEmptyStations)
{
	Time depTime("10h00");
	Time stopDur("00h05");
	Train t1("Train", 80.0, 0.005, 356.0, 500.0, "", "B", depTime, stopDur);
	Train t2("Train", 80.0, 0.005, 356.0, 500.0, "A", "", depTime, stopDur);
	EXPECT_FALSE(t1.isValid());
	EXPECT_FALSE(t2.isValid());
}

TEST_F(TrainTest, StateManagement)
{
	Time depTime("10h00");
	Time stopDur("00h05");
	Train t("Express", 80.0, 0.005, 356.0, 500.0, "A", "B", depTime, stopDur);
	
	EXPECT_EQ(t.getCurrentState(), nullptr);
	// setState tested in Week 3 with actual ITrainState implementations
}

TEST_F(TrainTest, ResetIDCounter)
{
	Time depTime("10h00");
	Time stopDur("00h05");
	
	Train t1("Train1", 80.0, 0.005, 356.0, 500.0, "A", "B", depTime, stopDur);
	EXPECT_EQ(t1.getID(), 1);
	
	Train::resetIDCounter();
	Train t2("Train2", 80.0, 0.005, 356.0, 500.0, "A", "B", depTime, stopDur);
	EXPECT_EQ(t2.getID(), 1);
}

TEST_F(TrainTest, GetNextID)
{
	EXPECT_EQ(Train::getNextID(), 1);
	Time depTime("10h00");
	Time stopDur("00h05");
	Train t("Train", 80.0, 0.005, 356.0, 500.0, "A", "B", depTime, stopDur);
	EXPECT_EQ(Train::getNextID(), 2);
}