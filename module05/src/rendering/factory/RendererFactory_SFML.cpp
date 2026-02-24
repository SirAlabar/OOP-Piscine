#include "rendering/factory/IRendererFactory.hpp"
#include "rendering/graphics/SFMLRenderer.hpp"

class RendererFactory_SFML : public IRendererFactory
{
public:
    IRenderer* create(SimulationManager& /*sim*/) override
    {
        return new SFMLRenderer();
    }
};

IRendererFactory* createRendererFactory()
{
    return new RendererFactory_SFML();
}