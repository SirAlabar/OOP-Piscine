#include <gtest/gtest.h>
#include "utils/Time.hpp"

TEST(TimeTest, DefaultConstructor)
{
	Time t;
	EXPECT_EQ(t.getHours(), 0);
	EXPECT_EQ(t.getMinutes(), 0);
	EXPECT_TRUE(t.isValid());
}

TEST(TimeTest, ParameterizedConstructor)
{
	Time t(14, 30);
	EXPECT_EQ(t.getHours(), 14);
	EXPECT_EQ(t.getMinutes(), 30);
	EXPECT_TRUE(t.isValid());
}

TEST(TimeTest, ParseValidString)
{
	Time t("14h30");
	EXPECT_EQ(t.getHours(), 14);
	EXPECT_EQ(t.getMinutes(), 30);
}

TEST(TimeTest, ParseStringWithLeadingZeros)
{
	Time t("04h05");
	EXPECT_EQ(t.getHours(), 4);
	EXPECT_EQ(t.getMinutes(), 5);
}

TEST(TimeTest, ParseStringMidnight)
{
	Time t("00h00");
	EXPECT_EQ(t.getHours(), 0);
	EXPECT_EQ(t.getMinutes(), 0);
}

TEST(TimeTest, ParseInvalidString)
{
	Time t("invalid");
	EXPECT_EQ(t.getHours(), 0);
	EXPECT_EQ(t.getMinutes(), 0);
}

TEST(TimeTest, ToMinutesConversion)
{
	Time t1(0, 0);
	Time t2(1, 0);
	Time t3(0, 30);
	Time t4(14, 45);
	
	EXPECT_EQ(t1.toMinutes(), 0);
	EXPECT_EQ(t2.toMinutes(), 60);
	EXPECT_EQ(t3.toMinutes(), 30);
	EXPECT_EQ(t4.toMinutes(), 885);  // 14*60 + 45
}

TEST(TimeTest, ToStringFormatting)
{
	Time t1(9, 5);
	Time t2(14, 30);
	Time t3(0, 0);
	
	EXPECT_EQ(t1.toString(), "09h05");
	EXPECT_EQ(t2.toString(), "14h30");
	EXPECT_EQ(t3.toString(), "00h00");
}

TEST(TimeTest, ValidationValid)
{
	Time t1(0, 0);
	Time t2(23, 59);
	Time t3(12, 30);
	
	EXPECT_TRUE(t1.isValid());
	EXPECT_TRUE(t2.isValid());
	EXPECT_TRUE(t3.isValid());
}

TEST(TimeTest, ValidationInvalidHours)
{
	Time t1(-1, 0);
	Time t2(24, 0);
	Time t3(25, 30);
	
	EXPECT_FALSE(t1.isValid());
	EXPECT_FALSE(t2.isValid());
	EXPECT_FALSE(t3.isValid());
}

TEST(TimeTest, ValidationInvalidMinutes)
{
	Time t1(12, -1);
	Time t2(12, 60);
	Time t3(12, 65);
	
	EXPECT_FALSE(t1.isValid());
	EXPECT_FALSE(t2.isValid());
	EXPECT_FALSE(t3.isValid());
}

TEST(TimeTest, EqualityOperator)
{
	Time t1(14, 30);
	Time t2(14, 30);
	Time t3(15, 30);
	
	EXPECT_TRUE(t1 == t2);
	EXPECT_FALSE(t1 == t3);
}

TEST(TimeTest, InequalityOperator)
{
	Time t1(14, 30);
	Time t2(15, 30);
	
	EXPECT_TRUE(t1 != t2);
	EXPECT_FALSE(t1 != t1);
}

TEST(TimeTest, LessThanOperator)
{
	Time t1(14, 30);
	Time t2(14, 45);
	Time t3(15, 00);
	
	EXPECT_TRUE(t1 < t2);
	EXPECT_TRUE(t1 < t3);
	EXPECT_FALSE(t2 < t1);
}

TEST(TimeTest, LessThanOrEqualOperator)
{
	Time t1(14, 30);
	Time t2(14, 30);
	Time t3(14, 45);
	
	EXPECT_TRUE(t1 <= t2);
	EXPECT_TRUE(t1 <= t3);
	EXPECT_FALSE(t3 <= t1);
}

TEST(TimeTest, GreaterThanOperator)
{
	Time t1(15, 00);
	Time t2(14, 30);
	
	EXPECT_TRUE(t1 > t2);
	EXPECT_FALSE(t2 > t1);
}

TEST(TimeTest, GreaterThanOrEqualOperator)
{
	Time t1(14, 30);
	Time t2(14, 30);
	Time t3(14, 15);
	
	EXPECT_TRUE(t1 >= t2);
	EXPECT_TRUE(t1 >= t3);
	EXPECT_FALSE(t3 >= t1);
}

TEST(TimeTest, AdditionOperator)
{
	Time t1(10, 30);
	Time t2(2, 45);
	Time result = t1 + t2;
	
	EXPECT_EQ(result.getHours(), 13);
	EXPECT_EQ(result.getMinutes(), 15);
}

TEST(TimeTest, AdditionWithMinuteOverflow)
{
	Time t1(10, 45);
	Time t2(0, 30);
	Time result = t1 + t2;
	
	EXPECT_EQ(result.getHours(), 11);
	EXPECT_EQ(result.getMinutes(), 15);
}

TEST(TimeTest, SubtractionOperator)
{
	Time t1(14, 30);
	Time t2(2, 15);
	Time result = t1 - t2;
	
	EXPECT_EQ(result.getHours(), 12);
	EXPECT_EQ(result.getMinutes(), 15);
}

TEST(TimeTest, SubtractionWithMinuteBorrow)
{
	Time t1(14, 15);
	Time t2(0, 30);
	Time result = t1 - t2;
	
	EXPECT_EQ(result.getHours(), 13);
	EXPECT_EQ(result.getMinutes(), 45);
}

TEST(TimeTest, SubtractionNegativeResult)
{
	Time t1(10, 0);
	Time t2(12, 0);
	Time result = t1 - t2;
	
	EXPECT_EQ(result.getHours(), 0);
	EXPECT_EQ(result.getMinutes(), 0);
}

TEST(TimeTest, CopyConstructor)
{
	Time original(14, 30);
	Time copy(original);
	
	EXPECT_EQ(copy.getHours(), 14);
	EXPECT_EQ(copy.getMinutes(), 30);
}

TEST(TimeTest, AssignmentOperator)
{
	Time t1(14, 30);
	Time t2(10, 15);
	
	t2 = t1;
	EXPECT_EQ(t2.getHours(), 14);
	EXPECT_EQ(t2.getMinutes(), 30);
}