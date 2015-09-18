#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include <vector>
#include "MATH/Size.h"
#include "MATH/Rectangle.h"

namespace GRAPH
{
    enum class ResolutionPolicy
    {
        /** The entire application is visible in the specified area without trying to preserve the original aspect ratio.
         * Distortion can occur, and the application may appear stretched or compressed.
         */
        EXACT_FIT,
        /** The entire application fills the specified area, without distortion but possibly with some cropping,
         * while maintaining the original aspect ratio of the application.
         */
        NO_BORDER,
        /** The entire application is visible in the specified area without distortion while maintaining the original
         * aspect ratio of the application. Borders can appear on two sides of the application.
         */
        SHOW_ALL,
        /** The application takes the height of the design resolution size and modifies the width of the internal
         * canvas so that it fits the aspect ratio of the device.
         * No distortion will occur however you must make sure your application works on different
         * aspect ratios.
         */
        FIXED_HEIGHT,
        /** The application takes the width of the design resolution size and modifies the height of the internal
         * canvas so that it fits the aspect ratio of the device.
         * No distortion will occur however you must make sure your application works on different
         * aspect ratios.
         */
        FIXED_WIDTH,

        UNKNOWN,
    };

    class RenderView
    {
    public:
        RenderView();
        virtual ~RenderView();

        virtual const MATH::Sizef& getFrameSize() const;
        virtual void setFrameSize(float width, float height);

        virtual void setFrameZoomFactor(float) {}
        virtual float getFrameZoomFactor() const { return 1.0; }

        virtual void setCursorVisible(bool) {}

        virtual int getRetinaFactor() const { return 1; }

        virtual bool setContentScaleFactor(float) { return false; }
        virtual float getContentScaleFactor() const { return 1.0; }

        virtual bool isRetinaDisplay() const { return false; }

        virtual MATH::Sizef getVisibleSize() const;
        virtual MATH::Vector2f getVisibleOrigin() const;
        virtual MATH::Rectf getVisibleRect() const;

        virtual void setDesignResolutionSize(float width, float height, ResolutionPolicy resolutionPolicy);
        virtual const MATH::Sizef&  getDesignResolutionSize() const;

        const MATH::Rectf& getViewPortRect() const;

        float getScaleX() const;
        float getScaleY() const;

    protected:
        void updateDesignResolutionSize();

        // real screen size
        MATH::Sizef screenSize_;
        // resolution size, it is the size appropriate for the app resources.
        MATH::Sizef designResolutionSize_;
        // the view port size
        MATH::Rectf viewPortRect_;

        float scaleX_;
        float scaleY_;
        ResolutionPolicy resolutionPolicy_;
    };
}

#endif // RENDERVIEW_H
