#ifndef HOBJECT_H
#define HOBJECT_H

#include <vector>
#include <unordered_map>
#include <algorithm>
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
    HObject *autorelease();

    uint32 getReferenceCount() const;

protected:
    HObject();

private:
    uint32 referenceCount_;
};

typedef void (HObject::*SelectorV)();
typedef void (HObject::*SelectorO)(HObject*);
typedef void (HObject::*SelectorF)(float);

class HObjectArray
{
public:
    HObjectArray(uint64 capacity);
    ~HObjectArray();

    void doubleCapacity();
    void ensureExtraCapacity(uint64 extra);
    void shrink();
    uint64 getIndexOfObject(HObject* object);
    bool containsObject(HObject* object);

    void appendObject(HObject* object);
    void appendObjectWithResize(HObject* object);
    void appendArray(HObjectArray *plusArr);
    void appendArrayWithResize(HObjectArray *plusArr);

    void insertObjectAtIndex(HObject* object, uint64 index);
    void swapObjectsAtIndexes(uint64 index1, uint64 index2);

    void fastRemoveObjectAtIndex(uint64 index);
    void fastRemoveObject(HObject* object);

    void removeAllObjects();
    void removeObjectAtIndex(uint64 index, bool releaseObj = true);
    void removeObject(HObject* object, bool releaseObj = true);
    void removeArray(HObjectArray *minusArr);
    void fullRemoveArray(HObjectArray *minusArr);

    HObject *operator[](int index) {
        return array_[index];
    }

    uint64 number() { return number_; }
    uint64 maximun() { return maximum_; }

private:
    uint64 number_;
    uint64 maximum_;
    HObject** array_;
};

template<class T>
class HObjectVector
{
public:
    typedef typename std::vector<T>::iterator iterator;
    typedef typename std::vector<T>::const_iterator const_iterator;

    typedef typename std::vector<T>::reverse_iterator reverse_iterator;
    typedef typename std::vector<T>::const_reverse_iterator const_reverse_iterator;

    iterator begin() { return data_.begin(); }
    const_iterator begin() const { return data_.begin(); }
    iterator end() { return data_.end(); }
    const_iterator end() const { return data_.end(); }

    const_iterator cbegin() const { return data_.cbegin(); }
    const_iterator cend() const { return data_.cend(); }

    reverse_iterator rbegin() { return data_.rbegin(); }
    const_reverse_iterator rbegin() const { return data_.rbegin(); }

    reverse_iterator rend() { return data_.rend(); }
    const_reverse_iterator rend() const { return data_.rend(); }

    const_reverse_iterator crbegin() const { return data_.crbegin(); }
    const_reverse_iterator crend() const { return data_.crend(); }

    HObjectVector<T>()
        : data_() {        
    }

    explicit HObjectVector<T>(uint64 capacity)
        : data_() {
        reserve(capacity);
    }

    ~HObjectVector<T>() {
        clear();
    }

    HObjectVector<T>(const HObjectVector<T>& other) {
        data_ = other.data_;
        addRefForAllObjects();
    }

    HObjectVector<T>(HObjectVector<T>&& other) {
        data_ = std::move(other.data_);
    }

    HObjectVector<T>& operator=(const HObjectVector<T>& other) {
        if (this != &other) {
            clear();
            data_ = other.data_;
            addRefForAllObjects();
        }
        return *this;
    }

    HObjectVector<T>& operator=(HObjectVector<T>&& other) {
        if (this != &other) {
            clear();
            data_ = std::move(other.data_);
        }
        return *this;
    }

    void reserve(uint64 n) {
        data_.reserve(n);
    }

    uint64 capacity() const {
        return data_.capacity();
    }

    uint64 size() const {
        return  data_.size();
    }

    bool empty() const {
        return data_.empty();
    }

    uint64 max_size() const {
        return data_.max_size();
    }

    uint64 getIndex(T object) const {
        auto iter = std::find(data_.begin(), data_.end(), object);
        if (iter != data_.end())
            return iter - data_.begin();

        return -1;
    }

    const_iterator find(T object) const {
        return std::find(data_.begin(), data_.end(), object);
    }

    iterator find(T object) {
        return std::find(data_.begin(), data_.end(), object);
    }

    T at(uint64 index) const {
        return data_[index];
    }

    T front() const {
        return data_.front();
    }

    T back() const {
        return data_.back();
    }

    T getRandomObject() const {
        if (!data_.empty())
        {
            uint64 randIdx = rand() % data_.size();
            return *(data_.begin() + randIdx);
        }
        return nullptr;
    }

    bool contains(T object) const {
        return( std::find(data_.begin(), data_.end(), object) != data_.end() );
    }

    bool equals(const HObjectVector<T> &other) {
        uint64 s = this->size();
        if (s != other.size())
            return false;

        for (uint64 i = 0; i < s; i++)
        {
            if (this->at(i) != other.at(i))
            {
                return false;
            }
        }
        return true;
    }


    void pushBack(T object) {
        data_.push_back( object );
        object->retain();
    }

    void pushBack(const HObjectVector<T>& other) {
        for(const auto &obj : other) {
            data_.push_back(obj);
            obj->retain();
        }
    }

    void insert(uint64 index, T object) {
        data_.insert((std::begin(data_) + index), object);
        object->retain();
    }

    void popBack() {
        auto last = data_.back();
        data_.pop_back();
        last->release();
    }

    void eraseObject(T object, bool removeAll = false) {
        if (removeAll) {
            for (auto iter = data_.begin(); iter != data_.end();) {
                if ((*iter) == object) {
                    iter = data_.erase(iter);
                    object->release();
                }
                else {
                    ++iter;
                }
            }
        }
        else {
            auto iter = std::find(data_.begin(), data_.end(), object);
            if (iter != data_.end()) {
                data_.erase(iter);
                object->release();
            }
        }
    }

