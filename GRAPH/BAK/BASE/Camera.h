#ifndef CAMERA_H
#define CAMERA_H

#include "GRAPH/BASE/Node.h"
#include "GRAPH/RENDERER/FrameBuffer.h"

namespace GRAPH
{
    class Scene;

    /**
     * Note:
     * Scene creates a default camera. And the default camera mask of Node is 1, therefore it can be seen by the default camera.
     * During rendering the scene, it draws the objects seen by each camera in the added order except default camera. The default camera is the last one being drawn with.
     * It's usually a good idea to render 3D objects in a separate camera.
     * And set the 3d camera flag to CameraFlag::USER1 or anything else except DEFAULT. Dedicate The DEFAULT camera for UI, because it is rendered at last.
     * You can change the camera order to get different result when depth test is not enabled.
     * For each camera, transparent 3d sprite is rendered after opaque 3d sprite and other 2d objects.
     */
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
    /**
     * Defines a camera .
     */
    class Camera :public Node
    {
        friend class Scene;
        friend class Director;
        friend class EventDispatcher;
    public:
        /**
        * The type of camera.
        */
        enum class Type
        {
            PERSPECTIVE = 1,
            ORTHOGRAPHIC = 2
        };
    public:
        /**
        * Creates a perspective camera.
        *
        * @param fieldOfView The field of view for the perspective camera (normally in the range of 40-60 degrees).
        * @param aspectRatio The aspect ratio of the camera (normally the width of the viewport divided by the height of the viewport).
        * @param nearPlane The near plane distance.
        * @param farPlane The far plane distance.
        */
        static Camera* createPerspective(float fieldOfView, float aspectRatio, float nearPlane, float farPlane);
        /**
        * Creates an orthographic camera.
        *
        * @param zoomX The zoom factor along the X-axis of the orthographic projection (the width of the ortho projection).
        * @param zoomY The zoom factor along the Y-axis of the orthographic projection (the height of the ortho projection).
        * @param nearPlane The near plane distance.
        * @param farPlane The far plane distance.
        */
        static Camera* createOrthographic(float zoomX, float zoomY, float nearPlane, float farPlane);

        /** create default camera, the camera type depends on Director::getProjection, the depth of the default camera is 0 */
        static Camera* create();

        /**
        * Gets the type of camera.
        *
        * @return The camera type.
        */
        Camera::Type getType() const { return _type; }

        /**get & set Camera flag*/
        CameraFlag getCameraFlag() const { return (CameraFlag)_cameraFlag; }
        void setCameraFlag(CameraFlag flag) { _cameraFlag = (unsigned short)flag; }

        /**
        * Make Camera looks at target
        *
        * @param target The target camera is point at
        * @param up The up vector, usually it's Y axis
        */
        virtual void lookAt(const MATH::Vector3f& target, const MATH::Vector3f& up = MATH::Vec3fUNITY);

        /**
        * Gets the camera's projection matrix.
        *
        * @return The camera projection matrix.
        */
        const MATH::Matrix4& getProjectionMatrix() const;
        /**
        * Gets the camera's view matrix.
        *
        * @return The camera view matrix.
        */
        const MATH::Matrix4& getViewMatrix() const;

        /**get view projection matrix*/
        const MATH::Matrix4& getViewProjectionMatrix() const;

        /* convert the specified point in 3D world-space coordinates into the screen-space coordinates.
         *
         * Origin point at left top corner in screen-space.
         * @param src The world-space position.
         * @return The screen-space position.
         */
        MATH::Vector2f project(const MATH::Vector3f& src) const;

        /* convert the specified point in 3D world-space coordinates into the GL-screen-space coordinates.
         *
         * Origin point at left bottom corner in GL-screen-space.
         * @param src The 3D world-space position.
         * @return The GL-screen-space position.
         */
        MATH::Vector2f projectGL(const MATH::Vector3f& src) const;

        /**
         * Convert the specified point of screen-space coordinate into the 3D world-space coordinate.
         *
         * Origin point at left top corner in screen-space.
         * @param src The screen-space position.
         * @return The 3D world-space position.
         */
        MATH::Vector3f unproject(const MATH::Vector3f& src) const;

