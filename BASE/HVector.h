#ifndef HVECTOR_H
#define HVECTOR_H

#include <vector>
#include <functional>
#include <algorithm> // for std::find
#include "BASE/HObject.h"

template<class T>
class HVector
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
    
    HVector<T>()
        : data_() {
        static_assert(std::is_convertible<T, HObject*>::value, "Invalid Type for HVector<T>!");
    }
    
    explicit HVector<T>(ssize_t capacity)
        : data_() {
        static_assert(std::is_convertible<T, HObject*>::value, "Invalid Type for HVector<T>!");
        reserve(capacity);
    }

    ~HVector<T>() {
        clear();
    }

    HVector<T>(const HVector<T>& other) {
        static_assert(std::is_convertible<T, HObject*>::value, "Invalid Type for HVector<T>!");
        data_ = other.data_;
        addRefForAllObjects();
    }
    
    HVector<T>(HVector<T>&& other)
    {
        static_assert(std::is_convertible<T, Ref*>::value, "Invalid Type for cocos2d::HVector<T>!");
        data_ = std::move(other.data_);
    }

    HVector<T>& operator=(const HVector<T>& other) {
        if (this != &other) {
            clear();
            data_ = other.data_;
            addRefForAllObjects();
        }
        return *this;
    }
    
    HVector<T>& operator=(HVector<T>&& other) {
        if (this != &other) {
            clear();
            data_ = std::move(other.data_);
        }
        return *this;
    }
    
    void reserve(ssize_t n) {
        data_.reserve(n);
    }
    
    ssize_t capacity() const {
        return data_.capacity();
    }

    ssize_t size() const {
        return  data_.size();
    }
    
    bool empty() const {
        return data_.empty();
    }
    
    ssize_t max_size() const {
        return data_.max_size();
    }
    
    ssize_t getIndex(T object) const {
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

    T at(ssize_t index) const {
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
            ssize_t randIdx = rand() % data_.size();
            return *(data_.begin() + randIdx);
        }
        return nullptr;
    }

    bool contains(T object) const {
        return( std::find(data_.begin(), data_.end(), object) != data_.end() );
    }

    bool equals(const HVector<T> &other) {
        ssize_t s = this->size();
        if (s != other.size())
            return false;
        
        for (ssize_t i = 0; i < s; i++)
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
    
    void pushBack(const HVector<T>& other) {
        for(const auto &obj : other) {
            data_.push_back(obj);
            obj->retain();
        }
    }

    void insert(ssize_t index, T object) {
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
    
    iterator erase(ssize_t index) {
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
        ssize_t idx1 = getIndex(object1);
        ssize_t idx2 = getIndex(object2);

        std::swap( data_[idx1], data_[idx2] );
    }
    
    void swap(ssize_t index1, ssize_t index2) {
        std::swap( data_[index1], data_[index2] );
    }

    void replace(ssize_t index, T object) {
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

#endif // HVECTOR_H
