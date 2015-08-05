#include "GRAPH/BASE/Camera.h"
#include "GRAPH/BASE/Director.h"
#include "GRAPH/BASE/GLView.h"
#include "GRAPH/BASE/Scene.h"
#include "GRAPH/RENDERER/Renderer.h"
#include "GRAPH/RENDERER/GLProgramState.h"
#include "GRAPH/RENDERER/GLProgramCache.h"
#include "GRAPH/RENDERER/GLProgram.h"
#include "GRAPH/RENDERER/GLStateCache.h"

namespace GRAPH
{
    Camera* Camera::_visitingCamera = nullptr;
    Viewport Camera::_defaultViewport;

    Camera* Camera::getDefaultCamera()
    {
        auto scene = Director::getInstance()->getRunningScene();
        if(scene)
        {
            return scene->getDefaultCamera();
        }

        return nullptr;
    }

    Camera* Camera::create()
    {
        Camera* camera = new (std::nothrow) Camera();
        camera->initDefault();
        camera->autorelease();
        camera->setDepth(0.f);

        return camera;
    }

    Camera* Camera::createPerspective(float fieldOfView, float aspectRatio, float nearPlane, float farPlane)
    {
        auto ret = new (std::nothrow) Camera();
        if (ret)
        {
            ret->initPerspective(fieldOfView, aspectRatio, nearPlane, farPlane);
            ret->autorelease();
            return ret;
        }
        SAFE_DELETE(ret);
        return nullptr;
    }

    Camera* Camera::createOrthographic(float zoomX, float zoomY, float nearPlane, float farPlane)
    {
        auto ret = new (std::nothrow) Camera();
        if (ret)
        {
            ret->initOrthographic(zoomX, zoomY, nearPlane, farPlane);
            ret->autorelease();
            return ret;
        }
        SAFE_DELETE(ret);
        return nullptr;
    }

    Camera::Camera()
    : _scene(nullptr)
    , _viewProjectionDirty(true)
    , _cameraFlag(1)
    , _frustumDirty(true)
    , _depth(-1)
    , _fbo(nullptr)
    {
    }

    Camera::~Camera()
    {
        SAFE_RELEASE_NULL(_fbo);
    }

    const MATH::Matrix4& Camera::getProjectionMatrix() const
    {
        return _projection;
    }
    const MATH::Matrix4& Camera::getViewMatrix() const
    {
        MATH::Matrix4 viewInv(getNodeToWorldTransform());
        static int count = sizeof(float) * 16;
        if (memcmp(viewInv.m, _viewInv.m, count) != 0)
        {
            _viewProjectionDirty = true;
            _frustumDirty = true;
            _viewInv = viewInv;
            _view = viewInv.getInversed();
        }
        return _view;
    }

