#pragma once

class LoadContent
{
public:
    LoadContent();
    static void doLoad(void);
protected:
    virtual void loadCallback(void) { }
};
