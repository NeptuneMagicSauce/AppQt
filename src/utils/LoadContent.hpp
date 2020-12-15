#pragma once

namespace Utils
{
    class LoadContent
    {
    public:
        LoadContent();
        static void doLoad(void);
    protected:
        virtual void loadCallback(void) { }
    };
}
