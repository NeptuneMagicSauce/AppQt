#include "LoadContent.hpp"

#include <stdexcept>
#include <vector>

class LoadContentImpl
{
public:
    std::vector<LoadContent*> instances;
    bool loaded { false };
} impl;


LoadContent::LoadContent()
{
    if (impl.loaded)
    {
        throw std::runtime_error(" too late");
    }
    impl.instances.emplace_back(this);
}

void LoadContent::doLoad(void)
{
    for (auto* i: impl.instances)
    {
        i->loadCallback();
    }
    impl.loaded = true;
}
