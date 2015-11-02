#include "HValue.h"

#include <string.h>
#include <sstream>
#include <iomanip>

#include "MATH/MathDefine.h"

const HValue HValue::Null;

HValue::HValue()
    : type_(Type::NONE) {
    memset(&_field, 0, sizeof(_field));
}

HValue::HValue(unsigned char v)
    : type_(Type::BYTE) {
    _field.byteVal = v;
}

HValue::HValue(int v)
    : type_(Type::INTEGER) {
    _field.intVal = v;
}

HValue::HValue(float v)
    : type_(Type::FLOAT) {
    _field.floatVal = v;
}

HValue::HValue(double v)
    : type_(Type::DOUBLE) {
    _field.doubleVal = v;
}

HValue::HValue(bool v)
    : type_(Type::BOOLEAN) {
    _field.boolVal = v;
}

HValue::HValue(const char* v)
    : type_(Type::STRING) {
    _field.strVal = new std::string();
    if (v) {
        *_field.strVal = v;
    }
}

HValue::HValue(const std::string& v)
    : type_(Type::STRING) {
    _field.strVal = new std::string();
    *_field.strVal = v;
}

HValue::HValue(const ValueVector& v)
    : type_(Type::VECTOR) {
    _field.vectorVal = new (std::nothrow) ValueVector();
    *_field.vectorVal = v;
}

HValue::HValue(ValueVector&& v)
    : type_(Type::VECTOR) {
    _field.vectorVal = new (std::nothrow) ValueVector();
    *_field.vectorVal = std::move(v);
}

HValue::HValue(const ValueMap& v)
    : type_(Type::MAP) {
    _field.mapVal = new (std::nothrow) ValueMap();
    *_field.mapVal = v;
}

HValue::HValue(ValueMap&& v)
    : type_(Type::MAP) {
    _field.mapVal = new (std::nothrow) ValueMap();
    *_field.mapVal = std::move(v);
}

HValue::HValue(const ValueMapIntKey& v)
    : type_(Type::INT_KEY_MAP) {
    _field.intKeyMapVal = new (std::nothrow) ValueMapIntKey();
    *_field.intKeyMapVal = v;
}

HValue::HValue(ValueMapIntKey&& v)
    : type_(Type::INT_KEY_MAP) {
    _field.intKeyMapVal = new (std::nothrow) ValueMapIntKey();
    *_field.intKeyMapVal = std::move(v);
}

HValue::HValue(const HValue& other)
    : type_(Type::NONE) {
    *this = other;
}

HValue::HValue(HValue&& other)
    : type_(Type::NONE) {
    *this = std::move(other);
}

HValue::~HValue() {
    clear();
}

HValue& HValue::operator= (const HValue& other) {
    if (this != &other) {
        reset(other.type_);

        switch (other.type_) {
            case Type::BYTE:
                _field.byteVal = other._field.byteVal;
                break;
            case Type::INTEGER:
                _field.intVal = other._field.intVal;
                break;
            case Type::FLOAT:
                _field.floatVal = other._field.floatVal;
                break;
            case Type::DOUBLE:
                _field.doubleVal = other._field.doubleVal;
                break;
            case Type::BOOLEAN:
                _field.boolVal = other._field.boolVal;
                break;
            case Type::STRING:
                if (_field.strVal == nullptr) {
                    _field.strVal = new std::string();
                }
                *_field.strVal = *other._field.strVal;
                break;
            case Type::VECTOR:
                if (_field.vectorVal == nullptr) {
                    _field.vectorVal = new (std::nothrow) ValueVector();
                }
                *_field.vectorVal = *other._field.vectorVal;
                break;
            case Type::MAP:
                if (_field.mapVal == nullptr) {
                    _field.mapVal = new (std::nothrow) ValueMap();
                }
                *_field.mapVal = *other._field.mapVal;
                break;
            case Type::INT_KEY_MAP:
                if (_field.intKeyMapVal == nullptr) {
                    _field.intKeyMapVal = new (std::nothrow) ValueMapIntKey();
                }
                *_field.intKeyMapVal = *other._field.intKeyMapVal;
                break;
            default:
                break;
        }
    }
    return *this;
}

