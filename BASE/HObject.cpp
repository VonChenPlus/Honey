#include <string.h>
#include "HObject.h"
#include "AutoreleasePool.h"

HObject::HObject()
    : referenceCount_(1) {

}

HObject::~HObject() {

}

void HObject::retain() {
    ++referenceCount_;
}

void HObject::release() {
    --referenceCount_;

    if (referenceCount_ == 0) {
        delete this;
    }
}

HObject *HObject::autorelease() {
    PoolManager::getInstance().getCurrentPool()->addObject(this);
    return this;
}

uint32 HObject::getReferenceCount() const {
    return referenceCount_;
}

HObjectArray::HObjectArray(uint64 capacity) {
    if (capacity == 0)
        capacity = 7;

    number_ = 0;
    array_ =  (HObject**)calloc(capacity, sizeof(HObject*));
    maximum_ = capacity;
}

HObjectArray::~HObjectArray() {
    if( array_ == nullptr ) {
        return;
    }
    removeAllObjects();
    free(array_);
}

void HObjectArray::doubleCapacity() {
    maximum_ *= 2;
    array_ = (HObject**)realloc( array_, maximum_ * sizeof(HObject*) );
}

void HObjectArray::ensureExtraCapacity(uint64 extra)
{
    while (maximum_ < maximum_ + extra) {
        doubleCapacity();
    }
}

void HObjectArray::shrink()
{
    uint64 newSize = 0;

    //only resize when necessary
    if (maximum_ > number_ && !(number_ == 0 && maximum_ == 1)) {
        if (number_!=0) {
            newSize = number_;
            maximum_ = number_;
        }
        else {
            //minimum capacity of 1, with 0 elements the array would be free'd by realloc
            newSize = 1;
            maximum_ = 1;
        }

        array_ = (HObject**)realloc(array_,newSize * sizeof(HObject*) );
    }
}

uint64 HObjectArray::getIndexOfObject(HObject* object)
{
    const auto arrNum = number_;
    HObject** ptr = array_;
    for (uint64 i = 0; i < arrNum; ++i, ++ptr) {
        if (*ptr == object)
            return i;
    }

    return -1;
}

bool HObjectArray::containsObject(HObject* object) {
    return getIndexOfObject(object) != -1;
}

void HObjectArray::appendObject(HObject* object) {
    object->retain();
    array_[number_] = object;
    number_++;
}

void HObjectArray::appendObjectWithResize(HObject* object) {
    ensureExtraCapacity(1);
    appendObject(object);
}

void HObjectArray::appendArray(HObjectArray *plusArr)
{
    for (uint64 i = 0; i < plusArr->number_; i++) {
        appendObject((*plusArr)[i]);
    }
}

void HObjectArray::appendArrayWithResize(HObjectArray *plusArr) {
    ensureExtraCapacity(plusArr->number_);
    appendArray(plusArr);
}

void HObjectArray::insertObjectAtIndex(HObject* object, uint64 index)
{
    ensureExtraCapacity(1);

    uint64 remaining = number_ - index;
    if (remaining > 0) {
        memmove((void *)&array_[index+1], (void *)&array_[index], sizeof(HObject*) * remaining );
    }

    object->retain();
    array_[index] = object;
    number_++;
}

void HObjectArray::swapObjectsAtIndexes(uint64 index1, uint64 index2) {
    HObject* object1 = array_[index1];
    array_[index1] = array_[index2];
    array_[index2] = object1;
}

void HObjectArray::fastRemoveObjectAtIndex(uint64 index) {
    SAFE_RELEASE(array_[index]);
    auto last = --number_;
    array_[index] = array_[last];
}

void HObjectArray::fastRemoveObject(HObject* object) {
    auto index = getIndexOfObject(object);
    if (index != -1) {
        fastRemoveObjectAtIndex(index);
    }
}

void HObjectArray::removeAllObjects()
{
    while (number_ > 0) {
        (array_[--number_])->release();
    }
}

void HObjectArray::removeObjectAtIndex(uint64 index, bool releaseObj/* = true*/) {
    if (releaseObj) {
        SAFE_RELEASE(array_[index]);
    }

    number_--;

    uint64 remaining = number_ - index;
    if(remaining>0) {
        memmove((void *)&array_[index], (void *)&array_[index+1], remaining * sizeof(HObject*));
    }
}

void HObjectArray::removeObject(HObject* object, bool releaseObj/* = true*/) {
    auto index = getIndexOfObject(object);
    if (index != -1) {
        removeObjectAtIndex(index, releaseObj);
    }
}

void HObjectArray::removeArray(HObjectArray *minusArr) {
    for (uint64 i = 0; i < minusArr->number_; i++) {
        removeObject((*minusArr)[i]);
    }
}

void HObjectArray::fullRemoveArray(HObjectArray *minusArr) {
    uint64 back = 0;

    for (uint64 i = 0; i < number_; i++) {
        if (minusArr->containsObject(array_[i])) {
            SAFE_RELEASE(array_[i]);
            back++;
        }
        else {
            array_[i - back] = array_[i];
        }
    }

    number_ -= back;
}
