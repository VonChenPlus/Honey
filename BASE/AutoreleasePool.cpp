#include "AutoreleasePool.h"

#define DEFAULTPOOLOBJECTCOUNT 150
#define DEFAULTPOOLCOUNT 10

AutoreleasePool::AutoreleasePool()
    : poolName_("") {
    managerObjectArray_.reserve(DEFAULTPOOLOBJECTCOUNT);
    PoolManager::getInstance().push(this);
}

AutoreleasePool::AutoreleasePool(std::string poolName)
    : poolName_(poolName) {
    managerObjectArray_.reserve(DEFAULTPOOLOBJECTCOUNT);
    PoolManager::getInstance().push(this);
}

AutoreleasePool::~AutoreleasePool() {
    clear();
    PoolManager::getInstance().pop();
}

void AutoreleasePool::addObject(HObject *object) {
    managerObjectArray_.push_back(object);
}

void AutoreleasePool::clear() {
    std::vector<HObject *> releasings;
    releasings.swap(managerObjectArray_);
    for (const auto &object : releasings) {
        object->release();
    }
}

bool AutoreleasePool::contains(HObject *object) const {
    for (const auto &_object : managerObjectArray_) {
        if (_object == object) {
            return true;
        }
    }

    return false;
}

PoolManager &PoolManager::getInstance() {
    static PoolManager instance;
    return instance;
}

PoolManager::PoolManager() {
    releasePoolStack_.reserve(DEFAULTPOOLCOUNT);
}

PoolManager::~PoolManager() {
    while (!releasePoolStack_.empty()) {
        AutoreleasePool *pool = releasePoolStack_.back();
        delete pool;
        releasePoolStack_.pop_back();
    }
}

AutoreleasePool* PoolManager::getCurrentPool() const {
    return releasePoolStack_.back();
}

bool PoolManager::isObjectInPools(HObject* obj) const {
    for (const auto& pool : releasePoolStack_) {
        if (pool->contains(obj))
            return true;
    }
    return false;
}

void PoolManager::push(AutoreleasePool *pool) {
    releasePoolStack_.push_back(pool);
}

void PoolManager::pop() {
    releasePoolStack_.pop_back();
}
