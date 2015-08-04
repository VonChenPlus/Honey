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

class HObjectArray
{
public:
    HObjectArray(int64 capacity);
    ~HObjectArray();

    void doubleCapacity();
    void ensureExtraCapacity(int64 extra);
    void shrink();
    int64 getIndexOfObject(HObject* object);
    bool containsObject(HObject* object);

    void appendObject(HObject* object);
    void appendObjectWithResize(HObject* object);
    void appendArray(HObjectArray *plusArr);
    void appendArrayWithResize(HObjectArray *plusArr);

    void insertObjectAtIndex(HObject* object, int64 index);
    void swapObjectsAtIndexes(int64 index1, int64 index2);

    void fastRemoveObjectAtIndex(int64 index);
    void fastRemoveObject(HObject* object);

    void removeAllObjects();
    void removeObjectAtIndex(int64 index, bool releaseObj = true);
    void removeObject(HObject* object, bool releaseObj = true);
    void removeArray(HObjectArray *minusArr);
    void fullRemoveArray(HObjectArray *minusArr);

    HObject *operator[](int index) {
        return array_[index];
    }

    int64 number() { return number_; }
    int64 maximun() { return maximum_; }

private:
    int64 number_;
    int64 maximum_;
    HObject** array_;
};

#endif // HOBJECT_H
