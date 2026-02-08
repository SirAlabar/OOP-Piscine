// #include <gtest/gtest.h>
// #include <iostream>
// #include <iomanip>

// #include "simulation/SimulationManager.hpp"
// #include "patterns/factories/TrainFactory.hpp"
// #include "patterns/strategies/DijkstraStrategy.hpp"

// #include "core/Graph.hpp"
// #include "core/Node.hpp"
// #include "core/Rail.hpp"
// #include "core/Train.hpp"

// #include "patterns/states/IdleState.hpp"

// using std::cout;
// using std::endl;

// class RealisticFlowTest : public ::testing::Test
// {
// protected:
//     Graph graph;
//     Node *nodeA, *nodeB;
//     Rail *rail;

//     void SetUp() override
//     {
//         Train::resetIDCounter();

//         nodeA = new Node("CityA");
//         nodeB = new Node("CityB");

//         graph.addNode(nodeA);
//         graph.addNode(nodeB);

//         rail = new Rail(nodeA, nodeB, 10.0, 200.0);
//         graph.addRail(rail);

//         SimulationManager::getInstance().reset();
//         SimulationManager::getInstance().setNetwork(&graph);
//     }

//     void TearDown() override
//     {
//         SimulationManager::getInstance().reset();

//         delete rail;
//         delete nodeA;
//         delete nodeB;
//     }

//     void printDebug(int step, Train* leader, Train* follower)
//     {
//         cout << "[t=" << std::setw(3) << step << "] "
//              << "Leader: pos=" << std::setw(6) << leader->getPosition()
//              << " v=" << std::setw(5) << leader->getVelocity()
//              << " state=" << leader->getCurrentState()->getName()
//              << " | Follower: pos=" << std::setw(6) << follower->getPosition()
//              << " v=" << std::setw(5) << follower->getVelocity()
//              << " state=" << follower->getCurrentState()->getName()
//              << endl;
//     }
// };

// TEST_F(RealisticFlowTest, SingleRealisticTrafficFlow)
// {
//     cout << "\n==== REALISTIC TRAFFIC FLOW DEBUG TEST ====\n";

//     TrainConfig leaderCfg = {
//         "Leader", 80.0, 0.005, 300.0, 400.0,
//         "CityA", "CityB",
//         Time("00h00"), Time("00h05")
//     };

//     TrainConfig followerCfg = {
//         "Follower", 80.0, 0.005, 300.0, 400.0,
//         "CityA", "CityB",
//         Time("00h00"), Time("00h05")
//     };

//     Train* leader = TrainFactory::create(leaderCfg, &graph);
//     Train* follower = TrainFactory::create(followerCfg, &graph);

//     ASSERT_NE(leader, nullptr);
//     ASSERT_NE(follower, nullptr);

//     DijkstraStrategy dijkstra;

//     leader->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
//     follower->setPath(dijkstra.findPath(&graph, nodeA, nodeB));

//     // follower começa atrás
//     follower->setPosition(-200.0);

//     static IdleState idle;
//     leader->setState(&idle);
//     follower->setState(&idle);

//     SimulationManager::getInstance().addTrain(leader);
//     SimulationManager::getInstance().addTrain(follower);

//     SimulationManager::getInstance().start();

//     bool sawBraking = false;
//     bool sawStopped = false;
//     bool sawWaiting = false;

//     double minDistance = 1e9;

//     for (int t = 0; t < 400; ++t)
//     {
//         SimulationManager::getInstance().step();

//         // Em t=100 o líder para abruptamente
//         if (t == 100)
//         {
//             std::cout << "\n>>> EVENT: LEADER FORCED TO STOP FOR EXTENDED PERIOD <<<\n";
//         }
//         if (t >= 100 && t < 180)
//         {
//             leader->setVelocity(0.0);
//         }

//         std::string state = follower->getCurrentState()->getName();

//         if (state == "Braking")
//             sawBraking = true;

//         if (state == "Stopped")
//             sawStopped = true;

//         if (state == "Waiting")
//             sawWaiting = true;

//         if (leader->getCurrentRail() && follower->getCurrentRail())
//         {
//             double dist =
//                 leader->getPosition() - follower->getPosition();

