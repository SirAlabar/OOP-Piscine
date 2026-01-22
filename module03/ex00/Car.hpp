#ifndef CAR_HPP
#define CAR_HPP

#include "Engine.hpp"
#include "Transmission.hpp"
#include "Steering.hpp"
#include "Brakes.hpp"

class Car
{
private:
    Engine engine;
    Transmission transmission;
    Steering steering;
    Brakes brakes;

public:
    void start()
    {
        engine.start();
    }

    void stop()
    {
        engine.stop();
        brakes.applyForce(50, engine);
    }

    void accelerate(int speed)
    {
        engine.accelerate(speed);
    }

    void shift_gears_up()
    {
        transmission.shiftUp();
    }

    void shift_gears_down()
    {
        transmission.shiftDown();
    }

    void reverse()
    {
        transmission.engageReverse();
    }

    void turn_wheel(int angle)
    {
        steering.turnWheel(angle);
    }

    void straighten_wheels()
    {
        steering.straighten();
    }

    void apply_force_on_brakes(int force)
    {
        brakes.applyForce(force, engine);
    }

    void apply_emergency_brakes()
    {
        brakes.applyEmergencyBrakes(engine);
    }
};

#endif