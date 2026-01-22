#include "ATool.hpp"

ATool::ATool() : numberOfUses(0), currentOwner(0)
{
}

ATool::~ATool()
{
}

Worker* ATool::getOwner() const
{
	return currentOwner;
}

void ATool::setOwner(Worker* owner)
{
	currentOwner = owner;
}