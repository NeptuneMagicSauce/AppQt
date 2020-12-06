#pragma once

#include <vector>

namespace Minus
{
    template<class T> class Pool
    {
    public:
        void reserve(int count)
        {
            instances.resize(count);
            for (auto& i: instances)
            {
                i = new T;
            }
        }
        auto* get(void)
        {
            if (index >= instances.size())
            {
                // TODO (size*3)/2
                instances.resize(instances.size() + instances.size() / 2);
                for (unsigned int i=index; i<instances.size(); ++i)
                {
                    instances[i] = new T;
                }
            }
            assert(index < instances.size());
            return instances[index++];
        }
        void reset(void)
        {
            index = 0;
        }

    private:
        std::vector<T*> instances;
        unsigned int index = 0;
    };
};