    iterator erase(iterator position) {
        (*position)->release();
        return data_.erase(position);
    }

    iterator erase(iterator first, iterator last) {
        for (auto iter = first; iter != last; ++iter) {
            (*iter)->release();
        }

        return data_.erase(first, last);
    }

    iterator erase(uint64 index) {
        auto it = std::next( begin(), index );
        (*it)->release();
        return data_.erase(it);
    }

    void clear() {
        for( auto it = std::begin(data_); it != std::end(data_); ++it ) {
            (*it)->release();
        }
        data_.clear();
    }

    void swap(T object1, T object2) {
        uint64 idx1 = getIndex(object1);
        uint64 idx2 = getIndex(object2);

        std::swap( data_[idx1], data_[idx2] );
    }

    void swap(uint64 index1, uint64 index2) {
        std::swap( data_[index1], data_[index2] );
    }

    void replace(uint64 index, T object) {
        data_[index]->release();
        data_[index] = object;
        object->retain();
    }

    void reverse() {
        std::reverse( std::begin(data_), std::end(data_) );
    }

    void shrinkToFit() {
        data_.shrink_to_fit();
    }

protected:
    void addRefForAllObjects() {
        for(const auto &obj : data_) {
            obj->retain();
        }
    }

    std::vector<T> data_;
};

template <class K, class V>
class HObjectMap
{
public:
    typedef std::unordered_map<K, V> RefMap;

    typedef typename RefMap::iterator iterator;
    typedef typename RefMap::const_iterator const_iterator;

    iterator begin() { return data_.begin(); }
    const_iterator begin() const { return data_.begin(); }

    iterator end() { return data_.end(); }
    const_iterator end() const { return data_.end(); }

    const_iterator cbegin() const { return data_.cbegin(); }
    const_iterator cend() const { return data_.cend(); }

    HObjectMap<K, V>()
        : data_() {
    }

    explicit HObjectMap<K, V>(uint64 capacity)
        : data_() {
        data_.reserve(capacity);
    }

    HObjectMap<K, V>(const HObjectMap<K, V>& other)
    {
        data_ = other.data_;
        addRefForAllObjects();
    }

    HObjectMap<K, V>(HObjectMap<K, V>&& other) {
        data_ = std::move(other.data_);
    }

    ~HObjectMap<K, V>() {
        clear();
    }

    void reserve(uint64 capacity) {
        data_.reserve(capacity);
    }

    uint64 bucketCount() const {
        return data_.bucket_count();
    }

    uint64 bucketSize(uint64 n) const {
        return data_.bucket_size(n);
    }

    uint64 bucket(const K& k) const {
        return data_.bucket(k);
    }

    uint64 size() const {
        return data_.size();
    }

    bool empty() const {
        return data_.empty();
    }

    std::vector<K> keys() const {
        std::vector<K> keys;

        if (!data_.empty()) {
            keys.reserve(data_.size());

            for (auto iter = data_.cbegin(); iter != data_.cend(); ++iter) {
                keys.push_back(iter->first);
            }
        }
        return keys;
    }

    std::vector<K> keys(V object) const {
        std::vector<K> keys;

        if (!data_.empty()) {
            keys.reserve(data_.size() / 10);

            for (auto iter = data_.cbegin(); iter != data_.cend(); ++iter) {
                if (iter->second == object) {
                    keys.push_back(iter->first);
                }
            }
        }

        keys.shrink_to_fit();

        return keys;
    }

    const V at(const K& key) const {
        auto iter = data_.find(key);
        if (iter != data_.end())
            return iter->second;
        return nullptr;
    }

    V at(const K& key) {
        auto iter = data_.find(key);
        if (iter != data_.end())
            return iter->second;
        return nullptr;
    }

    const_iterator find(const K& key) const {
        return data_.find(key);
    }

    iterator find(const K& key) {
        return data_.find(key);
    }

    void insert(const K& key, V object) {
        erase(key);
        data_.insert(std::make_pair(key, object));
        object->retain();
    }

    iterator erase(const_iterator position) {
        position->second->release();
        return data_.erase(position);
    }

    uint64 erase(const K& k) {
        auto iter = data_.find(k);
        if (iter != data_.end())
        {
            iter->second->release();
            data_.erase(iter);
            return 1;
        }

        return 0;
    }

    void erase(const std::vector<K>& keys) {
        for(const auto &key : keys) {
            this->erase(key);
        }
    }

    void clear() {
        for (auto iter = data_.cbegin(); iter != data_.cend(); ++iter) {
            iter->second->release();
        }

        data_.clear();
    }

    V getRandomObject() const {
        if (!data_.empty())
        {
            uint64 randIdx = rand() % data_.size();
            const_iterator randIter = data_.begin();
            std::advance(randIter , randIdx);
            return randIter->second;
        }
        return nullptr;
    }

    HObjectMap<K, V>& operator= ( const HObjectMap<K, V>& other ) {
        if (this != &other) {
            clear();
            data_ = other.data_;
            addRefForAllObjects();
        }
        return *this;
    }

    HObjectMap<K, V>& operator= ( HObjectMap<K, V>&& other ) {
        if (this != &other) {
            clear();
            data_ = std::move(other.data_);
        }
        return *this;
    }

protected:
    void addRefForAllObjects() {
        for (auto iter = data_.begin(); iter != data_.end(); ++iter) {
            iter->second->retain();
        }
    }

    RefMap data_;
};

#endif // HOBJECT_H
