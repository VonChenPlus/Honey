#ifndef HAUTORELEASEPOOL_H
#define HAUTORELEASEPOOL_H

#include <string>
#include <vector>

#include "BASE/HObject.h"

class AutoreleasePool final
{
public:
    AutoreleasePool();
    AutoreleasePool(std::string poolName);
    ~AutoreleasePool();

    void addObject(HObject *object);
    void clear();

    bool contains(HObject *object) const;

private:
    std::vector<HObject *> managerObjectArray_;
    std::string poolName_;
};

class PoolManager final
{
    static PoolManager &getInstance();

    AutoreleasePool *getCurrentPool() const;

    bool isObjectInPools(HObject *object) const;

private:
    PoolManager();
    ~PoolManager();

    void push(AutoreleasePool *pool);
    void pop();

    friend class AutoreleasePool;
private:
    std::vector<AutoreleasePool*> releasePoolStack_;
};

#endif // HAUTORELEASEPOOL_H
