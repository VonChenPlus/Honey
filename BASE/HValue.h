#ifndef HVALUE_H
#define HVALUE_H

#include "BASE/Honey.h"

#include <vector>
#include <unordered_map>

class HValue;

typedef std::vector<HValue> ValueVector;
typedef std::unordered_map<std::string, HValue> ValueMap;
typedef std::unordered_map<int, HValue> ValueMapIntKey;

class HValue
{
public:
    static const HValue Null;

    HValue();

    explicit HValue(unsigned char v);
    explicit HValue(int v);
    explicit HValue(float v);
    explicit HValue(double v);
    explicit HValue(bool v);
    explicit HValue(const char* v);
    explicit HValue(const std::string& v);
    explicit HValue(const ValueVector& v);
    explicit HValue(ValueVector&& v);
    explicit HValue(const ValueMap& v);
    explicit HValue(ValueMap&& v);
    explicit HValue(const ValueMapIntKey& v);
    explicit HValue(ValueMapIntKey&& v);
    HValue(const HValue& other);
    HValue(HValue&& other);
    ~HValue();

    HValue& operator= (const HValue& other);
    HValue& operator= (HValue&& other);
    HValue& operator= (unsigned char v);
    HValue& operator= (int v);
    HValue& operator= (float v);
    HValue& operator= (double v);
    HValue& operator= (bool v);
    HValue& operator= (const char* v);
    HValue& operator= (const std::string& v);
    HValue& operator= (const ValueVector& v);
    HValue& operator= (ValueVector&& v);
    HValue& operator= (const ValueMap& v);
    HValue& operator= (ValueMap&& v);
    HValue& operator= (const ValueMapIntKey& v);
    HValue& operator= (ValueMapIntKey&& v);

    bool operator!= (const HValue& v);
    bool operator!= (const HValue& v) const;
    bool operator== (const HValue& v);
    bool operator== (const HValue& v) const;

    unsigned char asByte() const;
    int asInt() const;
    float asFloat() const;
    double asDouble() const;
    bool asBool() const;
    std::string asString() const;

    ValueVector& asValueVector();
    const ValueVector& asValueVector() const;

    ValueMap& asValueMap();
    const ValueMap& asValueMap() const;

    ValueMapIntKey& asIntKeyMap();
    const ValueMapIntKey& asIntKeyMap() const;

    inline bool isNull() const { return type_ == Type::NONE; }

    enum class Type
    {
        /// no value is wrapped, an empty Value
        NONE = 0,
        /// wrap byte
        BYTE,
        /// wrap integer
        INTEGER,
        /// wrap float
        FLOAT,
        /// wrap double
        DOUBLE,
        /// wrap bool
        BOOLEAN,
        /// wrap string
        STRING,
        /// wrap vector
        VECTOR,
        /// wrap ValueMap
        MAP,
        /// wrap ValueMapIntKey
        INT_KEY_MAP
    };

    inline Type getType() const { return type_; }

private:
    void clear();
    void reset(Type type);

    union
    {
        unsigned char byteVal;
        int intVal;
        float floatVal;
        double doubleVal;
        bool boolVal;

        std::string* strVal;
        ValueVector* vectorVal;
        ValueMap* mapVal;
        ValueMapIntKey* intKeyMapVal;
    }_field;

    Type type_;
};

#endif // HVALUE_H
