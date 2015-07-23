#include "HObject.h"

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
