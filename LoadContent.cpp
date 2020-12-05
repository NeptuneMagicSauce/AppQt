#include "LoadContent.hpp"

#include <stdexcept>
#include <vector>

class LoadContentImpl
{
public:
    std::vector<LoadContent*> instances;
    bool loaded = false;
} impl_lc;


LoadContent::LoadContent()
{
    if (impl_lc.loaded)
    {
        throw std::runtime_error(" too late");
    }
    impl_lc.instances.emplace_back(this);
}

void LoadContent::doLoad(void)
{
    for (auto* i: impl_lc.instances)
    {
        i->loadCallback();
    }
    impl_lc.loaded = true;
}