HValue& HValue::operator= (HValue&& other) {
    if (this != &other)
    {
        clear();
        switch (other.type_) {
            case Type::BYTE:
                _field.byteVal = other._field.byteVal;
                break;
            case Type::INTEGER:
                _field.intVal = other._field.intVal;
                break;
            case Type::FLOAT:
                _field.floatVal = other._field.floatVal;
                break;
            case Type::DOUBLE:
                _field.doubleVal = other._field.doubleVal;
                break;
            case Type::BOOLEAN:
                _field.boolVal = other._field.boolVal;
                break;
            case Type::STRING:
                _field.strVal = other._field.strVal;
                break;
            case Type::VECTOR:
                _field.vectorVal = other._field.vectorVal;
                break;
            case Type::MAP:
                _field.mapVal = other._field.mapVal;
                break;
            case Type::INT_KEY_MAP:
                _field.intKeyMapVal = other._field.intKeyMapVal;
                break;
            default:
                break;
        }
        type_ = other.type_;

        memset(&other._field, 0, sizeof(other._field));
        other.type_ = Type::NONE;
    }

    return *this;
}

HValue& HValue::operator= (unsigned char v) {
    reset(Type::BYTE);
    _field.byteVal = v;
    return *this;
}

HValue& HValue::operator= (int v) {
    reset(Type::INTEGER);
    _field.intVal = v;
    return *this;
}

HValue& HValue::operator= (float v) {
    reset(Type::FLOAT);
    _field.floatVal = v;
    return *this;
}

HValue& HValue::operator= (double v) {
    reset(Type::DOUBLE);
    _field.doubleVal = v;
    return *this;
}

HValue& HValue::operator= (bool v) {
    reset(Type::BOOLEAN);
    _field.boolVal = v;
    return *this;
}

HValue& HValue::operator= (const char* v) {
    reset(Type::STRING);
    *_field.strVal = v ? v : "";
    return *this;
}

HValue& HValue::operator= (const std::string& v) {
    reset(Type::STRING);
    *_field.strVal = v;
    return *this;
}

HValue& HValue::operator= (const ValueVector& v) {
    reset(Type::VECTOR);
    *_field.vectorVal = v;
    return *this;
}

HValue& HValue::operator= (ValueVector&& v) {
    reset(Type::VECTOR);
    *_field.vectorVal = std::move(v);
    return *this;
}

HValue& HValue::operator= (const ValueMap& v) {
    reset(Type::MAP);
    *_field.mapVal = v;
    return *this;
}

HValue& HValue::operator= (ValueMap&& v) {
    reset(Type::MAP);
    *_field.mapVal = std::move(v);
    return *this;
}

HValue& HValue::operator= (const ValueMapIntKey& v) {
    reset(Type::INT_KEY_MAP);
    *_field.intKeyMapVal = v;
    return *this;
}

HValue& HValue::operator= (ValueMapIntKey&& v) {
    reset(Type::INT_KEY_MAP);
    *_field.intKeyMapVal = std::move(v);
    return *this;
}

bool HValue::operator!= (const HValue& v) {
    return !(*this == v);
}

bool HValue::operator!= (const HValue& v) const {
    return !(*this == v);
}

