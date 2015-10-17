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
    uint64 getSize() const;

    void copy(const HBYTE* bytes, const uint64 size);
    void fastSet(HBYTE* bytes, const uint64 size);
    void clear();
    bool isNull() const;

private:
    void move(HData& other);

private:
    HBYTE* bytes_;
    uint64 size_;
};

#endif // HDATA_H
