#include "GRAPH/Camera.h"
#include "GRAPH/Director.h"

namespace GRAPH
{
    Camera* Camera::create() {
        Camera* camera = new (std::nothrow) Camera();
        camera->initDefault();
        camera->autorelease();
        return camera;
    }

    Camera* Camera::createPerspective(float fieldOfView, float aspectRatio, float nearPlane, float farPlane) {
        auto ret = new (std::nothrow) Camera();
        if (ret) {
            ret->initPerspective(fieldOfView, aspectRatio, nearPlane, farPlane);
            ret->autorelease();
            return ret;
        }
        SAFE_DELETE(ret);
        return nullptr;
    }

    Camera* Camera::createOrthographic(float zoomX, float zoomY, float nearPlane, float farPlane) {
        auto ret = new (std::nothrow) Camera();
        if (ret) {
            ret->initOrthographic(zoomX, zoomY, nearPlane, farPlane);
            ret->autorelease();
            return ret;
        }
        SAFE_DELETE(ret);
        return nullptr;
    }

    Camera::Camera()
        : viewProjectionDirty_(true) {

    }

    Camera::~Camera() {

    }

    const MATH::Matrix4& Camera::getProjectionMatrix() const {
        return projection_;
    }

    const MATH::Matrix4& Camera::getViewMatrix() const {
        MATH::Matrix4 viewInv(getNodeToWorldTransform());
        static int count = sizeof(float) * 16;
        if (memcmp(viewInv.m, viewInv_.m, count) != 0) {
            viewProjectionDirty_ = true;
            viewInv_ = viewInv;
            _view = viewInv.getInversed();
        }
        return _view;
    }

    void Camera::lookAt(const MATH::Vector3f& lookAtPos, const MATH::Vector3f& up) {
        MATH::Vector3f upv = up;
        upv.normalize();
        MATH::Vector3f zaxis;
        MATH::Vector3f::subtract(this->getPosition3D(), lookAtPos, &zaxis);
        zaxis.normalize();

        MATH::Vector3f xaxis;
        MATH::Vector3f::cross(upv, zaxis, &xaxis);
        xaxis.normalize();

        MATH::Vector3f yaxis;
        MATH::Vector3f::cross(zaxis, xaxis, &yaxis);
        yaxis.normalize();
        MATH::Matrix4  rotation;
        rotation.m[0] = xaxis.x;
        rotation.m[1] = xaxis.y;
        rotation.m[2] = xaxis.z;
        rotation.m[3] = 0;

        rotation.m[4] = yaxis.x;
        rotation.m[5] = yaxis.y;
        rotation.m[6] = yaxis.z;
        rotation.m[7] = 0;
        rotation.m[8] = zaxis.x;
        rotation.m[9] = zaxis.y;
        rotation.m[10] = zaxis.z;
        rotation.m[11] = 0;

        MATH::Quaternion  quaternion;
        MATH::Quaternion::createFromRotationMatrix(rotation,&quaternion);
        quaternion.normalize();
        setRotationQuat(quaternion);
    }

    const MATH::Matrix4& Camera::getViewProjectionMatrix() const {
        getViewMatrix();
        if (viewProjectionDirty_) {
            viewProjectionDirty_ = false;
            MATH::Matrix4::multiply(projection_, _view, &viewProjection_);
        }

        return viewProjection_;
    }

    bool Camera::initDefault() {
        auto size = Director::getInstance().getWinSize();
        //create default camera
        auto projection = Director::getInstance().getProjection();
        switch (projection) {
            case Projection::_2D:
            {
                initOrthographic(size.width, size.height, -1024, 1024);
                setPosition3D(MATH::Vector3f(0.0f, 0.0f, 0.0f));
                setRotation3D(MATH::Vector3f(0.f, 0.f, 0.f));
                break;
            }
            case Projection::_3D: {
                float zeye = Director::getInstance().getZEye();
                initPerspective(60, (GLfloat)size.width / size.height, 10, zeye + size.height / 2.0f);
                MATH::Vector3f eye(size.width/2, size.height/2.0f, zeye), center(size.width/2, size.height/2, 0.0f), up(0.0f, 1.0f, 0.0f);
                setPosition3D(eye);
                lookAt(center, up);
                break;
            }
            default:
                break;
        }
        return true;
    }

    bool Camera::initPerspective(float fieldOfView, float aspectRatio, float nearPlane, float farPlane) {
        fieldOfView_ = fieldOfView;
        aspectRatio_ = aspectRatio;
        nearPlane_ = nearPlane;
        farPlane_ = farPlane;
        MATH::Matrix4::createPerspective(fieldOfView_, aspectRatio_, nearPlane_, farPlane_, &projection_);
        viewProjectionDirty_ = true;

        return true;
    }

