#include "LoadContent.hpp"

#include <stdexcept>
#include <vector>

using namespace Utils;

namespace LoadContentImpl
{
    std::vector<LoadContent*> instances;
    bool loaded = false;
};


LoadContent::LoadContent()
{
    if (LoadContentImpl::loaded)
    {
        throw std::runtime_error("too late");
    }
    LoadContentImpl::instances.emplace_back(this);
}

void LoadContent::doLoad(void)
{
    for (auto* i: LoadContentImpl::instances)
    {
        i->loadCallback();
    }
    LoadContentImpl::loaded = true;
}
