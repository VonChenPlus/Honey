#ifndef CAMERA_H
#define CAMERA_H

#include "GRAPH/Node.h"

namespace GRAPH
{
    struct Viewport
    {
        Viewport(float left, float bottom, float width, float height)
            : _left(left)
            , _bottom(bottom)
            , _width(width)
            , _height(height) {

            }
        Viewport() {
            _left = _bottom = 0.f;
            _width = _height = 1.0f;
        }

        float _left;
        float _bottom;
        float _width;
        float _height;
    };

    class Camera :public Node
    {
        friend class Scene;
        friend class Director;
        friend class EventDispatcher;

    public:
        enum class Type
        {
            PERSPECTIVE = 1,
            ORTHOGRAPHIC = 2
        };

    public:
        static Camera* createPerspective(float fieldOfView, float aspectRatio, float nearPlane, float farPlane);
        static Camera* createOrthographic(float zoomX, float zoomY, float nearPlane, float farPlane);
        static Camera* create();

        virtual void lookAt(const MATH::Vector3f& target, const MATH::Vector3f& up = MATH::Vec3fUNITY);

        const MATH::Matrix4& getProjectionMatrix() const;
        const MATH::Matrix4& getViewMatrix() const;
        const MATH::Matrix4& getViewProjectionMatrix() const;

        MATH::Vector2f project(const MATH::Vector3f& src) const;
        MATH::Vector2f projectGL(const MATH::Vector3f& src) const;
        MATH::Vector3f unproject(const MATH::Vector3f& src) const;
        MATH::Vector3f unprojectGL(const MATH::Vector3f& src) const;

        void unproject(const MATH::Sizef& size, const MATH::Vector3f* src, MATH::Vector3f* dst) const;
        void unprojectGL(const MATH::Sizef& size, const MATH::Vector3f* src, MATH::Vector3f* dst) const;

        float getDepthInView(const MATH::Matrix4& transform) const;

        int getRenderOrder() const;
        float getFarPlane() const { return farPlane_; }
        float getNearPlane() const { return nearPlane_; }

        void apply();

        void setViewport(const Viewport& vp) { viewport_ = vp; }

    public:
        Camera();
        ~Camera();

        bool initDefault();
        bool initPerspective(float fieldOfView, float aspectRatio, float nearPlane, float farPlane);
        bool initOrthographic(float zoomX, float zoomY, float nearPlane, float farPlane);

        void applyViewport();

    protected:
        MATH::Matrix4 projection_;
        mutable MATH::Matrix4 _view;
        mutable MATH::Matrix4 viewInv_;
        mutable MATH::Matrix4 viewProjection_;
        float fieldOfView_;
        float zoom_[2];
        float aspectRatio_;
        float nearPlane_;
        float farPlane_;
        mutable bool  viewProjectionDirty_;
        Viewport viewport_;
    };
}

#endif// CAMERA_H