        /**
         * Convert the specified point of GL-screen-space coordinate into the 3D world-space coordinate.
         *
         * Origin point at left bottom corner in GL-screen-space.
         * @param src The GL-screen-space position.
         * @return The 3D world-space position.
         */
        MATH::Vector3f unprojectGL(const MATH::Vector3f& src) const;

        /**
         * Convert the specified point of screen-space coordinate into the 3D world-space coordinate.
         *
         * Origin point at left top corner in screen-space.
         * @param size The window size to use.
         * @param src  The screen-space position.
         * @param dst  The 3D world-space position.
         */
        void unproject(const MATH::Sizef& size, const MATH::Vector3f* src, MATH::Vector3f* dst) const;

        /**
         * Convert the specified point of GL-screen-space coordinate into the 3D world-space coordinate.
         *
         * Origin point at left bottom corner in GL-screen-space.
         * @param size The window size to use.
         * @param src  The GL-screen-space position.
         * @param dst  The 3D world-space position.
         */
        void unprojectGL(const MATH::Sizef& size, const MATH::Vector3f* src, MATH::Vector3f* dst) const;

        /**
         * Get object depth towards camera
         */
        float getDepthInView(const MATH::Matrix4& transform) const;

        /**
         * set depth, camera with larger depth is drawn on top of camera with smaller depth, the depth of camera with CameraFlag::DEFAULT is 0, user defined camera is -1 by default
         */
        void setDepth(int8_t depth);

        /**
         * get depth, camera with larger depth is drawn on top of camera with smaller depth, the depth of camera with CameraFlag::DEFAULT is 0, user defined camera is -1 by default
         */
        int8_t getDepth() const { return _depth; }

        /**
         get rendered order
         */
        int getRenderOrder() const;

        /**
         * Get the frustum's far plane.
         */
        float getFarPlane() const { return _farPlane; }

        /**
         * Get the frustum's near plane.
         */
        float getNearPlane() const { return _nearPlane; }

        //override
        virtual void onEnter() override;
        virtual void onExit() override;

        /**
         * Get the visiting camera , the visiting camera shall be set on Scene::render
         */
        static const Camera* getVisitingCamera() { return _visitingCamera; }

        /**
         * Get the default camera of the current running scene.
         */
        static Camera* getDefaultCamera();
        /**
         Before rendering scene with this camera, the background need to be cleared.
         */
        void clearBackground(float depth);
        /**
         Apply the FBO, RenderTargets and viewport.
         */
        void apply();
        /**
         Set FBO, which will attacha several render target for the rendered result.
        */
        void setFrameBufferObject(FrameBuffer* fbo);
        /**
         Set Viewport for camera.
         */
        void setViewport(const Viewport& vp) { _viewport = vp; }
    public:
        Camera();
        ~Camera();

        /**
         * Set the scene,this method shall not be invoke manually
         */
        void setScene(Scene* scene);

        /**set additional matrix for the projection matrix, it multiplys mat to projection matrix when called, used by WP8*/
        void setAdditionalProjection(const MATH::Matrix4& mat);

        /** init camera */
        bool initDefault();
        bool initPerspective(float fieldOfView, float aspectRatio, float nearPlane, float farPlane);
        bool initOrthographic(float zoomX, float zoomY, float nearPlane, float farPlane);
        void applyFrameBufferObject();
        void applyViewport();
    protected:

        Scene* _scene; //Scene camera belongs to
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
        unsigned short _cameraFlag; // camera flag
        mutable bool _frustumDirty;
        int8_t  _depth;                 //camera depth, the depth of camera with CameraFlag::DEFAULT flag is 0 by default, a camera with larger depth is drawn on top of camera with smaller detph
        static Camera* _visitingCamera;

        Viewport _viewport;

        FrameBuffer* _fbo;
    protected:
        static Viewport _defaultViewport;
    public:
        static const Viewport& getDefaultViewport() { return _defaultViewport; }
        static void setDefaultViewport(const Viewport& vp) { _defaultViewport = vp; }
    };
}

#endif// CAMERA_H