    void Camera::lookAt(const MATH::Vector3f& lookAtPos, const MATH::Vector3f& up)
    {
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

    const MATH::Matrix4& Camera::getViewProjectionMatrix() const
    {
        getViewMatrix();
        if (_viewProjectionDirty)
        {
            _viewProjectionDirty = false;
            MATH::Matrix4::multiply(_projection, _view, &_viewProjection);
        }

        return _viewProjection;
    }

    void Camera::setAdditionalProjection(const MATH::Matrix4& mat)
    {
        _projection = mat * _projection;
        getViewProjectionMatrix();
    }

    bool Camera::initDefault()
    {
        auto size = Director::getInstance()->getWinSize();
        //create default camera
        auto projection = Director::getInstance()->getProjection();
        switch (projection)
        {
            case Director::Projection::_2D:
            {
                initOrthographic(size.width, size.height, -1024, 1024);
                setPosition3D(MATH::Vector3f(0.0f, 0.0f, 0.0f));
                setRotation3D(MATH::Vector3f(0.f, 0.f, 0.f));
                break;
            }
            case Director::Projection::_3D:
            {
                float zeye = Director::getInstance()->getZEye();
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

    bool Camera::initPerspective(float fieldOfView, float aspectRatio, float nearPlane, float farPlane)
    {
        _fieldOfView = fieldOfView;
        _aspectRatio = aspectRatio;
        _nearPlane = nearPlane;
        _farPlane = farPlane;
        MATH::Matrix4::createPerspective(_fieldOfView, _aspectRatio, _nearPlane, _farPlane, &_projection);
        _viewProjectionDirty = true;
        _frustumDirty = true;

        return true;
    }

    bool Camera::initOrthographic(float zoomX, float zoomY, float nearPlane, float farPlane)
    {
        _zoom[0] = zoomX;
        _zoom[1] = zoomY;
        _nearPlane = nearPlane;
        _farPlane = farPlane;
        MATH::Matrix4::createOrthographicOffCenter(0, _zoom[0], 0, _zoom[1], _nearPlane, _farPlane, &_projection);
        _viewProjectionDirty = true;
        _frustumDirty = true;

        return true;
    }

    MATH::Vector2f Camera::project(const MATH::Vector3f& src) const
    {
        MATH::Vector2f screenPos;

        auto viewport = Director::getInstance()->getWinSize();
        MATH::Vector4f clipPos;
        getViewProjectionMatrix().transformVector(MATH::Vector4f(src.x, src.y, src.z, 1.0f), &clipPos);

        float ndcX = clipPos.x / clipPos.w;
        float ndcY = clipPos.y / clipPos.w;

        screenPos.x = (ndcX + 1.0f) * 0.5f * viewport.width;
        screenPos.y = (1.0f - (ndcY + 1.0f) * 0.5f) * viewport.height;
        return screenPos;
    }

    MATH::Vector2f Camera::projectGL(const MATH::Vector3f& src) const
    {
        MATH::Vector2f screenPos;

        auto viewport = Director::getInstance()->getWinSize();
        MATH::Vector4f clipPos;
        getViewProjectionMatrix().transformVector(MATH::Vector4f(src.x, src.y, src.z, 1.0f), &clipPos);

        float ndcX = clipPos.x / clipPos.w;
        float ndcY = clipPos.y / clipPos.w;

        screenPos.x = (ndcX + 1.0f) * 0.5f * viewport.width;
        screenPos.y = (ndcY + 1.0f) * 0.5f * viewport.height;
        return screenPos;
    }

    MATH::Vector3f Camera::unproject(const MATH::Vector3f& src) const
    {
        MATH::Vector3f dst;
        unproject(Director::getInstance()->getWinSize(), &src, &dst);
        return dst;
    }

    MATH::Vector3f Camera::unprojectGL(const MATH::Vector3f& src) const
    {
        MATH::Vector3f dst;
        unprojectGL(Director::getInstance()->getWinSize(), &src, &dst);
        return dst;
    }

    void Camera::unproject(const MATH::Sizef& viewport, const MATH::Vector3f* src, MATH::Vector3f* dst) const
    {
        MATH::Vector4f screen(src->x / viewport.width, ((viewport.height - src->y)) / viewport.height, src->z, 1.0f);
        screen.x = screen.x * 2.0f - 1.0f;
        screen.y = screen.y * 2.0f - 1.0f;
        screen.z = screen.z * 2.0f - 1.0f;

        getViewProjectionMatrix().getInversed().transformVector(screen, &screen);
        if (screen.w != 0.0f)
        {
            screen.x /= screen.w;
            screen.y /= screen.w;
            screen.z /= screen.w;
        }

        dst->set(screen.x, screen.y, screen.z);
    }

    void Camera::unprojectGL(const MATH::Sizef& viewport, const MATH::Vector3f* src, MATH::Vector3f* dst) const
    {
        MATH::Vector4f screen(src->x / viewport.width, src->y / viewport.height, src->z, 1.0f);
        screen.x = screen.x * 2.0f - 1.0f;
        screen.y = screen.y * 2.0f - 1.0f;
        screen.z = screen.z * 2.0f - 1.0f;

        getViewProjectionMatrix().getInversed().transformVector(screen, &screen);
        if (screen.w != 0.0f)
        {
            screen.x /= screen.w;
            screen.y /= screen.w;
            screen.z /= screen.w;
        }

        dst->set(screen.x, screen.y, screen.z);
    }

    float Camera::getDepthInView(const MATH::Matrix4& transform) const
    {
        MATH::Matrix4 camWorldMat = getNodeToWorldTransform();
        const MATH::Matrix4 &viewMat = camWorldMat.getInversed();
        float depth = -(viewMat.m[2] * transform.m[12] + viewMat.m[6] * transform.m[13] + viewMat.m[10] * transform.m[14] + viewMat.m[14]);
        return depth;
    }

    void Camera::setDepth(int8_t depth)
    {
        if (_depth != depth)
        {
            _depth = depth;
            if (_scene)
            {
                //notify scene that the camera order is dirty
                _scene->setCameraOrderDirty();
            }
        }
    }

    void Camera::onEnter()
    {
        if (_scene == nullptr)
        {
            auto scene = getScene();
            if (scene)
            {
                setScene(scene);
            }
        }
        Node::onEnter();
    }

    void Camera::onExit()
    {
        // remove this camera from scene
        setScene(nullptr);
        Node::onExit();
    }

    void Camera::setScene(Scene* scene)
    {
        if (_scene != scene)
        {
            //remove old scene
            if (_scene)
            {
                auto& cameras = _scene->_cameras;
                auto it = std::find(cameras.begin(), cameras.end(), this);
                if (it != cameras.end())
                    cameras.erase(it);
                _scene = nullptr;
            }
            //set new scene
            if (scene)
            {
                _scene = scene;
                auto& cameras = _scene->_cameras;
                auto it = std::find(cameras.begin(), cameras.end(), this);
                if (it == cameras.end())
                {
                    _scene->_cameras.push_back(this);
                    //notify scene that the camera order is dirty
                    _scene->setCameraOrderDirty();
                }
            }
        }
    }

    void Camera::clearBackground(float)
    {
        GLboolean oldDepthTest;
        GLint oldDepthFunc;
        GLboolean oldDepthMask;
        {
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glStencilMask(0);

            oldDepthTest = glIsEnabled(GL_DEPTH_TEST);
            glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFunc);
            glGetBooleanv(GL_DEPTH_WRITEMASK, &oldDepthMask);

            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_ALWAYS);
        }

        //draw
        static V3F_C4B_T2F_Quad quad;
        quad.bl.vertices = MATH::Vector3f(-1,-1,0);
        quad.br.vertices = MATH::Vector3f(1,-1,0);
        quad.tl.vertices = MATH::Vector3f(-1,1,0);
        quad.tr.vertices = MATH::Vector3f(1,1,0);

        quad.bl.colors = quad.br.colors = quad.tl.colors = quad.tr.colors = Color4B(0,0,0,1);

        quad.bl.texCoords = Tex2F(0,0);
        quad.br.texCoords = Tex2F(1,0);
        quad.tl.texCoords = Tex2F(0,1);
        quad.tr.texCoords = Tex2F(1,1);

        auto shader = GLProgramCache::getInstance()->getGLProgram(GLProgram::SHADER_CAMERA_CLEAR);
        auto programState = GLProgramState::getOrCreateWithGLProgram(shader);
        programState->setUniformFloat("depth", 1.0);
        programState->apply(MATH::Matrix4());
        GLshort indices[6] = {0, 1, 2, 3, 2, 1};

        {
            bindVAO(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            enableVertexAttribs(VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);

            // vertices
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), &quad.tl.vertices);

            // colors
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V3F_C4B_T2F), &quad.tl.colors);

            // tex coords
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), &quad.tl.texCoords);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
        }


        {
            if(GL_FALSE == oldDepthTest)
            {
                glDisable(GL_DEPTH_TEST);
            }
            glDepthFunc(oldDepthFunc);

            if(GL_FALSE == oldDepthMask)
            {
                glDepthMask(GL_FALSE);
            }

            /* IMPORTANT: We only need to update the states that are not restored.
             Since we don't know what was the previous value of the mask, we update the RenderState
             after setting it.
             The other values don't need to be updated since they were restored to their original values
             */
            glStencilMask(0xFFFFF);
    //        RenderState::StateBlock::_defaultState->setStencilWrite(0xFFFFF);

            /* BUG: RenderState does not support glColorMask yet. */
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
    }

    void Camera::setFrameBufferObject(FrameBuffer *fbo)
    {
        SAFE_RETAIN(fbo);
        SAFE_RELEASE_NULL(_fbo);
        _fbo = fbo;
        if(_scene)
        {
            _scene->setCameraOrderDirty();
        }
    }

    void Camera::applyFrameBufferObject()
    {
        if(nullptr == _fbo)
        {
            FrameBuffer::applyDefaultFBO();
        }
        else
        {
            _fbo->applyFBO();
        }
    }

    void Camera::apply()
    {
        applyFrameBufferObject();
        applyViewport();
    }

    void Camera::applyViewport()
    {
        if(nullptr == _fbo)
        {
            glViewport(getDefaultViewport()._left, getDefaultViewport()._bottom, getDefaultViewport()._width, getDefaultViewport()._height);
        }
        else
        {
            glViewport(_viewport._left * _fbo->getWidth(), _viewport._bottom * _fbo->getHeight(),
                       _viewport._width * _fbo->getWidth(), _viewport._height * _fbo->getHeight());
        }

    }

    int Camera::getRenderOrder() const
    {
        int result(0);
        if(_fbo)
        {
            result = _fbo->getFID()<<8;
        }
        else
        {
            result = 127 <<8;
        }
        result += _depth;
        return result;
    }
}
