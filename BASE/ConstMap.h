#ifndef CONSTMAP_H
#define CONSTMAP_H

#include <map>

template <typename T, typename U>
class ConstMap
{
private:
    std::map<T, U> map_;
public:
    ConstMap(const T& key, const U& val)
    {
        map_[key] = val;
    }

    ConstMap<T, U>& operator()(const T& key, const U& val)
    {
        map_[key] = val;
        return *this;
    }

    operator std::map<T, U>()
    {
        return map_;
    }
};

#endif // CONSTMAP_H

