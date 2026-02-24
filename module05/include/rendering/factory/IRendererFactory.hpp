#ifndef IRENDERERFACTORY_HPP
#define IRENDERERFACTORY_HPP

class IRenderer;
class SimulationManager;

class IRendererFactory
{
public:
    virtual ~IRendererFactory() = default;

    // Returns a heap-allocated IRenderer, or nullptr if rendering is unavailable.
    // Caller takes ownership.
    virtual IRenderer* create(SimulationManager& sim) = 0;
};

// Defined in RendererFactory_SFML.cpp or RendererFactory_Stub.cpp (CMake selects one).
IRendererFactory* createRendererFactory();

#endif