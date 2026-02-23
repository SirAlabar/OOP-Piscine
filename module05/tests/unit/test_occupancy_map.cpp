#include <gtest/gtest.h>
#include "simulation/state/OccupancyMap.hpp"
#include "core/Rail.hpp"
#include "core/Node.hpp"
#include "core/Train.hpp"

TEST(OccupancyMapTest, EmptyByDefault)
{
    Node a("CityA");
    Node b("CityB");
    Rail r(&a, &b, 10.0, 100.0);
    OccupancyMap map;

    EXPECT_FALSE(map.hasTrains(&r));
    EXPECT_TRUE(map.get(&r).empty());
}

TEST(OccupancyMapTest, NullRailSafe)
{
    OccupancyMap map;

    EXPECT_FALSE(map.hasTrains(nullptr));
    EXPECT_TRUE(map.get(nullptr).empty());
}

TEST(OccupancyMapTest, AddAndRemoveTrain)
{
    Node a("CityA");
    Node b("CityB");
    Rail  r(&a, &b, 10.0, 100.0);
    Train t;
    OccupancyMap map;

    map.add(&r, &t);
    EXPECT_TRUE(map.hasTrains(&r));
    EXPECT_EQ(map.get(&r).size(), 1u);
    EXPECT_EQ(map.get(&r)[0], &t);

    map.remove(&r, &t);
    EXPECT_FALSE(map.hasTrains(&r));
    EXPECT_TRUE(map.get(&r).empty());
}

TEST(OccupancyMapTest, NoDuplicates)
{
    Node  a("CityA");
    Node  b("CityB");
    Rail  r(&a, &b, 10.0, 100.0);
    Train t;
    OccupancyMap map;

    map.add(&r, &t);
    map.add(&r, &t);  // Should be ignored.

    EXPECT_EQ(map.get(&r).size(), 1u);
}

TEST(OccupancyMapTest, MultipleTrains)
{
    Node  a("CityA");
    Node  b("CityB");
    Rail  r(&a, &b, 10.0, 100.0);
    Train t1, t2, t3;
    OccupancyMap map;

    map.add(&r, &t1);
    map.add(&r, &t2);
    map.add(&r, &t3);

    EXPECT_EQ(map.get(&r).size(), 3u);
    EXPECT_TRUE(map.hasTrains(&r));
}

TEST(OccupancyMapTest, RemoveSpecificTrain)
{
    Node  a("CityA");
    Node  b("CityB");
    Rail  r(&a, &b, 10.0, 100.0);
    Train t1, t2, t3;
    OccupancyMap map;

    map.add(&r, &t1);
    map.add(&r, &t2);
    map.add(&r, &t3);

    map.remove(&r, &t2);

    EXPECT_EQ(map.get(&r).size(), 2u);

    const auto& occupants = map.get(&r);
    bool hasT1 = false, hasT2 = false, hasT3 = false;

    for (Train* t : occupants)
    {
        if (t == &t1) hasT1 = true;
        if (t == &t2) hasT2 = true;
        if (t == &t3) hasT3 = true;
    }

    EXPECT_TRUE(hasT1);
    EXPECT_FALSE(hasT2);
    EXPECT_TRUE(hasT3);
}

TEST(OccupancyMapTest, ClearAll)
{
    Node  a("CityA");
    Node  b("CityB");
    Node  c("CityC");
    Rail  r1(&a, &b, 10.0, 100.0);
    Rail  r2(&b, &c, 20.0, 80.0);
    Train t1, t2;
    OccupancyMap map;

    map.add(&r1, &t1);
    map.add(&r2, &t2);

    std::vector<Rail*> rails = { &r1, &r2 };
    map.clearAll(rails);

    EXPECT_TRUE(map.get(&r1).empty());
    EXPECT_TRUE(map.get(&r2).empty());
}
