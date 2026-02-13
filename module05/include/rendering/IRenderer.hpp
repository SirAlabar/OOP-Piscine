#ifndef IRENDERER_HPP
#define IRENDERER_HPP

class SimulationManager;

class IRenderer
{
public:
	virtual ~IRenderer() = default;
	virtual void run(SimulationManager& simulation) = 0;
};

#endif