//             if (dist > 0)
//                 minDistance = std::min(minDistance, dist);
//         }

//         printDebug(t, leader, follower);
//     }

//     cout << "\n==== TEST SUMMARY ====\n";
//     cout << "Saw Braking: " << (sawBraking ? "YES" : "NO") << endl;
//     cout << "Saw Stopped: " << (sawStopped ? "YES" : "NO") << endl;
//     cout << "Saw Waiting: " << (sawWaiting ? "YES" : "NO") << endl;
//     cout << "Minimum distance observed: " << minDistance << endl;

//     EXPECT_TRUE(sawBraking) << "Follower must enter Braking when leader stops";
//     EXPECT_TRUE(sawStopped) << "Follower must reach Stopped before Waiting";
//     EXPECT_TRUE(sawWaiting) << "Follower must eventually enter Waiting";

//     EXPECT_GT(minDistance, 80.0)
//         << "Safety distance violated: " << minDistance;

//     delete leader;
//     delete follower;
// }

// TEST_F(RealisticFlowTest, WaitingEndsWhenLeaderResumes)
// {
//     cout << "\n==== TEST: WaitingEndsWhenLeaderResumes ====\n";

//     TrainConfig leaderCfg = {
//         "Leader", 80.0, 0.005, 300.0, 400.0,
//         "CityA", "CityB",
//         Time("00h00"), Time("00h05")
//     };

//     TrainConfig followerCfg = leaderCfg;
//     followerCfg.name = "Follower";

//     Train* leader = TrainFactory::create(leaderCfg, &graph);
//     Train* follower = TrainFactory::create(followerCfg, &graph);

//     ASSERT_NE(leader, nullptr);
//     ASSERT_NE(follower, nullptr);

//     DijkstraStrategy dijkstra;

//     leader->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
//     follower->setPath(dijkstra.findPath(&graph, nodeA, nodeB));

//     follower->setPosition(-200.0);

//     static IdleState idle;
//     leader->setState(&idle);
//     follower->setState(&idle);

//     SimulationManager::getInstance().addTrain(leader);
//     SimulationManager::getInstance().addTrain(follower);

//     SimulationManager::getInstance().start();

//     bool enteredWaiting = false;
//     bool leftWaiting = false;

//     int firstWaitingT = -1;
//     int firstLeaveWaitingT = -1;

//     for (int t = 0; t < 300; ++t)
//     {
//         if (t == 50)
//         {
//             cout << "\n>>> EVENT: LEADER FORCED TO STOP (t=50..119) <<<\n";
//         }

//         // Leader stops for a while
//         if (t >= 50 && t < 120)
//         {
//             leader->setVelocity(0.0);
//         }

//         SimulationManager::getInstance().step();

//         const std::string state = follower->getCurrentState()->getName();

//         if (!enteredWaiting && state == "Waiting")
//         {
//             enteredWaiting = true;
//             firstWaitingT = t;
//             cout << "\n>>> EVENT: FOLLOWER ENTERED WAITING at t=" << t << " <<<\n";
//         }

//         if (enteredWaiting && !leftWaiting && state == "Accelerating")
//         {
//             leftWaiting = true;
//             firstLeaveWaitingT = t;
//             cout << "\n>>> EVENT: FOLLOWER LEFT WAITING (Accelerating) at t=" << t << " <<<\n";
//         }

//         printDebug(t, leader, follower);
//     }

//     cout << "\n==== TEST SUMMARY ====\n";
//     cout << "Entered Waiting: " << (enteredWaiting ? "YES" : "NO") << endl;
//     cout << "Left Waiting: " << (leftWaiting ? "YES" : "NO") << endl;
//     cout << "First Waiting t: " << firstWaitingT << endl;
//     cout << "First Leave Waiting t: " << firstLeaveWaitingT << endl;

//     EXPECT_TRUE(enteredWaiting) << "Follower must enter Waiting";
//     EXPECT_TRUE(leftWaiting) << "Follower must leave Waiting when leader moves";

//     delete leader;
//     delete follower;
// }


