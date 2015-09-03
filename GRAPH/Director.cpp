#include <string>
#include "GRAPH/Director.h"

namespace GRAPH
{
    Director& Director::getInstance() {
        static Director instance;
        return instance;
    }

    Director::Director() {
    }

    bool Director::init(void) {
        initMatrixStack();

        return true;
    }

    Director::~Director(void) {

    }

    void Director::initMatrixStack() {
        while (!modelViewMatrixStack_.empty()) {
            modelViewMatrixStack_.pop();
        }

        while (!projectionMatrixStack_.empty()) {
            projectionMatrixStack_.pop();
        }

        while (!textureMatrixStack_.empty()) {
            textureMatrixStack_.pop();
        }

        modelViewMatrixStack_.push(MATH::Matrix4::IDENTITY);
        projectionMatrixStack_.push(MATH::Matrix4::IDENTITY);
        textureMatrixStack_.push(MATH::Matrix4::IDENTITY);
    }

    void Director::resetMatrixStack() {
        initMatrixStack();
    }

    void Director::popMatrix(MATRIX_STACK_TYPE type) {
        if(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW == type) {
            modelViewMatrixStack_.pop();
        }
        else if(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION == type) {
            projectionMatrixStack_.pop();
        }
        else if(MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE == type) {
            textureMatrixStack_.pop();
        }
    }

    void Director::loadIdentityMatrix(MATRIX_STACK_TYPE type) {
        if(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW == type) {
            modelViewMatrixStack_.top() = MATH::Matrix4::IDENTITY;
        }
        else if(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION == type) {
            projectionMatrixStack_.top() = MATH::Matrix4::IDENTITY;
        }
        else if(MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE == type) {
            textureMatrixStack_.top() = MATH::Matrix4::IDENTITY;
        }
    }

    void Director::loadMatrix(MATRIX_STACK_TYPE type, const MATH::Matrix4& mat) {
        if(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW == type) {
            modelViewMatrixStack_.top() = mat;
        }
        else if(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION == type) {
            projectionMatrixStack_.top() = mat;
        }
        else if(MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE == type) {
            textureMatrixStack_.top() = mat;
        }
    }

    void Director::multiplyMatrix(MATRIX_STACK_TYPE type, const MATH::Matrix4& mat) {
        if(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW == type) {
            modelViewMatrixStack_.top() *= mat;
        }
        else if(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION == type) {
            projectionMatrixStack_.top() *= mat;
        }
        else if(MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE == type) {
            textureMatrixStack_.top() *= mat;
        }
    }

    void Director::pushMatrix(MATRIX_STACK_TYPE type) {
        if(type == MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW) {
            modelViewMatrixStack_.push(modelViewMatrixStack_.top());
        }
        else if(type == MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION) {
            projectionMatrixStack_.push(projectionMatrixStack_.top());
        }
        else if(type == MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE) {
            textureMatrixStack_.push(textureMatrixStack_.top());
        }
    }

    const MATH::Matrix4& Director::getMatrix(MATRIX_STACK_TYPE type) {
        if(type == MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW) {
            return modelViewMatrixStack_.top();
        }
        else if(type == MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION) {
            return projectionMatrixStack_.top();
        }
        else if(type == MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE) {
            return textureMatrixStack_.top();
        }

        return  modelViewMatrixStack_.top();
    }
}
