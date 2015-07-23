#ifndef HOBJECT_H
#define HOBJECT_H

#include "BASE/Honey.h"

class HObject
{
public:
    virtual ~HObject();

    void retain();
    void release();
    void autorelease();

    uint32 getReferenceCount();

protected:
    HObject();

private:
    uint32 referenceCount_;
};

#endif // HOBJECT_H
