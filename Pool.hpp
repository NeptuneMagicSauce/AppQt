#pragma once

#include <vector>
#include "Utils.hpp"

namespace Utils
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
                instances.resize((instances.size() * 3) / 2);
                for (unsigned int i=index; i<instances.size(); ++i)
                {
                    instances[i] = new T;
                }
            }
            Assert(index < instances.size());
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