    bool Camera::initOrthographic(float zoomX, float zoomY, float nearPlane, float farPlane) {
        zoom_[0] = zoomX;
        zoom_[1] = zoomY;
        nearPlane_ = nearPlane;
        farPlane_ = farPlane;
        MATH::Matrix4::createOrthographicOffCenter(0, zoom_[0], 0, zoom_[1], nearPlane_, farPlane_, &projection_);
        viewProjectionDirty_ = true;

        return true;
    }

    MATH::Vector2f Camera::project(const MATH::Vector3f& src) const {
        MATH::Vector2f screenPos;

        auto viewport = Director::getInstance().getWinSize();
        MATH::Vector4f clipPos;
        getViewProjectionMatrix().transformVector(MATH::Vector4f(src.x, src.y, src.z, 1.0f), &clipPos);

        float ndcX = clipPos.x / clipPos.w;
        float ndcY = clipPos.y / clipPos.w;

        screenPos.x = (ndcX + 1.0f) * 0.5f * viewport.width;
        screenPos.y = (1.0f - (ndcY + 1.0f) * 0.5f) * viewport.height;
        return screenPos;
    }

    MATH::Vector2f Camera::projectGL(const MATH::Vector3f& src) const {
        MATH::Vector2f screenPos;

        auto viewport = Director::getInstance().getWinSize();
        MATH::Vector4f clipPos;
        getViewProjectionMatrix().transformVector(MATH::Vector4f(src.x, src.y, src.z, 1.0f), &clipPos);

        float ndcX = clipPos.x / clipPos.w;
        float ndcY = clipPos.y / clipPos.w;

        screenPos.x = (ndcX + 1.0f) * 0.5f * viewport.width;
        screenPos.y = (ndcY + 1.0f) * 0.5f * viewport.height;
        return screenPos;
    }

    MATH::Vector3f Camera::unproject(const MATH::Vector3f& src) const {
        MATH::Vector3f dst;
        unproject(Director::getInstance().getWinSize(), &src, &dst);
        return dst;
    }

    MATH::Vector3f Camera::unprojectGL(const MATH::Vector3f& src) const {
        MATH::Vector3f dst;
        unprojectGL(Director::getInstance().getWinSize(), &src, &dst);
        return dst;
    }

    void Camera::unproject(const MATH::Sizef& viewport, const MATH::Vector3f* src, MATH::Vector3f* dst) const {
        MATH::Vector4f screen(src->x / viewport.width, ((viewport.height - src->y)) / viewport.height, src->z, 1.0f);
        screen.x = screen.x * 2.0f - 1.0f;
        screen.y = screen.y * 2.0f - 1.0f;
        screen.z = screen.z * 2.0f - 1.0f;

        getViewProjectionMatrix().getInversed().transformVector(screen, &screen);
        if (screen.w != 0.0f) {
            screen.x /= screen.w;
            screen.y /= screen.w;
            screen.z /= screen.w;
        }

        dst->set(screen.x, screen.y, screen.z);
    }

    void Camera::unprojectGL(const MATH::Sizef& viewport, const MATH::Vector3f* src, MATH::Vector3f* dst) const {
        MATH::Vector4f screen(src->x / viewport.width, src->y / viewport.height, src->z, 1.0f);
        screen.x = screen.x * 2.0f - 1.0f;
        screen.y = screen.y * 2.0f - 1.0f;
        screen.z = screen.z * 2.0f - 1.0f;

        getViewProjectionMatrix().getInversed().transformVector(screen, &screen);
        if (screen.w != 0.0f) {
            screen.x /= screen.w;
            screen.y /= screen.w;
            screen.z /= screen.w;
        }

        dst->set(screen.x, screen.y, screen.z);
    }

    float Camera::getDepthInView(const MATH::Matrix4& transform) const {
        MATH::Matrix4 camWorldMat = getNodeToWorldTransform();
        const MATH::Matrix4 &viewMat = camWorldMat.getInversed();
        float depth = -(viewMat.m[2] * transform.m[12] + viewMat.m[6] * transform.m[13] + viewMat.m[10] * transform.m[14] + viewMat.m[14]);
        return depth;
    }

    void Camera::apply() {
        applyViewport();
    }

    void Camera::applyViewport() {
        glViewport(viewport_._left, viewport_._bottom, viewport_._width, viewport_._height);
    }
}