bool HValue::operator== (const HValue& v) {
    const auto &t = *this;
    return t == v;
}
bool HValue::operator== (const HValue& v) const {
    if (this == &v) return true;
    if (v.type_ != this->type_) return false;
    if (this->isNull()) return true;
    switch (type_) {
    case Type::BYTE:    return v._field.byteVal   == this->_field.byteVal;
    case Type::INTEGER: return v._field.intVal    == this->_field.intVal;
    case Type::BOOLEAN: return v._field.boolVal   == this->_field.boolVal;
    case Type::STRING:  return *v._field.strVal   == *this->_field.strVal;
    case Type::FLOAT:   return fabs(v._field.floatVal  - this->_field.floatVal)  <= MATH::MATH_FLOAT_EPSILON();
    case Type::DOUBLE:  return fabs(v._field.doubleVal - this->_field.doubleVal) <= MATH::MATH_FLOAT_EPSILON();
    case Type::VECTOR: {
        const auto &v1 = *(this->_field.vectorVal);
        const auto &v2 = *(v._field.vectorVal);
        const auto size = v1.size();
        if (size == v2.size()) {
            for (uint64 i = 0; i < size; i++) {
                if (v1[i] != v2[i]) return false;
            }
        }
        return true;
    }
    case Type::MAP: {
        const auto &map1 = *(this->_field.mapVal);
        const auto &map2 = *(v._field.mapVal);
        for (const auto &kvp : map1) {
            auto it = map2.find(kvp.first);
            if (it == map2.end() || it->second != kvp.second)
            {
                return false;
            }
        }
        return true;
    }
    case Type::INT_KEY_MAP: {
        const auto &map1 = *(this->_field.intKeyMapVal);
        const auto &map2 = *(v._field.intKeyMapVal);
        for (const auto &kvp : map1) {
            auto it = map2.find(kvp.first);
            if (it == map2.end() || it->second != kvp.second) {
                return false;
            }
        }
        return true;
    }
    default:
        break;
    };

    return false;
}

/// Convert HValue to a specified type
unsigned char HValue::asByte() const
{
    if (type_ == Type::BYTE) {
        return _field.byteVal;
    }

    if (type_ == Type::INTEGER) {
        return static_cast<unsigned char>(_field.intVal);
    }

    if (type_ == Type::STRING) {
        return static_cast<unsigned char>(atoi(_field.strVal->c_str()));
    }

    if (type_ == Type::FLOAT) {
        return static_cast<unsigned char>(_field.floatVal);
    }

    if (type_ == Type::DOUBLE) {
        return static_cast<unsigned char>(_field.doubleVal);
    }

    if (type_ == Type::BOOLEAN) {
        return _field.boolVal ? 1 : 0;
    }

    return 0;
}

int HValue::asInt() const {
    if (type_ == Type::INTEGER) {
        return _field.intVal;
    }

    if (type_ == Type::BYTE) {
        return _field.byteVal;
    }

    if (type_ == Type::STRING) {
        return atoi(_field.strVal->c_str());
    }

    if (type_ == Type::FLOAT) {
        return static_cast<int>(_field.floatVal);
    }

    if (type_ == Type::DOUBLE) {
        return static_cast<int>(_field.doubleVal);
    }

    if (type_ == Type::BOOLEAN) {
        return _field.boolVal ? 1 : 0;
    }

    return 0;
}

float HValue::asFloat() const
{
    if (type_ == Type::FLOAT) {
        return _field.floatVal;
    }

    if (type_ == Type::BYTE) {
        return static_cast<float>(_field.byteVal);
    }

    if (type_ == Type::STRING) {
        return atof(_field.strVal->c_str());
    }

    if (type_ == Type::INTEGER) {
        return static_cast<float>(_field.intVal);
    }

    if (type_ == Type::DOUBLE) {
        return static_cast<float>(_field.doubleVal);
    }

    if (type_ == Type::BOOLEAN) {
        return _field.boolVal ? 1.0f : 0.0f;
    }

    return 0.0f;
}

