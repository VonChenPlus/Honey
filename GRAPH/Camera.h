#ifndef CAMERA_H
#define CAMERA_H

#include "GRAPH/Node.h"

namespace GRAPH
{
    class Scene;

    enum class CameraFlag
    {
        DEFAULT = 1,
        USER1 = 1 << 1,
        USER2 = 1 << 2,
        USER3 = 1 << 3,
        USER4 = 1 << 4,
        USER5 = 1 << 5,
        USER6 = 1 << 6,
        USER7 = 1 << 7,
        USER8 = 1 << 8,
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

        Camera::Type getType() const { return _type; }

        CameraFlag getCameraFlag() const { return (CameraFlag)_cameraFlag; }
        void setCameraFlag(CameraFlag flag) { _cameraFlag = (unsigned short)flag; }

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

        void setDepth(int8_t depth);
        int8_t getDepth() const { return _depth; }

        int getRenderOrder() const;

        float getFarPlane() const { return _farPlane; }
        float getNearPlane() const { return _nearPlane; }

        virtual void onEnter() override;
        virtual void onExit() override;

        static const Camera* getVisitingCamera() { return _visitingCamera; }
        static Camera* getDefaultCamera();

    public:
        Camera();
        ~Camera();

        void setScene(Scene* scene);

        bool initDefault();
        bool initPerspective(float fieldOfView, float aspectRatio, float nearPlane, float farPlane);
        bool initOrthographic(float zoomX, float zoomY, float nearPlane, float farPlane);

    protected:
        Scene* _scene;
        MATH::Matrix4 _projection;
        mutable MATH::Matrix4 _view;
        mutable MATH::Matrix4 _viewInv;
        mutable MATH::Matrix4 _viewProjection;
        MATH::Vector3f _up;
        Camera::Type _type;
        float _fieldOfView;
        float _zoom[2];
        float _aspectRatio;
        float _nearPlane;
        float _farPlane;
        mutable bool  _viewProjectionDirty;
        unsigned short _cameraFlag;
        mutable bool _frustumDirty;
        int8_t  _depth;
        static Camera* _visitingCamera;
    };
}

#endif// CAMERA_H
