#include "GRAPH/RenderView.h"
#include "GRAPH/UNITY3D/GLCommon.h"

namespace GRAPH
{
    RenderView::RenderView()
        : scaleX_(1.0f)
        , scaleY_(1.0f)
        , resolutionPolicy_(ResolutionPolicy::UNKNOWN) {

    }

    RenderView::~RenderView() {

    }

    void RenderView::updateDesignResolutionSize() {
        if (screenSize_.width > 0 && screenSize_.height > 0
            && designResolutionSize_.width > 0 && designResolutionSize_.height > 0) {
            scaleX_ = (float)screenSize_.width / designResolutionSize_.width;
            scaleY_ = (float)screenSize_.height / designResolutionSize_.height;

            if (resolutionPolicy_ == ResolutionPolicy::NO_BORDER) {
                scaleX_ = scaleY_ = MATH::MATH_MAX(scaleX_, scaleY_);
            }
            else if (resolutionPolicy_ == ResolutionPolicy::SHOW_ALL) {
                scaleX_ = scaleY_ = MATH::MATH_MIN(scaleX_, scaleY_);
            }
            else if ( resolutionPolicy_ == ResolutionPolicy::FIXED_HEIGHT) {
                scaleX_ = scaleY_;
                designResolutionSize_.width = ceilf(screenSize_.width/scaleX_);
            }
            else if ( resolutionPolicy_ == ResolutionPolicy::FIXED_WIDTH) {
                scaleY_ = scaleX_;
                designResolutionSize_.height = ceilf(screenSize_.height/scaleY_);
            }

            // calculate the rect of viewport
            float viewPortW = designResolutionSize_.width * scaleX_;
            float viewPortH = designResolutionSize_.height * scaleY_;

            viewPortRect_.setRect((screenSize_.width - viewPortW) / 2, (screenSize_.height - viewPortH) / 2, viewPortW, viewPortH);
        }
    }

    void RenderView::setDesignResolutionSize(float width, float height, ResolutionPolicy resolutionPolicy) {
        if (width == 0.0f || height == 0.0f) {
            return;
        }

        designResolutionSize_.setSize(width, height);
        resolutionPolicy_ = resolutionPolicy;

        updateDesignResolutionSize();
    }

    const MATH::Sizef& RenderView::getDesignResolutionSize() const {
        return designResolutionSize_;
    }

    const MATH::Sizef& RenderView::getFrameSize() const {
        return screenSize_;
    }

    void RenderView::setFrameSize(float width, float height) {
        designResolutionSize_ = screenSize_ = MATH::Sizef(width, height);
    }

    MATH::Rectf RenderView::getVisibleRect() const {
        MATH::Rectf ret;
        ret.size = getVisibleSize();
        ret.origin = getVisibleOrigin();
        return ret;
    }

    MATH::Sizef RenderView::getVisibleSize() const {
        if (resolutionPolicy_ == ResolutionPolicy::NO_BORDER) {
            return MATH::Sizef(screenSize_.width/scaleX_, screenSize_.height/scaleY_);
        }
        else {
            return designResolutionSize_;
        }
    }

    MATH::Vector2f RenderView::getVisibleOrigin() const {
        if (resolutionPolicy_ == ResolutionPolicy::NO_BORDER) {
            return MATH::Vector2f((designResolutionSize_.width - screenSize_.width/scaleX_)/2,
                               (designResolutionSize_.height - screenSize_.height/scaleY_)/2);
        }
        else {
            return MATH::Vec2fZERO;
        }
    }

    const MATH::Rectf& RenderView::getViewPortRect() const {
        return viewPortRect_;
    }

    float RenderView::getScaleX() const {
        return scaleX_;
    }

    float RenderView::getScaleY() const {
        return scaleY_;
    }

    void RenderView::setScissorInPoints(float x , float y , float w , float h) {
        glScissor((GLint)(x * scaleX_ + viewPortRect_.origin.x),
                      (GLint)(y * scaleY_ + viewPortRect_.origin.y),
                      (GLsizei)(w * scaleX_),
                      (GLsizei)(h * scaleY_));
    }
}
