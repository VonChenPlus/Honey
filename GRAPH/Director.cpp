#include <string>
#include "GRAPH/Director.h"
#include "GRAPH/Action.h"
#include "GRAPH/Event.h"
#include "GRAPH/Scheduler.h"
#include "GRAPH/EventDispatcher.h"
#include "GRAPH/RenderView.h"
#include "GRAPH/RENDERER/Texture2D.h"
#include "GRAPH/RENDERER/Renderer.h"

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

        actionManager_ = new (std::nothrow) ActionManager;
        scheduler_ = new (std::nothrow) Scheduler;
        eventDispatcher_ = new (std::nothrow) EventDispatcher;
        textureCache_ = new (std::nothrow) TextureCache(scheduler_);
        renderer_ = new (std::nothrow) Renderer;

        return true;
    }

    Director::~Director(void) {
        if (textureCache_) {
            textureCache_->waitForQuit();
            SAFE_RELEASE_NULL(textureCache_);
        }
    }

    void Director::setProjection(Projection projection) {
        MATH::Sizef size = getWinSize();

        switch (projection) {
            case Projection::_2D: {
                loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);

                MATH::Matrix4 orthoMatrix;
                MATH::Matrix4::createOrthographicOffCenter(0, size.width, 0, size.height, -1024, 1024, &orthoMatrix);
                multiplyMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION, orthoMatrix);
                loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
                break;
            }
            case Projection::_3D: {
                float zeye = this->getZEye();

                MATH::Matrix4 matrixPerspective, matrixLookup;

                loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);

                // issue #1334
                MATH::Matrix4::createPerspective(60, (GLfloat)size.width/size.height, 10, zeye+size.height/2, &matrixPerspective);

                multiplyMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION, matrixPerspective);

                MATH::Vector3f eye(size.width/2, size.height/2, zeye), center(size.width/2, size.height/2, 0.0f), up(0.0f, 1.0f, 0.0f);
                MATH::Matrix4::createLookAt(eye, center, up, &matrixLookup);
                multiplyMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION, matrixLookup);

                loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
                break;
            }
            default:
                break;
        }

        projection_ = projection;
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

    void Director::resetMatrixStack() {
        initMatrixStack();
    }

    const MATH::Sizef& Director::getWinSize() const {
        return renderView_->getDesignResolutionSize();
    }

    MATH::Vector2f Director::convertToGL(const MATH::Vector2f& uiPoint) {
        MATH::Matrix4 transform;
        glToClipTransform(&transform);

        MATH::Matrix4 transformInv = transform.getInversed();

        // Calculate z=0 using -> transform*[0, 0, 0, 1]/w
        float zClip = transform.m[14]/transform.m[15];

        MATH::Sizef glSize = renderView_->getDesignResolutionSize();
        MATH::Vector4f clipCoord(2.0f*uiPoint.x/glSize.width - 1.0f, 1.0f - 2.0f*uiPoint.y/glSize.height, zClip, 1);

        MATH::Vector4f glCoord;
        //transformInv.transformPoint(clipCoord, &glCoord);
        transformInv.transformVector(clipCoord, &glCoord);
        float factor = 1.0/glCoord.w;
        return MATH::Vector2f(glCoord.x * factor, glCoord.y * factor);
    }

    MATH::Vector2f Director::convertToUI(const MATH::Vector2f& glPoint) {
        MATH::Matrix4 transform;
        glToClipTransform(&transform);

        MATH::Vector4f clipCoord;
        // Need to calculate the zero depth from the transform.
        MATH::Vector4f glCoord(glPoint.x, glPoint.y, 0.0, 1);
        transform.transformVector(glCoord, &clipCoord);

        clipCoord.x = clipCoord.x / clipCoord.w;
        clipCoord.y = clipCoord.y / clipCoord.w;
        clipCoord.z = clipCoord.z / clipCoord.w;

        MATH::Sizef glSize = renderView_->getDesignResolutionSize();
        float factor = 1.0/glCoord.w;
        return MATH::Vector2f(glSize.width*(clipCoord.x*0.5 + 0.5) * factor, glSize.height*(-clipCoord.y*0.5 + 0.5) * factor);
    }

    void Director::glToClipTransform(MATH::Matrix4 *transformOut) {
        if(nullptr == transformOut) return;

        auto projection = getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
        auto modelview = getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        *transformOut = projection * modelview;
    }
}
