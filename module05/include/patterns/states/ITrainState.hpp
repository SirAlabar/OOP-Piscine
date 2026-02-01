#ifndef ITRAINSTATE_HPP
#define ITRAINSTATE_HPP

#include <string>

class Train;

class ITrainState
{
public:
	virtual ~ITrainState() = default;
	
	virtual void update(Train* train, double dt) = 0;
	virtual std::string getName() const = 0;
};

#endif