double HValue::asDouble() const {
    if (type_ == Type::DOUBLE) {
        return _field.doubleVal;
    }

    if (type_ == Type::BYTE) {
        return static_cast<double>(_field.byteVal);
    }

    if (type_ == Type::STRING) {
        return static_cast<double>(atof(_field.strVal->c_str()));
    }

    if (type_ == Type::INTEGER) {
        return static_cast<double>(_field.intVal);
    }

    if (type_ == Type::FLOAT) {
        return static_cast<double>(_field.floatVal);
    }

    if (type_ == Type::BOOLEAN) {
        return _field.boolVal ? 1.0 : 0.0;
    }

    return 0.0;
}

bool HValue::asBool() const {
    if (type_ == Type::BOOLEAN) {
        return _field.boolVal;
    }

    if (type_ == Type::BYTE) {
        return _field.byteVal == 0 ? false : true;
    }

    if (type_ == Type::STRING) {
        return (*_field.strVal == "0" || *_field.strVal == "false") ? false : true;
    }

    if (type_ == Type::INTEGER) {
        return _field.intVal == 0 ? false : true;
    }

    if (type_ == Type::FLOAT) {
        return _field.floatVal == 0.0f ? false : true;
    }

    if (type_ == Type::DOUBLE) {
        return _field.doubleVal == 0.0 ? false : true;
    }

    return false;
}

std::string HValue::asString() const
{
    if (type_ == Type::STRING) {
        return *_field.strVal;
    }

    std::stringstream ret;

    switch (type_) {
        case Type::BYTE:
            ret << _field.byteVal;
            break;
        case Type::INTEGER:
            ret << _field.intVal;
            break;
        case Type::FLOAT:
            ret << std::fixed << std::setprecision(7)<< _field.floatVal;
            break;
        case Type::DOUBLE:
            ret << std::fixed << std::setprecision(16) << _field.doubleVal;
            break;
        case Type::BOOLEAN:
            ret << (_field.boolVal ? "true" : "false");
            break;
        default:
            break;
    }
    return ret.str();
}

ValueVector& HValue::asValueVector() {
    return *_field.vectorVal;
}

const ValueVector& HValue::asValueVector() const {
    return *_field.vectorVal;
}

ValueMap& HValue::asValueMap() {
    return *_field.mapVal;
}

const ValueMap& HValue::asValueMap() const {
    return *_field.mapVal;
}

ValueMapIntKey& HValue::asIntKeyMap() {
    return *_field.intKeyMapVal;
}

const ValueMapIntKey& HValue::asIntKeyMap() const {
    return *_field.intKeyMapVal;
}

void HValue::clear()
{
    // Free memory the old HValue allocated
    switch (type_) {
        case Type::BYTE:
            _field.byteVal = 0;
            break;
        case Type::INTEGER:
            _field.intVal = 0;
            break;
        case Type::FLOAT:
            _field.floatVal = 0.0f;
            break;
        case Type::DOUBLE:
            _field.doubleVal = 0.0;
            break;
        case Type::BOOLEAN:
            _field.boolVal = false;
            break;
        case Type::STRING:
            SAFE_DELETE(_field.strVal);
            break;
        case Type::VECTOR:
            SAFE_DELETE(_field.vectorVal);
            break;
        case Type::MAP:
            SAFE_DELETE(_field.mapVal);
            break;
        case Type::INT_KEY_MAP:
            SAFE_DELETE(_field.intKeyMapVal);
            break;
        default:
            break;
    }

    type_ = Type::NONE;
}

void HValue::reset(Type type) {
    if (type_ == type)
        return;

    clear();

    // Allocate memory for the new HValue
    switch (type) {
        case Type::STRING:
            _field.strVal = new std::string();
            break;
        case Type::VECTOR:
            _field.vectorVal = new (std::nothrow) ValueVector();
            break;
        case Type::MAP:
            _field.mapVal = new (std::nothrow) ValueMap();
            break;
        case Type::INT_KEY_MAP:
            _field.intKeyMapVal = new (std::nothrow) ValueMapIntKey();
            break;
        default:
            break;
    }

    type_ = type;
}

