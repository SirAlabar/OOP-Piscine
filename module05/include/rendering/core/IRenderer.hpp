#ifndef IRENDERER_HPP
#define IRENDERER_HPP

class SimulationManager;

class IRenderer
{
public:
	virtual ~IRenderer() = default;
	virtual void initialize(SimulationManager& simulation) = 0;
	virtual bool processFrame(SimulationManager& simulation) = 0;
	virtual void shutdown() = 0;
};

#endif
