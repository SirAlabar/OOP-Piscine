#include "rendering/factory/IRendererFactory.hpp"
#include <iostream>

class RendererFactory_Stub : public IRendererFactory
{
public:
    IRenderer* create(SimulationManager& /*sim*/) override
    {
        std::cerr << "Render mode requested but binary was built without SFML.\n";
        return nullptr;
    }
};

IRendererFactory* createRendererFactory()
{
    return new RendererFactory_Stub();
}