// TEST_F(RealisticFlowTest, MustRespectStateOrder)
// {
//     cout << "\n==== TEST: MustRespectStateOrder ====\n";

//     TrainConfig cfg = {
//         "TrainA", 80.0, 0.005, 300.0, 400.0,
//         "CityA", "CityB",
//         Time("00h00"), Time("00h05")
//     };

//     Train* leader = TrainFactory::create(cfg, &graph);
//     Train* follower = TrainFactory::create(cfg, &graph);

//     ASSERT_NE(leader, nullptr);
//     ASSERT_NE(follower, nullptr);

//     DijkstraStrategy dijkstra;

//     leader->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
//     follower->setPath(dijkstra.findPath(&graph, nodeA, nodeB));

//     follower->setPosition(-150.0);

//     static IdleState idle;
//     leader->setState(&idle);
//     follower->setState(&idle);

//     SimulationManager::getInstance().addTrain(leader);
//     SimulationManager::getInstance().addTrain(follower);

//     SimulationManager::getInstance().start();

//     bool sawBraking = false;
//     bool sawStoppedAfterBraking = false;

//     for (int t = 0; t < 200; ++t)
//     {
//         if (t == 40)
//             cout << "\n>>> EVENT: LEADER STOPS <<<\n";

//         if (t > 40)
//             leader->setVelocity(0.0);

//         SimulationManager::getInstance().step();

//         std::string state = follower->getCurrentState()->getName();

//         if (state == "Braking")
//             sawBraking = true;

//         if (sawBraking && state == "Stopped")
//             sawStoppedAfterBraking = true;

//         printDebug(t, leader, follower);
//     }

//     cout << "\n==== TEST SUMMARY ====\n";
//     cout << "Saw Braking: " << (sawBraking ? "YES" : "NO") << endl;
//     cout << "Reached Stopped After Braking: "
//          << (sawStoppedAfterBraking ? "YES" : "NO") << endl;

//     EXPECT_TRUE(sawBraking)
//         << "Must brake first";

//     EXPECT_TRUE(sawStoppedAfterBraking)
//         << "Must reach Stopped after Braking";

//     delete leader;
//     delete follower;
// }


// TEST_F(RealisticFlowTest, NeverViolatesSafetyDistance)
// {
//     cout << "\n==== TEST: NeverViolatesSafetyDistance ====\n";

//     TrainConfig cfg = {
//         "TrainA", 80.0, 0.005, 300.0, 400.0,
//         "CityA", "CityB",
//         Time("00h00"), Time("00h05")
//     };

//     Train* leader = TrainFactory::create(cfg, &graph);
//     Train* follower = TrainFactory::create(cfg, &graph);

//     ASSERT_NE(leader, nullptr);
//     ASSERT_NE(follower, nullptr);

//     DijkstraStrategy dijkstra;

//     leader->setPath(dijkstra.findPath(&graph, nodeA, nodeB));
//     follower->setPath(dijkstra.findPath(&graph, nodeA, nodeB));

//     follower->setPosition(-100.0);

//     static IdleState idle;
//     leader->setState(&idle);
//     follower->setState(&idle);

//     SimulationManager::getInstance().addTrain(leader);
//     SimulationManager::getInstance().addTrain(follower);

//     SimulationManager::getInstance().start();

//     double minDistance = 1e9;

//     for (int t = 0; t < 300; ++t)
//     {
//         if (t == 81)
//         {
//             cout << "\n>>> EVENT: LEADER FORCED TO STOP (t>80) <<<\n";
//         }

//         if (t > 80)
//         {
//             leader->setVelocity(0.0);
//         }

//         SimulationManager::getInstance().step();

//         double dist =
//             leader->getPosition() - follower->getPosition();

//         if (dist > 0)
//         {
//             minDistance = std::min(minDistance, dist);
//         }

//         printDebug(t, leader, follower);
//     }

//     cout << "\n==== TEST SUMMARY ====\n";
//     cout << "Minimum distance observed: " << minDistance << endl;

//     EXPECT_GT(minDistance, 50.0)
//         << "Trains came too close: " << minDistance;

//     delete leader;
//     delete follower;
// }



