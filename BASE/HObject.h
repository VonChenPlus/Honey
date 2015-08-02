#ifndef HOBJECT_H
#define HOBJECT_H

#include "BASE/Honey.h"

#ifndef SAFE_RELEASE
    #define SAFE_RELEASE(p) do { if(p) { (p)->release(); } } while(0)
#endif
#ifndef SAFE_RELEASE_NULL
    #define SAFE_RELEASE_NULL(p)     do { if(p) { (p)->release(); (p) = nullptr; } } while(0)
#endif
#ifndef SAFE_RETAIN
    #define SAFE_RETAIN(p)     do { if(p) { (p)->retain(); } } while(0)
#endif

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
