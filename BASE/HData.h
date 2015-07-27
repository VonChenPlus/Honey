#ifndef HDATA_H
#define HDATA_H

#include "BASE/Honey.h"

class HData
{
public:
    static const HData Null;

    HData();
    HData(const HData& other);
    HData(HData&& other);
    ~HData();

    HData& operator= (const HData& other);
    HData& operator= (HData&& other);

    HBYTE* getBytes() const;
    size_t getSize() const;

    void copy(const HBYTE* bytes, const size_t size);
    void fastSet(HBYTE* bytes, const size_t size);
    void clear();
    bool isNull() const;

private:
    void move(HData& other);

private:
    HBYTE* bytes_;
    size_t size_;
};

#endif // HDATA_H
